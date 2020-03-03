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

/*! \file cairoexport.c
 * This file contains to code for for SVG output.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#include <stdio.h>
#include <stdint.h>
#include <cairo.h>
#ifdef CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif

#include "scan.h"
#include "memimg.h"
#include "smlog.h"


static int nodelistpath(cairo_t *ctx, const pos_t *p, int n, double c)
{
   int i;

   // safety check
   if (ctx == NULL || p == NULL)
      return -1;

   if (n <= 1)
      return 0;

   cairo_new_path(ctx);
   for (i = 1; i < n; i++, p++)
      cairo_line_to(ctx, p->xf, p->yf);
   cairo_close_path(ctx);
   cairo_set_source_rgb(ctx, 0, 0, 0);
#ifndef FILLING
   cairo_stroke(ctx);
#else
   cairo_stroke_preserve(ctx);
   cairo_set_source_rgb(ctx, 0, 0, c);
   cairo_fill(ctx);
#endif

   return i;
}


int export_svg(const layer_t *l, const char *s, int nlayers, memimg_t *mem)
{
#ifdef CAIRO_HAS_SVG_SURFACE
   cairo_surface_t *sfc, *svg;
   cairo_t *ctx;
   int i, j;

   sfc = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, NULL);
   ctx = cairo_create(sfc);
   cairo_set_line_width(ctx, .3);

   for (j = nlayers - 1; j >= 0; j--)
   {
      cairo_push_group(ctx);
      for (i = 0; i < l[j].plist_cnt; i++)
         nodelistpath(ctx, l[j].plist[i], l[j].n[i], (double) (nlayers - 1 - j) / nlayers);
      cairo_pop_group_to_source(ctx);
      cairo_paint(ctx);
   }

   cairo_destroy(ctx);

   // FIXME: size not correct
   svg = cairo_svg_surface_create(s, (double) mem->width * 72 / 300, (double) mem->height * 72 / 300);
   ctx = cairo_create(svg);
   cairo_set_source_surface(ctx, sfc, 0, 0);
   cairo_paint(ctx);
   cairo_destroy(ctx);
   cairo_surface_destroy(svg);
   cairo_surface_destroy(sfc);
#else
   log_msg(LOG_NOTICE, "cairo compiled without SVG support");
#endif

   return 0;
}

