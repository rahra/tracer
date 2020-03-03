/* Copyright 2020 Bernhard R. Fischer, 4096R/8E24F29D <bf@abenteuerland.at>
 *
 * This file is part of tracer.
 *
 * Tracer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Tracer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tracer. If not, see <http://www.gnu.org/licenses/>.
 */

/*! \file scan.c
 * This file contains the main function of the image tracer.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "scan.h"
#include "memimg.h"
#include "smlog.h"

#define LAYERS 16
#define VERSION_STRING "'scan' image tracer (c) 2020 Bernhard R. Fischer, <bf@abenteuerland.at>"

enum {MODE_DIRECT, MODE_GREY};


void txtout(const memimg_t *mem)
{
   char *rlc = " rl|";
   char *upc = " ud-";
   int c, d, e;

   for (int y = -1; y < mem->height; y++)
   {
      fprintf(stderr, "%3d: ", y);
      for (int x = 0; x < mem->width; x++)
      {
         if (y >= 0)
         {
            c = memimg_get(mem, x, y);
            d = c & ~VALL;
            e = (c & VALL) >> 27;
            fprintf(stderr, "%c%c%2x ", rlc[e >> 2], upc[e & 3], d);
         }
         else
            fprintf(stderr, "  %2x ", x);
      }
      fprintf(stderr, "\n");
   }
}


static double cold(int c, int s)
{
   return ((double) ((c >> s) & 0xff)) / 255;
}


static int c2grey(int c, void * UNUSED(res))
{
   double cl, cf;

   cl = 0.2126 * cold(c, 16) + 0.7152 * cold(c, 8) + 0.0722 * cold(c, 0);
   if (cl <= 0.0031308)
      cf = 12.92 * cl;
   else
      cf = 1.055 * pow(cl, 1 / 2.4) - 0.055;

   return cf * MAXVAL;
}


static int cdirect(int c, void * UNUSED(res))
{
   return c & 0xffffff;
}


static int cinvert(int c, void * UNUSED(res))
{
   return MAXVAL - (c & 0xffffff);
}


void memprep(memimg_t *mem, int (*colfunc)(int, void*), void *res)
{
   int x, y;

   for (y = 0; y < mem->height; y++)
      for (x = 0; x < mem->width; x++)
         memimg_put(mem, x, y, colfunc(memimg_get(mem, x, y), res));
}


static int cmax(int c, void *res)
{
   if (res != NULL && *(int*)res < c)
      *(int*)res = c;
   return c;
}


static int cmin(int c, void *res)
{
   if (res != NULL && *(int*)res > c)
      *(int*)res = c;
   return c;
}


struct minmax
{
   int min, max;
};


static int cstretch(int c, void *res)
{
   double d;

   // safety check
   if (res == NULL)
      return c;

   d = ((struct minmax*) res)->max - ((struct minmax*) res)->min;
   return round((c - ((struct minmax*) res)->min) * MAXVAL / d);
}


void memstretch(memimg_t *mem)
{
   struct minmax mm;

   mm.min = mm.max = memimg_get(mem, 0, 0);
   memprep(mem, cmax, &mm.max);
   memprep(mem, cmin, &mm.min);
   log_debug("min = %d, max = %d", mm.min, mm.max);
   memprep(mem, cstretch, &mm);
}


int scan_layer(layer_t *l, memimg_t *mem)
{
//! MAXW is just here to prevent memory overflows
#define MAXW 10000
   pos_t p, scan_pos;
   int i;

   //safety check
   if (l == NULL || mem == NULL)
      return -1;

   log_debug("scanning layer v = %d", l->v);
   scan_pos.x = scan_pos.y = 0;
   for (i = l->plist_cnt; i < MAXW; i++)
   {
      if (!next_unvisited(mem, &scan_pos, l->v))
         break;

      if (add_plist(l) == -1)
      {
         log_errno(LOG_ERR, "add_plist() failed");
         break;
      }

      p = scan_pos;
      if (!(l->n[i] = scan(mem, l->v, &p, &l->plist[i])))
      {
         // reuse current plist
         i--;
         l->plist_cnt--;

         scan_pos.x++;
         if (scan_pos.x >= mem->width)
         {
            scan_pos.x = 0;
            scan_pos.y++;
         }
         continue;
      }
      log_debug("plist %d, x = %d, y = %d, %d points", i, scan_pos.x, scan_pos.y, l->n[i]);
      scan_pos.x = 0;
      scan_pos.y++;

      l->n[i] = reduce(l->plist[i], l->n[i], 5);
      log_debug("reduced plist %d points", l->n[i]);

//#define GEN_DEBUG_PNG
#ifdef GEN_DEBUG_PNG
      char buf[32];
      snprintf(buf, sizeof(buf), "XY_%03d%03d.png", l->v, i);
      memcairo(mem, buf);
#endif
   }
   clear_marks(mem);

   if (i == MAXW)
      log_msg(LOG_NOTICE, "max iteration count %d reached. You may increase MAXW and recompile", MAXW);

   return 0;
}


void usage(const char *s)
{
   printf("%s\nusage: %s [OPTIONS] [<filename>]\n", VERSION_STRING, s);
   printf("   OPTIONS\n"
          "      -h ............ Print this message.\n"
          "      -m <mode> ..... Scan mode, 'direct' or 'grey'.\n"
          "      -n <layers> ... Number of layers to scan (default = %d).\n"
          "      -s ............ Stretch color values from 0 - MAXVAL.\n"
          "      -x <factor> ... Scaling factor for geo coordinates (default = 1.0).\n"
          "\n", LAYERS);
}


int main(int argc, char **argv)
{
   char *s = "a.png";
   int nlayers = LAYERS, n, mode = MODE_GREY, stretch = 0;
   memimg_t mem;
   layer_t l[MAXL];

   init_log("stderr", LOG_INFO);

   while ((n = getopt(argc, argv, "hm:n:sx:")) != -1)
      switch (n)
      {
         case 'm':
            if (!strcasecmp(optarg, "direct"))
               mode = MODE_DIRECT;
            else if (!strcasecmp(optarg, "grey"))
               mode = MODE_GREY;
            else
            {
               log_msg(LOG_NOTICE, "unknown mode '%s', defaulting to greyscale", optarg);
               mode = MODE_GREY;
            }
            break;

         case 'n':
            if ((nlayers = atoi(optarg)) <= 0)
            {
               nlayers = 1;
               log_msg(LOG_NOTICE, "number of layers reset to %d", nlayers);
            }
            break;

         case 's':
            stretch = 1;
            break;

         case 'x':
            osm_scale_ = atof(optarg);
            break;

         case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
      }

   if (argv[optind] != NULL)
      s = argv[optind];

   if (cairomem(&mem, s) == -1)
   {
      log_msg(LOG_ERR, "cairo_mem() failed");
      exit(1);
   }

   switch (mode)
   {
      case MODE_DIRECT:
         memprep(&mem, cdirect, NULL);
         break;

      case MODE_GREY:
         memprep(&mem, c2grey, NULL);
         break;

      default:
         log_msg(LOG_EMERG, "this should never happen, mode = %d", mode);
         exit(1);
   }

   if (stretch)
      memstretch(&mem);

   for (int j = 0; j < nlayers; j++)
   {
      memset(&l[j], 0, sizeof(l[j]));
      l[j].v = MAXVAL - MAXVAL / (nlayers + 1) * (j + 1);

      log_msg(LOG_INFO, "layer %d", j);
      scan_layer(&l[j], &mem);
   }

   export_osm(l, "a.osm", nlayers, &mem);
   export_svg(l, "a.svg", nlayers, &mem);

   free(l->plist);
   memimg_free(&mem);

   return 0;
}

