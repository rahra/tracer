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

/*! \file memimg.h
 * This file contains all declarations for the memory image functions.
 *
 * @author Bernhard R. Fischer
 * @version 2020/02/26
 */

#ifndef MEMIMG_H
#define MEMIMG_H


typedef struct memimg
{
   int *mem;
   int width;
   int height;
} memimg_t;


int memimg_init(memimg_t *mi);
void memimg_free(memimg_t *mi);
int memimg_copy(memimg_t *src, memimg_t *dst);
int memimg_get(const memimg_t *mi, int x, int y);
int memimg_put(memimg_t *mi, int x, int y, int f);
int memimg_or(memimg_t *mi, int x, int y, int f);
int memimg_and(memimg_t *mi, int x, int y, int f);


#endif

