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

/*! \file wosm.c
 * This file contains to code for the OSM output.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "scan.h"


double osm_scale_ = 1;


static int64_t node_id(int i, int n)
{
   return (int64_t) i | (int64_t) n << 16;
}


static void osmway(FILE *f, const pos_t *p, int n, int id, int v)
{
   int c = 0;

   if (n <= 1)
      return;

   c = p[0].x == p[n - 1].x && p[0].y == p[n - 1].y;

   fprintf(f, "<way id='%d' action='modify' visible='true'>\n<tag k=\"ele\" v=\"%d\"/>\n", -id, v);

   for (int i = 0; i < n - c; i++)
      fprintf(f, "<nd ref=\"%ld\"/>\n", -node_id(i + 1, id));

   if (c)
      fprintf(f, "<nd ref=\"%ld\"/>\n", -node_id(1, id));

   fprintf(f, "</way>\n");
}


static void osmnode(FILE *f, const pos_t *p, int64_t id, int peak, memimg_t *mem)
{
   fprintf(f, "<node id=\"%ld\" action=\"modify\" lon=\"%f\" lat=\"%f\" visible=\"true\">\n", id, (double) p->xf / mem->width * osm_scale_, (double) (mem->height - p->yf - 1) / mem->height * osm_scale_);
   fprintf(f, "<tag k=\"random\" v=\"%f\"/>\n", (double) random() / RAND_MAX);
   if (peak)
      fprintf(f, "<tag k=\"summit\" v=\"yes\"/>\n<tag k=\"ele\" v=\"%d\"/>\n", peak);
   fprintf(f, "</node>");
}


static void osmnodelist(FILE *f, const pos_t *p, int n, int id, memimg_t *mem)
{
   if (n > 1 && p[0].x == p[n - 1].x && p[0].y == p[n - 1].y)
      n--;

   for (int i = 0; i < n; i++)
      osmnode(f, &p[i], -node_id(i + 1, id), n == 1 ? memimg_get(mem, p[0].x, p[0].y) & ~VALL : 0, mem);
}


static void startosm(FILE *f)
{
   fprintf(f, "<?xml version='1.0' encoding='UTF-8'?>\n<osm version='0.6' upload='true' generator='Wolken_BF'>\n");
}


static void endosm(FILE *f)
{
   fprintf(f, "</osm>\n");
}


int export_osm(const layer_t *l, const char *s, int nlayers, memimg_t *mem)
{
   FILE *f;
   int i, j;

   if ((f = fopen(s, "w")) == NULL)
      return -1;

   startosm(f);

   for (j = 0; j < nlayers; j++)
   {
      for (i = 0; i < l[j].plist_cnt; i++)
         osmnodelist(f, l[j].plist[i], l[j].n[i], (i + 1) | (j << 16), mem);
   }

   for (j = 0; j < nlayers; j++)
   {
      for (i = 0; i < l[j].plist_cnt; i++)
         osmway(f, l[j].plist[i], l[j].n[i], (i + 1) | (j << 16), l[j].v);
   }

   endosm(f);
   fclose(f);

   return 0;
}

