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

/*! \file layer.c
 * This file contains helper functions for the layer_t structure.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "scan.h"


layer_t *new_layer(int v)
{
   layer_t *nl;

   if ((nl = calloc(1, sizeof(*nl))) == NULL)
      return NULL;

   nl->v = v;
   return nl;
}


int add_plist(layer_t *l)
{
   pos_t **plist;
   int *n;

   if ((plist = realloc(l->plist, (l->plist_cnt + 1) * sizeof(*plist))) == NULL)
      return -1;
   l->plist = plist;

   if ((n = realloc(l->n, (l->plist_cnt + 1) * sizeof(*n))) == NULL)
      return -1;
   l->n = n;

   l->plist_cnt++;
   return l->plist_cnt;
}

