/*
 * UTF/GSM/CDMA encoding conversion utils.
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
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "tinycode.h"

typedef struct _utf_coding utf_coding;
typedef int (*utf_encode)(void **buf, size_t *size, unsigned int code_point);
typedef int (*utf_decode)(void **buf, size_t *size, unsigned int *code_point);

struct _utf_coding{
    int coding;
    char *name;
    utf_encode encode;
    utf_decode decode;
};

static int utf_encode_8(void **buf, size_t *size, unsigned int cp);
static int utf_encode_16be(void **buf, size_t *size, unsigned int cp);
static int utf_encode_16le(void **buf, size_t *size, unsigned int cp);

static int utf_decode_8(void **buf, size_t *size, unsigned int *cp);
static int utf_decode_16be(void **buf, size_t *size, unsigned int *cp);
static int utf_decode_16le(void **buf, size_t *size, unsigned int *cp);

static const utf_coding utf_coding_table[] = {
    {UTF_CODING_UTF8, "UTF8", utf_encode_8, utf_decode_8,},
    {UTF_CODING_UTF16BE, "UTF16BE", utf_encode_16be, utf_decode_16be,},
    {UTF_CODING_UTF16LE, "UTF16LE", utf_encode_16le, utf_decode_16le,},
};

static const char bcd_tbl[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'a', 'b', 'c', 
};


static const char cdma_bcd_tbl[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '*', '#',
};

static const char *gsm_alphabet[] = {
    "@", "£", "$", "¥", "è", "é", "ù", "ì", "ò", "Ç", "\n", "Ø", "ø", "\r", "Å", "å", /* 0x00 */
    "Δ", "_", "Φ", "Γ", "Λ", "Ω", "Π", "Ψ", "Σ", "Θ", "Ξ", "\x1B", "Æ", "æ", "ß", "É", /* 0x10 */
    " ", "!", "\"", "#", "¤", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", /* 0x20 */
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", /* 0x30 */
    "¡", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", /* 0x40 */
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Ä", "Ö", "Ñ", "Ü", "§", /* 0x50 */
    "¿", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", /* 0x60 */
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "ä", "ö", "ñ", "ü", "à", /* 0x70 */
};

static const char *gsm_alphabet_ex[128] = {
    [0x0A] = "\n",
    [0x14] = "^",
    [0x28] = "{", [0x29] = "}", [0x2F] = "\\",
    [0x3C] = "[", [0x3D] = "~", [0x3E] = "]",
    [0x40] = "|",
    [0x60] = "€",
};

static const char *single_shift_Turkish[] = {
    [0x14] = "^",
    [0x28] = "{", [0x29] = "}", [0x2F] = "\\",
    [0x3C] = "[", [0x3D] = "~", [0x3E] = "]",
    [0x40] = "|", [0x47] = "Ğ", [0x49] = "İ",
    [0x53] = "Ş",
    [0x63] = "ç", [0x67] = "ğ", [0x69] = "ı",
    [0x73] = "ş"
};

static const char *locking_shift_Turkish[] = {
    "@", "£", "$", "¥", "€", "é", "ù", "ı", "ò", "Ç", "\n", "Ğ", "ğ", "\r", "Å", "å",
    "Δ", "_", "Φ", "Γ", "Λ", "Ω", "Π", "Ψ", "Σ", "Θ", "Ξ", "\x1B", "Ş", "ş", "ß", "É",
    " ", "!", "\"", "#", "¤", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", 
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", 
    "İ", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Ä", "Ö", "Ñ", "Ü", "§",
    "ç", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "ä", "ö", "ñ", "ü", "à", 
};

static const char *single_shift_Spanish[] = {
    [0x09] = "ç",
    [0x14] = "^",
    [0x28] = "{", [0x29] = "}", [0x2F] = "\\",
    [0x3C] = "[", [0x3D] = "~", [0x3E] = "]",
    [0x40] = "|", [0x41] = "Á", [0x49] = "Í", [0x4F] = "Ó",
    [0x55] = "Ú",
    [0x61] = "á", [0x69] = "í", [0x6F] = "ó",
    [0x75] = "ú",
};

static const char *single_shift_Portuguese[] = {
    [0x05] = "ê", [0x09] = "ç", [0x0B] = "Ô", [0x0C] = "ô", [0x0E] = "Á", [0x0F] = "á", 
    [0x12] = "Φ", [0x13] = "Γ", [0x14] = "^", [0x15] = "Ω", [0x16] = "Π", [0x17] = "Ψ", [0x18] = "Σ", [0x19] = "Θ", [0x1F] = "Ê", 
    [0x28] = "{", [0x29] = "}", [0x2F] = "\\",
    [0x3C] = "[", [0x3D] = "~", [0x3E] = "]", 
    [0x41] = "|", [0x42] = "À", [0x49] = "Í", [0x4F] = "Ó", 
    [0x55] = "Ú", [0x5B] = "Ã", [0x5C] = "Õ", 
    [0x61] = "Â", [0x65] = "€", [0x69] = "í", [0x6F] = "ó", 
    [0x75] = "ú", [0x7B] = "ã", [0x7C] = "õ", [0x7F] = "â", 
};

