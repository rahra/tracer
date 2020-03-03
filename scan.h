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

/*! \file wolken.h
 * This file contains all declarations and macros.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#ifndef SCAN_H
#define SCAN_H

#include "memimg.h"

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#define MAXVAL 255

#define VLEFT (1 << 30)
#define VRIGHT (1 << 29)
#define VDOWN (1 << 28)
#define VUP (1 << 27)
#define VALL (VLEFT | VDOWN | VRIGHT | VUP)

#define MAXL 256


typedef struct pos
{
   int x;
   int y;
   double xf;
   double yf;
} pos_t;

typedef struct layer
{
   int v;
   int plist_cnt;
   pos_t **plist;
   int *n;
} layer_t;

/* LEFT and DOWN means decreasing coordinates, RIGHT and UP increasing. */
enum {LEFT, DOWN, RIGHT, UP};


/* wcairo.c */
void memcairo(const memimg_t *mem, const char *s);
int cairomem(memimg_t *mem, const char *s);

/* cairoexport.c */
int export_svg(const layer_t *l, const char *s, int nlayers, memimg_t *mem);

/* tracer.c */
int next_unvisited(const memimg_t *mem, pos_t *p, int v);
int scan(memimg_t *mem, int v, pos_t *pos, pos_t **plist);
void clear_marks(memimg_t *mem);
int reduce(pos_t *pos, int n, int m);

/* layer.c */
layer_t *new_layer(int v);
int add_plist(layer_t *l);

/* wosm.c */
int export_osm(const layer_t *l, const char *s, int nlayers, memimg_t *mem);

extern double osm_scale_;


#endif

