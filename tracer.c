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

/*! \file tracer.c
 * This file contains all image tracing code.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scan.h"
#include "smlog.h"

#define memimg_get0(a, b, c) (memimg_get(a, b, c) & ~VALL)


int next_unvisited(const memimg_t *mem, pos_t *p, int v)
{
   int c, in;

   for (; p->y < mem->height; p->y++)
   {
      in = 0;
      for (; p->x < mem->width; p->x++)
      {
         c = memimg_get(mem, p->x, p->y);
         if ((c & ~VALL) < v)
         {
            in = 0;
            continue;
         }

         if (c & VALL)
         {
            in = 1;
            continue;
         }

         if (!in)
            return 1;
      }
      p->x = 0;
   }
   return 0;
}


int is_inside(const memimg_t *mem, int v, const pos_t *pos)
{
   return memimg_get(mem, pos->x, pos->y) >= v;
}


void set_dir(int *dirp, int dir)
{
   if (dirp != NULL)
      *dirp = dir;
}


void clear_marks(memimg_t *mem)
{
   int x, y;

   for (y = 0; y < mem->height; y++)
      for (x = 0; x < mem->width; x++)
         memimg_and(mem, x, y, ~VALL);
}


int scan_inside_down0(const memimg_t *mem, int v, pos_t *pos, int *dir, int mark)
{
   int c;

   set_dir(dir, LEFT);

   for (; pos->y > 0; pos->y--)
   {
      if (mark)
         memimg_or((memimg_t*) mem, pos->x, pos->y, VRIGHT);

      if ((c = memimg_get(mem, pos->x, pos->y - 1)) < v)
      {
         pos->yf = pos->y - (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->y;
      }
   }

   pos->yf = pos->y;
   return pos->y;
}


int scan_inside_down(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   return scan_inside_down0(mem, v, pos, dir, 1);
}


int scan_inside_left(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   int c;

   set_dir(dir, UP);

   for (; pos->x > 0; pos->x--)
   {
      memimg_or((memimg_t*) mem, pos->x, pos->y, VUP);

      if ((c = memimg_get(mem, pos->x - 1, pos->y)) < v)
      {
         pos->xf = pos->x - (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->x;
      }
   }

   pos->xf = pos->x;
   return pos->x;
}


int scan_sideway_down(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   int c;

   if (pos->x >= mem->width - 1)
      return scan_inside_down(mem, v, pos, dir);

   set_dir(dir, LEFT);
   for (; pos->y > 0; pos->y--)
   {
      memimg_or((memimg_t*) mem, pos->x, pos->y, VRIGHT);

      if ((c = memimg_get(mem, pos->x, pos->y - 1)) < v)
      {
         pos->yf = pos->y - (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->y;
      }
      if ((c = memimg_get(mem, pos->x + 1, pos->y - 1)) >= v)
      {
         pos->yf = pos->y - (double) (v - c) / (memimg_get0(mem, pos->x + 1, pos->y) - c);
         set_dir(dir, RIGHT);
         return --pos->y;
      }
   }

   pos->yf = pos->y;
   return pos->y;
}


static int sqdist(int a, int b)
{
   return a * a + b * b;
}


int reduce(pos_t *pos, int n, int m)
{
   int i;

   for (i = 0; i < n - 2 && n > 4;)
   {
      if (sqdist(pos[i].x - pos[i + 1].x, pos[i].y - pos[i + 1].y) < m * m)
      {
         memmove(&pos[i + 1], &pos[i + 2], (n - i - 2) * sizeof(*pos));
         n--;
      }
      else
         i++;
   }

   return n;
}


int scan_sideway_left0(const memimg_t *mem, int v, pos_t *pos, int *dir, int mark)
{
   int c;

   if (!pos->y)
      return scan_inside_left(mem, v, pos, dir);

   set_dir(dir, UP);
   for (; pos->x > 0; pos->x--)
   {
      if (mark)
         memimg_or((memimg_t*) mem, pos->x, pos->y, VUP);

      if ((c = memimg_get(mem, pos->x - 1, pos->y)) < v)
      {
         pos->xf = pos->x - (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->x;
      }
      if ((c = memimg_get(mem, pos->x - 1, pos->y - 1)) >= v)
      {
         pos->xf = pos->x - (double) (v - c) / (memimg_get0(mem, pos->x, pos->y - 1) - c);
         set_dir(dir, DOWN);
         return --pos->x;
      }
   }

   pos->xf = pos->x;
   return pos->x;
}


int scan_sideway_left(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   return scan_sideway_left0(mem, v, pos, dir, 1);
}


int scan_inside_up(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   int c;

   set_dir(dir, RIGHT);

   for (; pos->y < mem->height - 1; pos->y++)
   {
      memimg_or((memimg_t*) mem, pos->x, pos->y, VLEFT);

      if ((c = memimg_get(mem, pos->x, pos->y + 1)) < v)
      {
         pos->yf = pos->y + (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->y;
      }
   }

   pos->yf = pos->y;
   return pos->y;
}


int scan_inside_right(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   int c;

   set_dir(dir, DOWN);

   for (; pos->x < mem->width - 1; pos->x++)
   {
      memimg_or((memimg_t*) mem, pos->x, pos->y, VDOWN);

      if ((c = memimg_get(mem, pos->x + 1, pos->y)) < v)
      {
         pos->xf = pos->x + (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->x;
      }
   }

   pos->xf = pos->x;
   return pos->x;
}


int scan_sideway_up(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   int c;

   if (!pos->x)
      return scan_inside_up(mem, v, pos, dir);

   set_dir(dir, RIGHT);
   for (; pos->y < mem->height - 1; pos->y++)
   {
      memimg_or((memimg_t*) mem, pos->x, pos->y, VLEFT);

      if ((c = memimg_get(mem, pos->x, pos->y + 1)) < v)
      {
         pos->yf = pos->y + (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->y;
      }
      if ((c = memimg_get(mem, pos->x - 1, pos->y + 1)) >= v)
      {
         pos->yf = pos->y + (double) (v - c) / (memimg_get0(mem, pos->x - 1, pos->y) - c);
         set_dir(dir, LEFT);
         return ++pos->y;
      }
   }

   pos->yf = pos->y;
   return pos->y;
}


int scan_sideway_right(const memimg_t *mem, int v, pos_t *pos, int *dir)
{
   int c;

   if (pos->y >= mem->height- 1)
      return scan_inside_right(mem, v, pos, dir);

   set_dir(dir, DOWN);
   for (; pos->x < mem->width - 1; pos->x++)
   {
      memimg_or((memimg_t*) mem, pos->x, pos->y, VDOWN);

      if ((c = memimg_get(mem, pos->x + 1, pos->y)) < v)
      {
         pos->xf = pos->x + (double) (v - c) / (memimg_get0(mem, pos->x, pos->y) - c);
         return pos->x;
      }
      if ((c = memimg_get(mem, pos->x + 1, pos->y + 1)) >= v)
      {
         pos->xf = pos->x + (double) (v - c) / (memimg_get0(mem, pos->x, pos->y + 1) - c);
         set_dir(dir, UP);
         return ++pos->x;
      }
   }

   pos->xf = pos->x;
   return pos->x;
}


void find_lowercorner(const memimg_t *mem, int v, pos_t *pos, int *scan_dir)
{
   // find 1st corner point
   scan_inside_down0(mem, v, pos, scan_dir, 0);
   scan_sideway_left0(mem, v, pos, scan_dir, 0);
}


static int poscmp(const pos_t *a, const pos_t *b)
{
   return !(a->x == b->x && a->y == b->y);
}


int scan(memimg_t *mem, int v, pos_t *pos, pos_t **plist)
{
   int i, n;
   int scan_dir;

   // make sure point is inside
   if (!is_inside(mem, v, pos))
      return -1;

   find_lowercorner(mem, v, pos, &scan_dir);
   if ((memimg_get(mem, pos->x, pos->y) & VALL))
   {
      // too much debugging
      //log_debug("edge %d/%d already visited", pos->x, pos->y);
      return 0;
   }

   // reserver memory for the 1st 2 pos elements
   if ((*plist = malloc(sizeof(**plist))) == NULL)
      return -1;

   (*plist)[0] = *pos;

   for (n = 1, i = 0; ; i++)
   {

      // make sure point is inside
      if (!is_inside(mem, v, pos))
      {
         log_msg(LOG_ERR, "something went wrong...");
         //return -1;
      }

      // break endless loop
      if (i >= 4 && n == 1)
      {
         log_debug("seems to be a single point");
         break;
      }

      // break if polygon is closed
      if (n > 1 && !poscmp(&(*plist)[0], &(*plist)[n - 1]))
      {
         log_debug("closed");
         break;
      }

      switch (scan_dir)
      {
         case LEFT:
            scan_sideway_left(mem, v, pos, &scan_dir);
            break;

         case RIGHT:
            scan_sideway_right(mem, v, pos, &scan_dir);
            break;

         case UP:
            scan_sideway_up(mem, v, pos, &scan_dir);
            break;

         case DOWN:
            scan_sideway_down(mem, v, pos, &scan_dir);
            break;

         default:
            log_msg(LOG_EMERG, "This should never happen...");
            exit(1);
      } // switch (scan_dir)

      // ignore duplicates
      if (poscmp(&(*plist)[n - 1], pos))
      {
         pos_t *tpos;
         if ((tpos = realloc(*plist, sizeof(*tpos) * (n + 1))) == NULL)
         {
            log_errno(LOG_ERR, "realloc()");
            return n;
         }
         *plist = tpos;
         (*plist)[n] = *pos;
         n++;
      }
   } // for (n = 1, i = 0; ; i++)

   log_debug("%d iterations, %d points", i + 1, n);

   if (n == 1)
      memimg_or(mem, (*plist)[0].x, (*plist)[0].y, VALL);

   return n;
}

