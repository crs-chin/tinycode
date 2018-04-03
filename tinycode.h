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

/* basic */
extern int dehex(char c);
extern char *dehex_string(const char *str, int *len);
extern void hex_dump(int tabs, char *_val, int len);

/* UTF handling */
extern int utf_convert(int from, void **in, size_t *in_sz,
                       int to, void **out, size_t *out_sz);
extern int utf_convert_name(const char *from, void **in, size_t *in_sz,
                            const char *to, void **out, size_t *out_sz);
extern char *utf_as_utf8(char *text, int len, int coding);

/* GSM/CDMA coding handling */
extern char *decode_ucs16be(unsigned char *txt, int len);
extern char *decode_unicode(unsigned char *pdu, int len, int bitoffset);

extern char *decode_asc7bit_packed(unsigned char *pdu, int septets, int bitoffset);
extern char *decode_asc7bit_unpacked(unsigned char *pdu, int septets, int bitoffset);

extern char *decode_ip_addr(unsigned char *pdu, int bitoffset);

extern char *decode_gsm7bit_packed(unsigned char *pdu, int septets, int padingbits);
extern char *decode_gsm8bit_unpacked(unsigned char *pdu, int len);

extern char *decode_ucs2(char *pdu, char base, int len);
extern char *decode_adn(unsigned char *pdu, int len);

extern int decode_bcd(unsigned char pdu);
extern int decode_bcd_cdma(unsigned char pdu);

extern unsigned char *decode_bcd_num(unsigned char *pdu, int sz);
extern unsigned char *decode_bcd_num_cdma(unsigned char *pdu, int sz, int bitoffset);

#endif  /* ! __TINYCODE_H */
