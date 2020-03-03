/* Copyright 2008-2020 Bernhard R. Fischer.
 *
 * This file is part of wolken which originally already included in OnionCat
 * and Smrender.
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

/*! \file smlog.h
 * This file simply contains the logging functions. It was originally written
 * for OnionCat and was adapted to be used for smrender.
 * 
 * @author Bernhard R. Fischer
 * @version 2020/02/20
 */

#ifndef SMLOG_H
#define SMLOG_H

#include <stdio.h>
#include <syslog.h>


#define LOG_WARN LOG_WARNING
#define log_debug(fmt, x...) log_msg(LOG_DEBUG, "%s() " fmt, __func__, ## x)
#define log_warn(x...) log_msg(LOG_WARN, ## x)

FILE *init_log(const char *s, int level);
int log_msg(int, const char*, ...) __attribute__((format (printf, 2, 3)));
int log_errno(int , const char *);

#endif

