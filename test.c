/*
 * UTF encoding conversion utils.
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
#include <string.h>

#include "tinycode.h"

static unsigned char utf16be[] = {
    0x00, 0x1b, 0x00, 0x5b, 0x00, 0x33, 0x00, 0x32,  0x00, 0x6d, 0x30, 0x0a, 0x5b, 0xfb, 0x89, 0x7f,
    0x5c, 0x71, 0x96, 0x90, 0x80, 0x05, 0x4e, 0x0d,  0x90, 0x47, 0x30, 0x0b, 0x00, 0x1b, 0x00, 0x5b,
    0x00, 0x6d, 0x00, 0x0a, 0x00, 0x1b, 0x00, 0x5b,  0x00, 0x33, 0x00, 0x33, 0x00, 0x6d, 0x4f, 0x5c,
    0x80, 0x05, 0xff, 0x1a, 0x90, 0xb1, 0x4e, 0x3a,  0x00, 0x1b, 0x00, 0x5b, 0x00, 0x6d, 0x00, 0x0a,
    0x7e, 0xdd, 0x98, 0x76, 0x4e, 0x00, 0x83, 0x05,  0x83, 0x28, 0xff, 0x0c, 0x76, 0xf4, 0x4e, 0x0a,
    0x4e, 0x09, 0x53, 0x41, 0x91, 0xcc, 0x30, 0x02,  0x00, 0x0a, 0x53, 0xe9, 0x51, 0x73, 0x65, 0xe0,
    0x50, 0xee, 0x4e, 0xc6, 0xff, 0x0c, 0x7a, 0xa5,  0x5b, 0xa4, 0x60, 0xdf, 0x68, 0x48, 0x51, 0xe0,
    0x30, 0x02, 0x00, 0x0a, 0x82, 0xe5, 0x97, 0x5e,  0x5d, 0xfe, 0x67, 0xf4, 0x8f, 0x66, 0xff, 0x0c,
    0x5e, 0x94, 0x66, 0x2f, 0x94, 0x93, 0x79, 0xcb,  0x6c, 0x34, 0x30, 0x02, 0x00, 0x0a, 0x5d, 0xee,
    0x6c, 0x60, 0x4e, 0x0d, 0x76, 0xf8, 0x89, 0xc1,  0xff, 0x0c, 0x9e, 0xfe, 0x52, 0xc9, 0x7a, 0x7a,
    0x4e, 0xf0, 0x6b, 0x62, 0x30, 0x02, 0x00, 0x0a,  0x83, 0x49, 0x82, 0x72, 0x65, 0xb0, 0x96, 0xe8,
    0x4e, 0x2d, 0xff, 0x0c, 0x67, 0x7e, 0x58, 0xf0,  0x66, 0x5a, 0x7a, 0x97, 0x91, 0xcc, 0x30, 0x02,
    0x00, 0x0a, 0x53, 0xca, 0x51, 0x79, 0x59, 0x51,  0x5e, 0x7d, 0x7e, 0xdd, 0xff, 0x0c, 0x81, 0xea,
    0x8d, 0xb3, 0x83, 0x61, 0x5f, 0xc3, 0x80, 0x33,  0x30, 0x02, 0x00, 0x0a, 0x86, 0x7d, 0x65, 0xe0,
    0x5b, 0xbe, 0x4e, 0x3b, 0x61, 0x0f, 0xff, 0x0c,  0x98, 0x87, 0x5f, 0x97, 0x6e, 0x05, 0x51, 0xc0,
    0x74, 0x06, 0x30, 0x02, 0x00, 0x0a, 0x51, 0x74,  0x5c, 0x3d, 0x65, 0xb9, 0x4e, 0x0b, 0x5c, 0x71,
    0xff, 0x0c, 0x4f, 0x55, 0x5f, 0xc5, 0x5f, 0x85,  0x4e, 0x4b, 0x5b, 0x50, 0x30, 0x02, 0x00, 0x0a,
};

static unsigned char utf16le[] = {
    0x1b, 0x00, 0x5b, 0x00, 0x33, 0x00, 0x32, 0x00,  0x6d, 0x00, 0x0a, 0x30, 0xfb, 0x5b, 0x7f, 0x89, 
    0x71, 0x5c, 0x90, 0x96, 0x05, 0x80, 0x0d, 0x4e,  0x47, 0x90, 0x0b, 0x30, 0x1b, 0x00, 0x5b, 0x00, 
    0x6d, 0x00, 0x0a, 0x00, 0x1b, 0x00, 0x5b, 0x00,  0x33, 0x00, 0x33, 0x00, 0x6d, 0x00, 0x5c, 0x4f, 
    0x05, 0x80, 0x1a, 0xff, 0xb1, 0x90, 0x3a, 0x4e,  0x1b, 0x00, 0x5b, 0x00, 0x6d, 0x00, 0x0a, 0x00, 
    0xdd, 0x7e, 0x76, 0x98, 0x00, 0x4e, 0x05, 0x83,  0x28, 0x83, 0x0c, 0xff, 0xf4, 0x76, 0x0a, 0x4e, 
    0x09, 0x4e, 0x41, 0x53, 0xcc, 0x91, 0x02, 0x30,  0x0a, 0x00, 0xe9, 0x53, 0x73, 0x51, 0xe0, 0x65, 
    0xee, 0x50, 0xc6, 0x4e, 0x0c, 0xff, 0xa5, 0x7a,  0xa4, 0x5b, 0xdf, 0x60, 0x48, 0x68, 0xe0, 0x51, 
    0x02, 0x30, 0x0a, 0x00, 0xe5, 0x82, 0x5e, 0x97,  0xfe, 0x5d, 0xf4, 0x67, 0x66, 0x8f, 0x0c, 0xff, 
    0x94, 0x5e, 0x2f, 0x66, 0x93, 0x94, 0xcb, 0x79,  0x34, 0x6c, 0x02, 0x30, 0x0a, 0x00, 0xee, 0x5d, 
    0x60, 0x6c, 0x0d, 0x4e, 0xf8, 0x76, 0xc1, 0x89,  0x0c, 0xff, 0xfe, 0x9e, 0xc9, 0x52, 0x7a, 0x7a, 
    0xf0, 0x4e, 0x62, 0x6b, 0x02, 0x30, 0x0a, 0x00,  0x49, 0x83, 0x72, 0x82, 0xb0, 0x65, 0xe8, 0x96, 
    0x2d, 0x4e, 0x0c, 0xff, 0x7e, 0x67, 0xf0, 0x58,  0x5a, 0x66, 0x97, 0x7a, 0xcc, 0x91, 0x02, 0x30, 
    0x0a, 0x00, 0xca, 0x53, 0x79, 0x51, 0x51, 0x59,  0x7d, 0x5e, 0xdd, 0x7e, 0x0c, 0xff, 0xea, 0x81, 
    0xb3, 0x8d, 0x61, 0x83, 0xc3, 0x5f, 0x33, 0x80,  0x02, 0x30, 0x0a, 0x00, 0x7d, 0x86, 0xe0, 0x65, 
    0xbe, 0x5b, 0x3b, 0x4e, 0x0f, 0x61, 0x0c, 0xff,  0x87, 0x98, 0x97, 0x5f, 0x05, 0x6e, 0xc0, 0x51, 
    0x06, 0x74, 0x02, 0x30, 0x0a, 0x00, 0x74, 0x51,  0x3d, 0x5c, 0xb9, 0x65, 0x0b, 0x4e, 0x71, 0x5c, 
    0x0c, 0xff, 0x55, 0x4f, 0xc5, 0x5f, 0x85, 0x5f,  0x4b, 0x4e, 0x50, 0x5b, 0x02, 0x30, 0x0a, 0x00, 
};

int main(int argc, char *argv[])
{
    static const char space[] = " \f\t\n\r\v";
    char a[] = "   Hello Wor ld  ";
    char b[] = "Hello Wor ld  ";
    char c[] = "  Hello Wor ld";
    char d[] = "  HelloWorld  ";
    char e[] = "HelloWorld  ";
    char f[] = "HelloWorld";
    char g[] = "  ";
    char list[50] = "1,23,4,5,6,,78,,,9,,";
    char list_a[100] = "123,456,789,111,222,333,444,55,66,77,88";
    char list_b[100] = "123,456,789,111,222,333,444,,,11,22,9999,8888,hello,  33";
    char **array;
    int i, cnt, res;

    tiny_hex_dump(0, space, strlen(space));
    printf("FROM UTF16BE to UTF8:\n================================\n");
    printf("%s\n", tiny_utf_to_utf8(utf16be, sizeof(utf16be), UTF_CODING_UTF16BE));
    printf("FROM UTF16LE to UTF8:\n================================\n");
    printf("%s\n", tiny_utf_to_utf8(utf16le, sizeof(utf16le), UTF_CODING_UTF16LE));

    printf("trim NONE:\n ==============================================\n");
    printf("\"%s\"\n", tiny_string_trim(a, NULL, 0));
    printf("\"%s\"\n", tiny_string_trim(b, NULL, 0));
    printf("\"%s\"\n", tiny_string_trim(c, NULL, 0));
    printf("\"%s\"\n", tiny_string_trim(d, NULL, 0));
    printf("\"%s\"\n", tiny_string_trim(e, NULL, 0));
    printf("\"%s\"\n", tiny_string_trim(f, NULL, 0));
    printf("\"%s\"\n", tiny_string_trim(g, NULL, 0));

    printf("trim ALL:\n ==============================================\n");
    printf("\"%s\"\n", tiny_string_trim(a, NULL, TRIM_ALL));
    printf("\"%s\"\n", tiny_string_trim(b, NULL, TRIM_ALL));
    printf("\"%s\"\n", tiny_string_trim(c, NULL, TRIM_ALL));
    printf("\"%s\"\n", tiny_string_trim(d, NULL, TRIM_ALL));
    printf("\"%s\"\n", tiny_string_trim(e, NULL, TRIM_ALL));
    printf("\"%s\"\n", tiny_string_trim(f, NULL, TRIM_ALL));
    printf("\"%s\"\n", tiny_string_trim(g, NULL, TRIM_ALL));

    printf("trim FRONT and END:\n ==============================================\n");
    printf("\"%s\"\n", tiny_string_trim(a, NULL, TRIM_FRONT | TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(b, NULL, TRIM_FRONT | TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(c, NULL, TRIM_FRONT | TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(d, NULL, TRIM_FRONT | TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(e, NULL, TRIM_FRONT | TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(f, NULL, TRIM_FRONT | TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(g, NULL, TRIM_FRONT | TRIM_END));

    printf("trim MIDDLE:\n ==============================================\n");
    printf("\"%s\"\n", tiny_string_trim(a, NULL, TRIM_MIDDLE));
    printf("\"%s\"\n", tiny_string_trim(b, NULL, TRIM_MIDDLE));
    printf("\"%s\"\n", tiny_string_trim(c, NULL, TRIM_MIDDLE));
    printf("\"%s\"\n", tiny_string_trim(d, NULL, TRIM_MIDDLE));
    printf("\"%s\"\n", tiny_string_trim(e, NULL, TRIM_MIDDLE));
    printf("\"%s\"\n", tiny_string_trim(f, NULL, TRIM_MIDDLE));
    printf("\"%s\"\n", tiny_string_trim(g, NULL, TRIM_MIDDLE));

    printf("trim END:\n ==============================================\n");
    printf("\"%s\"\n", tiny_string_trim(a, NULL, TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(b, NULL, TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(c, NULL, TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(d, NULL, TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(e, NULL, TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(f, NULL, TRIM_END));
    printf("\"%s\"\n", tiny_string_trim(g, NULL, TRIM_END));

    printf("split:\n ==============================================\n");
    printf("\"%s\"\n", list);
    array = tiny_string_list_split(list, NULL, &cnt);
    printf("splt:cnt:%d\n", cnt);
    for(i = 0; i < cnt; i++)
        printf("split:\"%s\"\n", array[i]);
    free(array);

    printf("insert: ===========================================\n");
    printf("\"%s\"\n", list);
    res = tiny_string_list_insert(list, NULL, sizeof(list), "911");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_insert(list, NULL, sizeof(list), "911");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_insert(list, NULL, sizeof(list), "23");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_insert(list, NULL, sizeof(list), "5");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_insert(list, NULL, 28, "555");
    printf("%d:\"%s\"\n", res, list);


    printf("find: ===========================================\n");
    printf("\"%s\"\n", list);
    res = tiny_string_list_find(list, NULL, "911");
    printf("%d:%s:\"%s\"\n", res, "911", list);
    res = tiny_string_list_find(list, NULL, "78");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_find(list, NULL, "9");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_find(list, NULL, "5555");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_find(list, NULL, "11");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_find(list, NULL, "23");
    printf("%d:\"%s\"\n", res, list);

    printf("remove: ===========================================\n");
    printf("\"%s\"\n", list);
    res = tiny_string_list_remove(list, NULL, "911");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_remove(list, NULL, "78");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_remove(list, NULL, "9");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_remove(list, NULL, "555");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_remove(list, NULL, "1");
    printf("%d:\"%s\"\n", res, list);
    res = tiny_string_list_remove(list, NULL, "6");
    printf("%d:\"%s\"\n", res, list);


    printf("concat: ===========================================\n");
    printf("\"A: %s\"\n", list_a);
    printf("\"A: %s\"\n", list_b);

    res = tiny_string_list_concat(list_a, NULL, sizeof(list_a), list_b);
    printf("\"O: %s\" %d\n", list_a, res);
    return 0;
}