static const char *locking_shift_Portuguese[] = {
 	"@", "£", "$", "¥", "ê", "é", "ú", "í", "ó", "ç", "\n", "Ô", "ô", "\r", "Á", "á",
    "Δ", "_", "ª", "Ç", "À", "∞", "^", "\\", "€", "Ó", "|", "\x1B", "Â", "â", "Ê", "É", 
    " ", "!", "\"", "#", "º", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "Í", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Ã", "Õ", "Ú", "Ü", "§",
    "~", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "ã", "õ", "`", "ü", "à", 
};
#define SHIFT_TABLE(idx,lang)                                           \
    __SHIFT_TABLE(idx,lang,single_shift_##lang,locking_shift_##lang)

#define SHIFT_TABLE_GSM7BIT(idx,lang)                       \
    __SHIFT_TABLE(idx,lang,gsm_alphabet_ex,gsm_alphabet)

#define __SHIFT_TABLE(idx,lang,single,locking)  \
    [idx] = {#lang, single, locking}

struct language_shift_table{
    char *name;
    const char **single;
    const char **locking;
};

#define LANG_SHIFT_GSM7BIT  0x00

static const struct language_shift_table language_shift_table[255] = {
    SHIFT_TABLE_GSM7BIT(0x00, GSM7BIT),
    SHIFT_TABLE(0x01, Turkish),
    __SHIFT_TABLE(0x02, Spanish, single_shift_Spanish, gsm_alphabet),
    SHIFT_TABLE(0x03, Portuguese),
};

#undef SHIFT_TABLE
#undef SHIFT_TABLE_GSM7BIT
#undef __SHIFT_TABLE

static const char *mon_tbl[] = {
    "Jan.", "Feb.", "Mar.", "Apr.", "May.", "Jun.", "Jul.", "Aug.", "Sep.", "Oct.", "Nov.", "Dec.",
};

#ifndef ARRAYSIZE
 #define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#endif

#ifndef BUILD_FAIL_IF
 #define BUILD_FAIL_IF(exp) ((void)sizeof(char[1 - 2 * (!!(exp))]))
#endif

#ifndef _GNU_SOURCE
#define __INIT_STRING_LEN 100

static int asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    char *buf;
    size_t len = __INIT_STRING_LEN;
    int res;

    if(! strp || ! fmt || ! (buf = (char *)malloc(len)))
        return -1;

    va_start(ap, fmt);
    do {
        res = vsnprintf(buf, len, fmt, ap);

        if(res >= len) {
            free(buf);

            if(! (buf = (char *)malloc(len + __INIT_STRING_LEN))) {
                res = -1;
                break;
            }

            len += __INIT_STRING_LEN;
            va_end(ap);
            va_start(ap, fmt);
            continue;
        }
    } while(0);
    va_end(ap);

    *strp = buf;
    return res;
}

static int vasprintf(char **strp, const char *fmt, va_list _ap)
{
    va_list ap;
    char *buf;
    size_t len = __INIT_STRING_LEN * 10;
    int res;

    if(! strp || ! fmt || ! (buf = (char *)malloc(len)))
        return -1;

    va_copy(ap, _ap);
    do {
        res = vsnprintf(buf, len, fmt, ap);
        if(res >= len) {
            free(buf);

            if(! (buf = (char *)malloc(len + __INIT_STRING_LEN))) {
                res = -1;
                break;
            }

            len += __INIT_STRING_LEN;
            va_end(ap);
            va_copy(ap, _ap);
            continue;
        }
    } while(0);
    va_end(ap);

    *strp = buf;
    return res;
}
#endif

int tiny_decode_hex(char c)
{
    if(c >= '0' && c <= '9')
        return c - '0';
    if(c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    if(c >= 'A' && c <= 'F')
        return 10 + c - 'A';
    return -1;
}

char *tiny_decode_hex_string(const char *str, int *len)
{
    const char *p;
    char *bin, *q;
    int l;

    if(! str || ! *str || ! len)
        return NULL;

    l = strlen(str) / 2;
    if(! l)
        return NULL;

    *len = l;
    bin = (char *)malloc(l);
    if(! bin)
        return NULL;

    for(p = str, q = bin; l; l--)  {
        *q++ = ((tiny_decode_hex(*p) << 4) | tiny_decode_hex(*(p + 1)));
        p += 2;
    }

    return bin;
}


void tiny_hex_dump(int tabs, const char *val, int len)
{
    const unsigned char *p = (const unsigned char *)val;
    int i, j;

    for(i = 0; i < len;)  {
        if(! (i % 16))  {
            for(j = 0; j < tabs; j++)
                printf("\t");
            printf("%-.04X-%.04X    ", i, i + 15);
        }

        if(! (i % 8) && (i % 16))
            printf("  ");

        switch(len - i)  {
        default:
            printf("%02X %02X %02X %02X ", p[i], p[i + 1], p[i + 2], p[i + 3]);
            i += 4;
            break;
        case 2:
        case 3:
            printf("%02X %02X ", p[i], p[i + 1]);
            i += 2;
            break;
        case 1:
            printf("%02X", p[i]);
            i += 1;
            break;
        }

        if(! (i % 16))
            printf("\n");
    }

    if(i % 16)
        printf("\n");
}

static int utf_encode_8(void **buf, size_t *size, unsigned int cp)
{
    unsigned char *p = *buf;
    size_t sz = 0;
    int err = UTF_ERR_OK;

    if(cp > 0x10FFFF)          /* currently UCS stops at 0x10FFFF */
        return UTF_ERR_BAD_CODE;

    if(cp >= 0xD800 && cp <= 0xDFFF) /* fail if fall in UTF16 surrogates */
        return UTF_ERR_BAD_CODE;

    while(*size > 0) {
        if(cp <= 0x7F) {
            *p = cp;
            sz = 1;
            break;
        }

        if(cp <= 0x7FF) {
            if(*size < 2) {
                err = UTF_ERR_SIZE;
                break;
            }

            *p++ = ((cp >> 6) & 0x1F) | 0xC0;
            *p = (cp & 0x3F) | 0x80;
            sz = 2;
            break;
        } else if(cp <= 0xFFFF) {
            if(*size < 3) {
                err = UTF_ERR_SIZE;
                break;
            }

            *p++ = ((cp >> 12) & 0x0F) | 0xE0;
            *p++ = ((cp >> 6) & 0x3F) | 0x80;
            *p = (cp & 0x3F) | 0x80;
            sz = 3;
            break;
        } else if(cp <= 0x1FFFFF) {
            if(*size < 4) {
                err = UTF_ERR_SIZE;
                break;
            }

            *p++ = ((cp >> 18) & 0x07) | 0xF0;
            *p++ = ((cp >> 12) & 0x3F) | 0x80;
            *p++ = ((cp >> 6) & 0x3F) | 0x80;
            *p = (cp & 0x3F) | 0x80;
            sz = 4;
            break;
        }

        err = UTF_ERR_BAD_CODE;
        break;
    }

    *buf = (char *)*buf + sz;
    *size -= sz;
    return err;
}

static inline int __big_endian()
{
    int i = 1;
    return ! *(char *)&i;
}

static void __write_be(void *buf, unsigned short val)
{
    if(__big_endian()) {
        *(unsigned short *)buf = val;
    } else {
        unsigned char *a = (unsigned char *)buf;

        *a = val >> 8;
        *(a + 1) = val & 0x0F;
    }
}

static void __write_le(void *buf, unsigned short val)
{
    if(__big_endian()) {
        unsigned char *a = buf;

        *a = val & 0x0F;
        *(a + 1) = val >> 8;
    } else {
        *(unsigned short *)buf = val;
    }
}

static int utf_encode_16(void (*write_short)(void *, unsigned short),
                         void **buf, size_t *size, unsigned int cp)
{
    unsigned short *p = (unsigned short *)*buf;
    size_t sz = 0;
    int err = UTF_ERR_OK;

    if(cp >= 0xD800 && cp <= 0xDFFF) /* fail if fall in UTF16 surrogates */
        return UTF_ERR_BAD_CODE;

    do{
        if(cp < 0x010000) {
            if(*size < 2) {
                err = UTF_ERR_SIZE;
                break;
            }

            write_short(p, cp);
            sz = 2;
            break;
        }

        if(cp > 0x10FFFF) {          /* currently UCS stops at 0x10FFFF */
            err = UTF_ERR_BAD_CODE;
            break;
        }

        if(*size < 4) {
            err = UTF_ERR_SIZE;
            break;
        }

        cp -= 0x010000;
        write_short(p, cp >> 10);
        write_short(p + 1, cp & 0x3FF);
        sz = 4;
        break;
    }while (0);

    *buf = (char *)*buf + sz;
    *size -= sz;
    return err;
}

static int utf_encode_16be(void **buf, size_t *size, unsigned int cp)
{
    return utf_encode_16(__write_be, buf, size, cp);
}

static int utf_encode_16le(void **buf, size_t *size, unsigned int cp)
{
    return utf_encode_16(__write_le, buf, size, cp);
}

static int utf_decode_8(void **buf, size_t *size, unsigned int *cp)
{
    unsigned char *p = *buf;
    size_t sz = 0;
    int err = UTF_ERR_OK;

    while(*size > 0) {
        if(! (*p >> 7)) {       /* ascii code */
            *cp = *p;
            sz = 1;
            break;
        }

#define __UTF8_ACCUM(_p)                                    \
        if((*++(_p) >> 6) != 0x02) {  /* 10XXXXXX format */ \
            err = UTF_ERR_BAD_CODE;                         \
            break;                                          \
        }                                                   \
        *cp = (*cp << 6) | (*(_p) & 0x3F);

        if((*p >> 5) == 0x06) { /* 110XXXXX, 2 byte case */
            if(*size < 2) {
                err = UTF_ERR_INCOMPLETE;
                break;
            }

            *cp = *p & 0x1F;

            __UTF8_ACCUM(p);

            sz = 2;
            break;
        } else if((*p >> 4) == 0x0E) { /* 1110XXXX, 3 byte case */
            if(*size < 3) {
                err = UTF_ERR_INCOMPLETE;
                break;
            }

            *cp = *p & 0x0F;

            __UTF8_ACCUM(p);
            __UTF8_ACCUM(p);

            sz = 3;
            break;
        } else if((*p >> 3) == 0xF0) { /* 11110XXX, 4 byte case */
            if(*size < 4) {
                err = UTF_ERR_INCOMPLETE;
                break;
            }

            *cp = *p & 0x07;

            __UTF8_ACCUM(p);
            __UTF8_ACCUM(p);
            __UTF8_ACCUM(p);

            sz = 4;
            break;
        }
#undef __UTF8_ACCUM

        err = UTF_ERR_BAD_CODE;
        break;
    }

    /* skip surrogates reserved for UTF16, and check UCS tops */
    if(err == UTF_ERR_OK &&
       ((*cp >= 0xD800 && *cp <= 0xDFFF) || *cp > 0x10FFFF)) {
        err = UTF_ERR_BAD_CODE;
    } else {
        *buf = (char *)*buf + sz;
        *size -= sz;
    }
    return err;
}

static unsigned short __read_be(void *buf)
{
    if(__big_endian())
        return *(unsigned short *)buf;

    unsigned char *a = buf;

    return (*a << 8 | *(a + 1));
}

static unsigned short __read_le(void *buf)
{
    if(__big_endian()) {
        unsigned char *a = buf;

        return (*a | *(a + 1) << 8);
    }

    return *(unsigned short *)buf;
}

static int utf_decode_16(unsigned short (*read_short)(void *),
                         void **buf, size_t *size, unsigned int *cp)
{
    unsigned short _cp;
    size_t sz = 0;
    int err = UTF_ERR_OK;

    if(*size < 2)
        return UTF_ERR_BAD_CODE;

    do{
        _cp = read_short(*buf);
        if(_cp < 0xD800 || _cp > 0xDFFF) { /* BMP plane */
            *cp = _cp;
            sz = 2;
            break;
        }

        /* UTF16/UCS16 extensions */
        if(_cp > 0xDBFF) {       /* high surrogate expected */
            err = UTF_ERR_BAD_CODE;
            break;
        }
        *cp = (_cp - 0xD800) << 10;

        if(_cp < 0xDC00) {       /* low surrogate expected */
            err = UTF_ERR_BAD_CODE;
            break;
        }
        *cp |= (_cp - 0xDC00);

        *cp += 0x010000;

        sz = 4;
        break;
    }while(0);

    *buf = (char *)*buf + sz;
    *size -= sz;
    return err;
}

static int utf_decode_16be(void **buf, size_t *size, unsigned int *cp)
{
    return utf_decode_16(__read_be, buf, size, cp);
}

static int utf_decode_16le(void **buf, size_t *size, unsigned int *cp)
{
    return utf_decode_16(__read_le, buf, size, cp);
}

static int utf_do_convert(const utf_coding *from, void **in, size_t *in_sz,
                          const utf_coding *to, void **out, size_t *out_sz)
{
    unsigned int code_point;
    int err;

    if(! in || ! in_sz || ! out || ! out_sz)
        return UTF_ERR_BAD_ARG;

    while(*in_sz > 0) {
        if((err = from->decode(in, in_sz, &code_point)))
            break;
        if((err = to->encode(out, out_sz, code_point)))
            break;
    }

    return err;
}

static const utf_coding *utf_coding_get(int coding, const char *name)
{
    unsigned int i;

    if(coding != UTF_CODING_INVALID) {
        for(i = 0; i < ARRAYSIZE(utf_coding_table); i++) {
            if(coding == utf_coding_table[i].coding) {
                return &utf_coding_table[i];
            }
        }
    }

    if(name && name[0]) {
        for(i = 0; i < ARRAYSIZE(utf_coding_table); i++) {
            if(! strcasecmp(name, utf_coding_table[i].name)) {
                return &utf_coding_table[i];
            }
        }
    }

    return NULL;
}

int tiny_utf_convert(int from, void **in, size_t *in_sz,
                     int to, void **out, size_t *out_sz)
{
    const utf_coding *fcoding = utf_coding_get(from, NULL);
    const utf_coding *tcoding = utf_coding_get(to, NULL);

    if(! fcoding || ! tcoding)
        return UTF_ERR_NO_SUPPORT;

    return utf_do_convert(fcoding, in, in_sz, tcoding, out, out_sz);
}

int tiny_utf_convert_name(const char *from, void **in, size_t *in_sz,
                          const char *to, void **out, size_t *out_sz)
{
    const utf_coding *fcoding = utf_coding_get(UTF_CODING_INVALID, from);
    const utf_coding *tcoding = utf_coding_get(UTF_CODING_INVALID, to);

    if(! fcoding || ! tcoding)
        return UTF_ERR_NO_SUPPORT;

    return utf_do_convert(fcoding, in, in_sz, tcoding, out, out_sz);
}

char *tiny_utf_to_utf8(const char *text, int len, int coding)
{
    const char *inbuf;
    char *outbuf, *str;
    size_t in, sz, out, res;

    if(len < 0)
        in = strlen(text);
    else
        in = (size_t)len;
    sz = out = len = in;

    str = (char *)malloc(sz + 1);
    if(! str)
        return NULL;

    for(inbuf = text, outbuf = str;;)  {
        res = tiny_utf_convert(coding, (void **)&inbuf, &in,
                               UTF_CODING_UTF8, (void **)&outbuf, &out);
        if(res == UTF_ERR_SIZE)  {
            out += len;
            sz += len;
            str = (char *)realloc(str, sz + 1);
            if(! str)  {
                printf("OOM converting encoding!\n");
                break;
            }
            outbuf = str + sz - out;
            continue;
        }
        break;
    }

    str[sz - out] = '\0';
    return str;
}

char *tiny_decode_ucs16be(const unsigned char *txt, int len)
{
    const unsigned short *ucs16 = (const unsigned short *)txt;

    /* skipping ending 0xFFFF */
    for(len /= 2; len >= 1 && ucs16[len - 1] == 0xFFFF; len--);
    if(! len)
        return strdup("");
    return tiny_utf_to_utf8((const char *)ucs16, len * 2, UTF_CODING_UTF16BE);
}


char *tiny_decode_unicode(const unsigned char *pdu, int len, int bitoffset)
{
    unsigned short *ucs16, *p;
    unsigned int i, j, c, charoffset, shift;
    char *txt = NULL;

    p = ucs16 = (unsigned short *)malloc(len * sizeof(unsigned short));
    if(p)  {
        for(i = 0, j = len, charoffset = bitoffset / 8, shift = bitoffset % 8;
            j;
            j--, charoffset += 2)  {

            c = (pdu[charoffset] & ((1 << (8 - shift)) - 1));
            c = ((c << 8) | pdu[charoffset + 1]);
            if(shift > 0)
                c = ((c << shift) | (pdu[charoffset + 2] >> (8 - shift)));

            p[i++] = c;
        }
        txt = tiny_utf_to_utf8((const char *)ucs16, len * 2, UTF_CODING_UTF16);
        free(ucs16);
    }
    return txt;
}


char *tiny_decode_asc7bit_packed(const unsigned char *pdu, int septets, int bitoffset)
{
    char *str;
    unsigned int i, c, charoffset, shift;

    str = (char *)malloc(septets + 1);
    if(! str)  {
        printf("OOM allocating str:%d!\n", septets + 1);
        return NULL;
    }

    for(i = 0, charoffset = bitoffset / 8, shift = bitoffset % 8;
        septets;
        septets--, bitoffset += 7, charoffset = bitoffset / 8, shift = bitoffset % 8)  {

        if(shift > 1)  {
            c = (pdu[charoffset] & ((1 << (8 - shift)) - 1));
            c = ((c << (shift - 1)) | (pdu[charoffset + 1] >> (9 - shift)));
        }else  {
            c = ((pdu[charoffset] >> (1 - shift)) & 0x7F);
        }

        if(! isprint(c))
            c = ' ';

        str[i++] = c;
    }

    str[i] = '\0';
    return str;    
}


char *tiny_decode_asc7bit_unpacked(const unsigned char *pdu, int septets, int bitoffset)
{
    unsigned char *buf, *p;
    unsigned int i, v, charoffset = bitoffset / 8, shift = bitoffset % 8;

    buf = (unsigned char *)malloc(septets + 1);
    if(buf)  {
        if(! shift)  {
            memcpy(buf, pdu + charoffset, septets);
        }else  {
            for(i = 0, p = buf; i < septets; i++, charoffset++)  {
                v = pdu[charoffset] & ((1 << (8 - shift)) - 1);
                v = ((v << shift) | ((pdu[charoffset + 1] >> (8 - shift)) & ((1 << shift) - 1)));
                *p++ = v;
            }
        }
        buf[septets] = '\0';
    }
    return buf;
}

char *tiny_decode_ip_addr(const unsigned char *pdu, int bitoffset)
{
    unsigned int v, charoffset = bitoffset / 8, shift = bitoffset % 8;
    unsigned char *buf = NULL, *p = (unsigned char *)&v;

    if(! shift)
        v = pdu[charoffset++];
    else
        v = pdu[charoffset++] & ((1 << (8 - shift)) - 1);
    v = (v << 8) | pdu[charoffset++];
    v = (v << 8) | pdu[charoffset++];
    if(! shift)
        v = (v << 8) | pdu[charoffset++];
    else
        v = (v << shift) | ((pdu[charoffset] >> (8 - shift)) & ((1 << shift) - 1));

    if(__big_endian())
        asprintf((char **)&buf, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    else
        asprintf((char **)&buf, "%d.%d.%d.%d", p[3], p[2], p[1], p[0]);

    return buf;
}

char *tiny_decode_gsm7bit_packed_ex(const unsigned char *pdu, int septets, int padingbits,
                                    int single_shift, int locking_shift)
{
    char *str, *p;
    int esc = 0, c, bitoffset, charoffset, shift;
    int i, sz = septets;
    struct language_shift_table transtbl = language_shift_table[LANG_SHIFT_GSM7BIT];

    if(single_shift > 0 && single_shift < ARRAYSIZE(language_shift_table)) {
        transtbl.single = language_shift_table[single_shift].single;
    }

    if(locking_shift > 0 && locking_shift < ARRAYSIZE(language_shift_table)) {
        transtbl.locking = language_shift_table[locking_shift].locking;
    }

    str = (char *)malloc(sz + 1);
    if(! str)  {
        printf("OOM allocating str:%d!\n", sz + 1);
        return NULL;
    }

    for(i = 0, bitoffset = padingbits, charoffset = 0, shift = padingbits;
        septets;
        septets--, bitoffset += 7, charoffset = bitoffset / 8, shift = bitoffset % 8)  {

        c = ((pdu[charoffset] >> shift) & 0x7F);
        if(shift > 1)
            c |= ((pdu[charoffset + 1] << (8 - shift)) & 0x7F);

        if(c == 0x1B)  {
            esc = 1;
            continue;
        }

        if(i + 4 > sz)  {
            sz += (septets / 2 ? : 4);
            p = (char *)realloc(str, sz + 1);
            if(! p)  {
                printf("OOM realloc str!\n");
                str[i] = '\0';
                return str;
            }
            str = p;
        }

        if(esc)  {
            /* fake a invalid escaped char as a space */
            if(! transtbl.single[c])
                str[i++] = ' ';
            else  {
                strcpy(str + i, transtbl.single[c]);
                i += strlen(transtbl.single[c]);
            }
            esc = 0;
            continue;
        }

        strcpy(str + i, transtbl.locking[c]);
        i += strlen(transtbl.locking[c]);
    }

    str[i] = '\0';
    return str;    
}

char *tiny_decode_gsm8bit_unpacked_ex(const unsigned char *pdu, int len,
                                      int single_shift, int locking_shift)
{
    char *str, *p;
    int esc, c, i, j, sz = len;
    struct language_shift_table transtbl = language_shift_table[LANG_SHIFT_GSM7BIT];

    if(single_shift > 0 && single_shift < ARRAYSIZE(language_shift_table)) {
        transtbl.single = language_shift_table[single_shift].single;
    }

    if(locking_shift > 0 && locking_shift < ARRAYSIZE(language_shift_table)) {
        transtbl.locking = language_shift_table[locking_shift].locking;
    }

    str = (char *)malloc(sz + 1);
    if(! str)  {
        printf("OOM allocating str:%d!\n", sz + 1);
        return NULL;
    }

    for(esc = 0, i = 0, j = 0; j < len; j++)  {
        c = pdu[j] & 0x7F;
        if(c == 0x1B)  {
            esc = 1;
            continue;
        }

        if(i + 4 > sz)  {
            sz += (len / 2 ? : 4);
            p = (char *)realloc(str, sz + 1);
            if(! str)  {
                printf("OOM realloc str!\n");
                str[i] = '\0';
                return str;
            }
            str = p;
        }

        if(esc)  {
            /* fake a invalid escaped char as a space */
            if(! transtbl.single[c])
                str[i++] = ' ';
            else  {
                strcpy(str + i, transtbl.single[c]);
                i += strlen(transtbl.single[c]);
            }
            esc = 0;
            continue;
        }

        strcpy(str + i, transtbl.locking[c]);
        i += strlen(transtbl.locking[c]);
    }

    str[i] = '\0';
    return str;
}


char *tiny_decode_ucs2(const char *pdu, char base, int len)
{
    char *ret, *tmp;
    size_t sz = len;
    int i, j, m;

    ret = (char *)malloc(sz + 1);
    if(! ret)  {
        printf("OOM alloc buffer!\n");
        return NULL;
    }

    for(i = 0, j = 0; i < len;)  {
        if(j == sz)  {
            sz += len;
            ret = (char *)realloc(ret, sz);
            if(! ret)  {
                printf("OOM enlarge buffer\n");
                break;
            }
        }

        if(pdu[i] < 0)
            ret[j++] = (char)(base + (pdu[i++] & 0x7F));

        for(m = i; m < len && pdu[m] >= 0; m++)
            ;

        tmp = tiny_decode_gsm8bit_unpacked(pdu + i, m - i);
        if(tmp)  {
            int l = strlen(tmp);

            if(l > sz - j)  {
                sz += l;
                ret = (char *)realloc(ret, sz);
                if(! ret)  {
                    printf("OOM enlarge buffer\n");
                    break;
                }
            }
            strcpy(ret + j, tmp);
            j += l;
            free(tmp);
        }
        i += m;
    }

    ret[j] = '\0';
    return ret;
}

char *tiny_decode_adn(const unsigned char *pdu, int len)
{
    int i = 0, l = 0, ucs2 = 0;
    char base = '\0';

    if(! len)
        return strdup("");

    if(len >= 1 && pdu[i] == 0x80)
        return tiny_decode_ucs16be(pdu + 1, len - 1);

    if(len >= 3 && pdu[i] == 0x81)  {
        l = pdu[i + 1] & 0xff;
        if(l > len - 3)
            l = len - 3;
        base = (char)((pdu[i + 2] & 0xff) << 7);
        i += 3;
        ucs2 = 1;
    }else if(len >= 4 && pdu[i] == 0x82)  {
        l = pdu[i + 1] & 0xff;
        if(l > len - 4)
            l = len - 4;
        base = (char)(((pdu[i + 2] & 0xff) << 8) | (pdu[i + 3] & 0xff));
        i += 4;
        ucs2 = 1;
    }

    if(ucs2)
        return tiny_decode_ucs2(pdu + i, base, len - i);

    return tiny_decode_gsm8bit_unpacked(pdu, len);
}

int tiny_decode_bcd(unsigned char pdu)
{
    int ret = 0;

    if((pdu & 0xF0) <= 0x90)
        ret = (pdu >> 4) & 0x0F;
    if((pdu & 0x0F) <= 9)
        ret += (pdu & 0x0F) * 10;

    return ret;
}


int tiny_decode_bcd_cdma(unsigned char pdu)
{
    int ret = 0;

    if((pdu & 0x0F) <= 9)
        ret = pdu & 0x0F;
    if((pdu & 0xF0) <= 0x90)
        ret += ((pdu >> 4) & 0x0F) * 10;

    return ret;
}


/* FIXME:modify num according num type */
unsigned char *tiny_decode_bcd_num(const unsigned char *pdu, int sz)
{
    unsigned char *num = (unsigned char *)malloc(sz + 1), idx;
    int  i;

    if(num)  {
        for(i = 0; i < sz;)  {
            idx = pdu[i / 2] & 0x0F;
            if(idx == 0x0F)
                break;
            num[i++] = bcd_tbl[idx];

            if(i == sz)
                break;

            idx = (pdu[i / 2] >> 4) & 0x0F;
            if(idx == 0x0F)
                break;
            num[i++] = bcd_tbl[idx];
        }
        num[i] = '\0';
    }

    return num;
}


unsigned char *tiny_decode_bcd_num_cdma(const unsigned char *pdu, int sz, int bitoffset)
{
    const unsigned char *buf;
    unsigned char *p, *num;
    unsigned int i, v, len, charoffset = bitoffset / 8, shift = bitoffset % 8;

    if(! shift)  {
        buf = pdu + charoffset;
    }else  {
        buf = p = (unsigned char *)alloca((sz + 1) / 2);
        for(i = 0, len = (sz + 1) / 2; i < len; i++, charoffset++)  {
            v = pdu[charoffset] & ((1 << (8 - shift)) - 1);
            v = ((v << shift) | ((pdu[charoffset + 1] >> (8 - shift)) & ((1 << shift) - 1)));
            *p++ = v;
        }
    }

    num = (unsigned char *)malloc(sz + 1);
    if(num)  {
        for(i = 0; i < sz;)  {
            v = ((buf[i / 2] >> 4) & 0x0F) - 1;
            num[i++] = (v < ARRAYSIZE(cdma_bcd_tbl)) ? cdma_bcd_tbl[v] : 'x';

            if(i == sz)
                break;

            v = (buf[i / 2] & 0x0F) - 1;
            num[i++] = (v < ARRAYSIZE(cdma_bcd_tbl)) ? cdma_bcd_tbl[v] : 'x';
        }
        num[i] = '\0';
    }

    return num;
}

char *tiny_string_trim(char *string, const char *junk, int flag)
{
    const char *_junk = " \f\t\n\r\v";
    char *s = string, *p = string, *_e, *e;

    if(! string || ! string[0])
        return s;

    if(! junk || ! junk[0])
        junk = _junk;

    if(! (flag & TRIM_IN_PLACE))
        string = strdup(string);

    s = p = string;

    if(! string || ! (flag & TRIM_ALL))
        return string;

    while(*p && strchr(junk, *p))
        p++;

    if(! (flag & TRIM_FRONT)) {
        s = p;
    }

    /* last non-junk char */
    for(e = _e = p; *_e; _e++) {
        if(! strchr(junk, *_e))
            e = _e;
    }

    if(flag & TRIM_MIDDLE) {
        while(*p && p <= e) {
            if(! strchr(junk, *p)) {
                *s++ = *p++;
                continue;
            }
            p++;
        }
    } else {
        while(*p && p <= e)
            *s++ = *p++;
    }

    if(! (flag & TRIM_END)) {
        while(*p)
            *s++ = *p++;
    }
    *s = '\0';

    return string;
}

char **tiny_string_list_split(const char *list, const char *delim, int *num)
{
    const char *_delim = ",", *p, *_p;
    char **array, **item, *content;
    unsigned int l, len;
    int cnt;

    if(! list || ! list[0])
        return NULL;

    if(! delim || ! delim[0])
        delim = _delim;

    for(cnt = 0, p = _p = list, len = strlen(delim); _p && *p; p = _p + len) {
        if(p != (_p = strstr(p, delim)))
            cnt++;
    }

    /* reside in a single block with last NULL item */
    if(! (array = (char **)malloc(strlen(list) + sizeof(char *) * (cnt + 1))))
        return NULL;

    item = array;
    content = (char *)(array + cnt + 1);
    for(p = _p = list; _p && *p; p = _p + len) {
        if(p != (_p = strstr(p, delim))) {
            if(_p) {
                l = _p - p;
                *item++ = strncpy(content, p, l);
                content += l;
                *content++ = '\0';
            } else {
                *item++ = strcpy(content, p);
            }
        }
    }

    *item = NULL;
    if(num)
        *num = cnt;

    return array;
}

/*
 * @size: list capacity, including terminating '\0'
 */
int tiny_string_list_insert(char *list, const char *delim, unsigned int size, const char *item)
{
    const char *_delim = ",", *p, *_p;
    unsigned int dlen, len;

    if(! list || ! item || ! item[0])
        return -1;

    if(! delim || ! delim[0])
        delim = _delim;

    for(p = _p = list, dlen = strlen(delim); _p && *p; p = _p + dlen) {
        if(p != (_p = strstr(p, delim))) {
            if((! _p && ! strcmp(item, p)) ||
               (_p && (_p - p) == strlen(item) && ! strncmp(item, p, _p - p)))
                return 0;
        }
    }

    len = strlen(list) + strlen(item) + 1;
    /* list not ending with delim */
    if(! _p)
        len += dlen;

    if(len > size)
        return -1;

    if(! _p)
        strcat(list, delim);
    strcat(list, item);
    return 0;
}

int tiny_string_list_remove(char *list, const char *delim, const char *item)
{
    const char *_delim = ",";
    unsigned int dlen, len;
    char *p, *_p;

    if(! list || ! item || ! item[0])
        return -1;

    if(! delim || ! delim[0])
        delim = _delim;

    for(p = _p = list, dlen = strlen(delim); _p && *p; p = _p + dlen) {
        if(p != (_p = strstr(p, delim))) {
            if(! _p && ! strcmp(item, p)) {
                while(p > list && ! strncmp(p - dlen, delim, dlen))
                    p -= dlen;

                *p = '\0';
                return 0;
            }

            if(_p && (_p - p) == strlen(item) && ! strncmp(item, p, _p - p)) {
                while(! strncmp(_p, delim, dlen))
                    _p += dlen;

                /* remove heading delim if no trailing item */
                if(! *_p) {
                    while(p > list && ! strncmp(p - dlen, delim, dlen))
                        p -= dlen;
                }

                while(*_p)
                    *p++ = *_p++;
                *p = '\0';
                return 0;
            }
        }
    }
    return 0;
}

int tiny_string_list_find(char *list, const char *delim, const char *item)
{
    const char *_delim = ",", *p, *_p;
    unsigned int dlen, len;

    if(! list || ! item || ! item[0])
        return -1;

    if(! delim || ! delim[0])
        delim = _delim;

    for(p = _p = list, dlen = strlen(delim); _p && *p; p = _p + dlen) {
        if(p != (_p = strstr(p, delim))) {
            if((! _p && ! strcmp(item, p)) ||
               (_p && (_p - p) == strlen(item) && ! strncmp(item, p, _p - p)))
                return 0;
        }
    }

    return -1;
}

int tiny_string_list_concat(char *list, const char *delim, unsigned int size, const char *add)
{
    char **arr, **p;
    const char *_delim = ",";
    int res = 0;

    if(! list || ! add)
        return -1;

    if((p = arr = tiny_string_list_split(add, delim, NULL))) {
        while(*p) {
            if((res = tiny_string_list_insert(list, delim, size, *p++))) break;
        }
        free(arr);
    }
    return res;
}

static __attribute__((unused)) void __build_check(void)
{
    BUILD_FAIL_IF(128 != ARRAYSIZE(gsm_alphabet));
}

