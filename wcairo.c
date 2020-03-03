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

/*! \file wcairo.c
 * This file contains to code for for PNG input and SVG output.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#include <stdio.h>
#include <stdint.h>
#include <cairo.h>

#include "scan.h"
#include "memimg.h"
#include "smlog.h"


uint32_t color(int c)
{
   //printf("%d\n", c);
   //return 0xff000000 | ((c & 0xff) | ((255 - (c & 0xff)) << 8)) | ((c & VALL) >> 7);
   //return 0xff000000 | c;
   return 0xff000000 | c | ((c & VALL) >> 7);
}


static inline int cairo_smr_bpp(cairo_format_t fmt)
{
   switch (fmt)
   {
      case CAIRO_FORMAT_ARGB32:
      case CAIRO_FORMAT_RGB24:
      case CAIRO_FORMAT_RGB30:
         return 4;

      case CAIRO_FORMAT_RGB16_565:
         return 2;

      // FIXME: not implemented yet case CAIRO_FORMAT_A1:

      case CAIRO_FORMAT_A8:
      default:
         return 1;

   }
}


/*! Return the memory address of a Pixel.
 *  @param x X position.
 *  @param y Y position.
 *  @param s Stride, i.e. the number of bytes per row.
 *  @param bpp Number of bytes per pixel.
 *  @return Memory offset to the base pointer of the pixel matrix.
 */
static inline int cairo_smr_pixel_pos(int x, int y, int s, int bpp)
{
   return x * bpp + y * s;
}


static uint32_t cairo_smr_get_raw_pixel(unsigned char *data, cairo_format_t fmt)
{
   uint32_t rc;

   switch (fmt)
   {
      case CAIRO_FORMAT_ARGB32:
      case CAIRO_FORMAT_RGB24:
         return *((uint32_t*) data);

      case CAIRO_FORMAT_RGB30:
         rc = *((uint32_t*) data);
         return ((rc >> 2) & 0xff) | ((rc >> 4) & 0xff00) | ((rc >> 6) & 0xff0000);
      
      case CAIRO_FORMAT_RGB16_565:
         rc = *((uint16_t*) data);
         return ((rc << 3) & 0xff) | ((rc << 5) & 0xfc00) | ((rc << 8) & 0xf80000);

      // FIXME: not implemented yet case CAIRO_FORMAT_A1:

      case CAIRO_FORMAT_A8:
         rc = *data;
         return rc | ((rc << 8) & 0xff00) | ((rc << 16) & 0xff0000);

      default:
         return 0;
   }
}


static void cairo_smr_set_raw_pixel(unsigned char *data, cairo_format_t fmt, uint32_t rc)
{
   switch (fmt)
   {
      case CAIRO_FORMAT_ARGB32:
      case CAIRO_FORMAT_RGB24:
         *((uint32_t*) data) = rc;
			break;

      case CAIRO_FORMAT_RGB30:
         *((uint32_t*) data) = ((rc >> 2) & 0xff) | ((rc >> 4) & 0xff00) | ((rc >> 6) & 0xff0000);
         break;
      
      case CAIRO_FORMAT_RGB16_565:
         *((uint16_t*) data) = ((rc << 3) & 0xff) | ((rc << 5) & 0xfc00) | ((rc << 8) & 0xf80000);
         break;

      // FIXME: not implemented yet case CAIRO_FORMAT_A1:

      case CAIRO_FORMAT_A8:
         *data = rc; //| ((rc << 8) & 0xff00) | ((rc << 16) & 0xff0000);
         break;

      default:
         printf("not implemented\n");
   }
}


int cairo_smr_get_pixel(cairo_surface_t *sfc, int x, int y)
{
   unsigned char *data;

   // FIXME: flush may be done outside get_pixel()
   cairo_surface_flush(sfc);
   if ((data = cairo_image_surface_get_data(sfc)) == NULL)
      return 0;

   return cairo_smr_get_raw_pixel(data + cairo_smr_pixel_pos(x, y,
                  cairo_image_surface_get_stride(sfc), cairo_smr_bpp(cairo_image_surface_get_format(sfc))),
         cairo_image_surface_get_format(sfc));
}


void cairo_smr_set_pixel(cairo_surface_t *sfc, int x, int y, int c)
{
   unsigned char *data;

   if ((data = cairo_image_surface_get_data(sfc)) == NULL)
      return;

   cairo_smr_set_raw_pixel(data + cairo_smr_pixel_pos(x, y,
                  cairo_image_surface_get_stride(sfc), cairo_smr_bpp(cairo_image_surface_get_format(sfc))),
         cairo_image_surface_get_format(sfc), c);
}


void memcairo(const memimg_t *mem, const char *s)
{
   cairo_surface_t *sfc;

   sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mem->width, mem->height);
   cairo_surface_flush(sfc);

   for (int y = 0; y < mem->height; y++)
      for (int x = 0; x < mem->width; x++)
         cairo_smr_set_pixel(sfc, x, y, color(memimg_get(mem, x, y)));

   cairo_surface_flush(sfc);
   cairo_surface_write_to_png(sfc, s);
   cairo_surface_destroy(sfc);
}


int cairomem(memimg_t *mem, const char *s)
{
   cairo_surface_t *sfc;

   // safety check
   if (mem == NULL || s == NULL)
      return -1;

   sfc = cairo_image_surface_create_from_png(s);
   if (cairo_surface_status(sfc) != CAIRO_STATUS_SUCCESS)
      return -1;

   mem->width = cairo_image_surface_get_width(sfc);
   mem->height = cairo_image_surface_get_height(sfc);

   if (memimg_init(mem) == -1)
      return -1;

   for (int y = 0; y < mem->height; y++)
      for (int x = 0; x < mem->width; x++)
         memimg_put(mem, x, y, cairo_smr_get_pixel(sfc, x, y));

   cairo_surface_destroy(sfc);
   return 0;
}

