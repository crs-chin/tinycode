/*
 * SMS TPDU dessector.
 * Copyright (C) <2018>  Crs Chin<crs.chin@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define INIT_STRING_LEN 100

int asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    char *buf;
    size_t len = INIT_STRING_LEN;
    int res;

    if(! strp || ! fmt || ! (buf = (char *)malloc(len)))
        return -1;

    va_start(ap, fmt);
    do {
        res = vsnprintf(buf, len, fmt, ap);

        if(res >= len) {
            free(buf);

            if(! (buf = (char *)malloc(len + INIT_STRING_LEN))) {
                res = -1;
                break;
            }

            len += INIT_STRING_LEN;
            va_end(ap);
            va_start(ap, fmt);
            continue;
        }
    } while(0);
    va_end(ap);

    *strp = buf;
    return res;
}

int vasprintf(char **strp, const char *fmt, va_list _ap)
{
    va_list ap;
    char *buf;
    size_t len = INIT_STRING_LEN * 10;
    int res;

    if(! strp || ! fmt || ! (buf = (char *)malloc(len)))
        return -1;

    va_copy(ap, _ap);
    do {
        res = vsnprintf(buf, len, fmt, ap);
        if(res >= len) {
            free(buf);

            if(! (buf = (char *)malloc(len + INIT_STRING_LEN))) {
                res = -1;
                break;
            }

            len += INIT_STRING_LEN;
            va_end(ap);
            va_copy(ap, _ap);
            continue;
        }
    } while(0);
    va_end(ap);

    *strp = buf;
    return res;
}

