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

#ifndef __TINYCODE_H
#define __TINYCODE_H

#define UTF_CODING_INVALID  (-1)
#define UTF_CODING_UTF8     0
#define UTF_CODING_UTF16BE  1
#define UTF_CODING_UTF16LE  2
#define UTF_CODING_UTF32    3

#if __BYTE_ORDER == __BIG_ENDIAN
 #define UTF_CODING_UTF16    UTF_CODING_UTF16BE
#else
 #define UTF_CODING_UTF16    UTF_CODING_UTF16LE
#endif

#define UTF_ERR_OK          0
#define UTF_ERR_BAD_ARG     (-1)
#define UTF_ERR_INCOMPLETE  (-2)
#define UTF_ERR_BAD_CODE    (-3)
#define UTF_ERR_SIZE        (-4)
#define UTF_ERR_NO_SUPPORT  (-5)

#define LANG_SHIFT_GSM7BIT      0x00
#define LANG_SHIFT_TURKISH      0x01
#define LANG_SHIFT_SPANISH      0x02
#define LANG_SHIFT_PORTUGUESE   0x03


/* basic */
extern int tiny_decode_hex(char c);
extern char *tiny_decode_hex_string(const char *str, int *len);
extern char *tiny_encode_hex_string(const char *str, int len);
extern void tiny_hex_dump(int tabs, const char *val, int len);

/* UTF handling */
extern int tiny_utf_convert(int from, void **in, size_t *in_sz,
                            int to, void **out, size_t *out_sz);
extern int tiny_utf_convert_name(const char *from, void **in, size_t *in_sz,
                                 const char *to, void **out, size_t *out_sz);
extern char *tiny_utf_to_utf8(const char *text, int len, int coding);

/* GSM/CDMA coding handling */
extern char *tiny_decode_ucs16be(const unsigned char *txt, int len);
extern char *tiny_decode_unicode(const unsigned char *pdu, int len, int bitoffset);

extern char *tiny_decode_asc7bit_packed(const unsigned char *pdu, int septets, int bitoffset);
extern char *tiny_decode_asc7bit_unpacked(const unsigned char *pdu, int septets, int bitoffset);

extern char *tiny_decode_ip_addr(const unsigned char *pdu, int bitoffset);

extern char *tiny_decode_gsm7bit_packed_ex(const unsigned char *pdu, int septets, int padingbits,
                                           int single_shift, int locking_shift);
extern char *tiny_decode_gsm8bit_unpacked_ex(const unsigned char *pdu, int len,
                                             int single_shift, int locking_shift);

static char *tiny_decode_gsm7bit_packed(const unsigned char *pdu, int septets, int padingbits)
{
    return tiny_decode_gsm7bit_packed_ex(pdu, septets, padingbits,
                                         LANG_SHIFT_GSM7BIT, LANG_SHIFT_GSM7BIT);
}

static char *tiny_decode_gsm8bit_unpacked(const unsigned char *pdu, int len)
{
    return tiny_decode_gsm8bit_unpacked_ex(pdu, len,
                                           LANG_SHIFT_GSM7BIT, LANG_SHIFT_GSM7BIT);
}


extern char *tiny_decode_ucs2(const char *pdu, char base, int len);
extern char *tiny_decode_adn(const unsigned char *pdu, int len);

extern int tiny_decode_bcd(unsigned char pdu);
extern int tiny_decode_bcd_cdma(unsigned char pdu);

extern unsigned char *tiny_decode_bcd_num(const unsigned char *pdu, int sz);
extern unsigned char *tiny_decode_bcd_num_cdma(const unsigned char *pdu, int sz, int bitoffset);

/* string utils */
#define TRIM_FRONT       1
#define TRIM_MIDDLE      (1<<1)
#define TRIM_END         (1<<2)
#define TRIM_IN_PLACE    (1<<3)
#define TRIM_ALL         (TRIM_FRONT | TRIM_MIDDLE | TRIM_END)

extern char *tiny_string_trim(char *string, const char *junk, int flag);
/* note: space isn't exluded during comparing, trim them before
   passing them into the functions */
extern char **tiny_string_list_split(const char *list, const char *delim, int *num);
extern int tiny_string_list_insert(char *list, const char *delim, unsigned int size, const char *item);
extern int tiny_string_list_remove(char *list, const char *delim, const char *item);
extern int tiny_string_list_find(char *list, const char *delim, const char *item);
extern int tiny_string_list_concat(char *list, const char *delim, unsigned int size, const char *add);
/* utils */

#endif  /* ! __TINYCODE_H */
