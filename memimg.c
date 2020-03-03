/* Copyright 2020 Bernhard R. Fischer, 4096R/8E24F29D <bf@abenteuerland.at>
 *
 * This file is part of wolken.
 *
 * Wolken is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * Wolken is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wolken. If not, see <http://www.gnu.org/licenses/>.
 */

/*! \file memimg.c
 * This file contains all code relavent to the memory image.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "memimg.h"


int memimg_init(memimg_t *mi)
{
   // safety check
   if (mi == NULL || mi->width <= 0 || mi->height <= 0)
      return -1;

   if ((mi->mem = calloc(mi->width * mi->height, sizeof(*mi->mem))) == NULL)
      return -1;

   return 0;
}


void memimg_free(memimg_t *mi)
{
   // safety check
   if (mi == NULL)
      return;

   free(mi->mem);
   mi->mem = NULL;
}


int memimg_copy(memimg_t *src, memimg_t *dst)
{
   size_t len;

   // safety check
   if (src == NULL || dst == NULL)
      return -1;

   memcpy(dst, src, sizeof(*dst));
   len = dst->width * dst->height * sizeof(*dst->mem);
   if ((dst->mem = malloc(len)) == NULL)
      return -1;

   memcpy(dst->mem, src->mem, len);
   return 0;
}


int memimg_get(const memimg_t *mi, int x, int y)
{
   // safety check
   if (x < 0 || x >= mi->width || y < 0 || y >= mi->height)
   {
      errno = EFAULT;
      return -1;
   }

   return mi->mem[y * mi->width + x];
}


int memimg_put(memimg_t *mi, int x, int y, int f)
{
   int tf;

   // safety check
   if (x < 0 || x >= mi->width || y < 0 || y >= mi->height)
   {
      errno = EFAULT;
      return -1;
   }
 
   tf = mi->mem[y * mi->width + x];
   mi->mem[y * mi->width + x] = f;

   return tf;
}


int f_or(int a, int b)
{
   return a | b;
}


int f_and(int a, int b)
{
   return a & b;
}


int memimg_op(memimg_t *mi, int x, int y, int f, int (*op)(int, int))
{
   int tf;

   // safety check
   if (x < 0 || x >= mi->width || y < 0 || y >= mi->height)
   {
      errno = EFAULT;
      return -1;
   }

   tf = mi->mem[y * mi->width + x];
   mi->mem[y * mi->width + x] = op(tf, f);

   return tf;
}


int memimg_or(memimg_t *mi, int x, int y, int f)
{
   return memimg_op(mi, x, y, f, f_or);
}


int memimg_and(memimg_t *mi, int x, int y, int f)
{
   return memimg_op(mi, x, y, f, f_and);
}

