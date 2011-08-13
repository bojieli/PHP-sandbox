/*
 * "streamable kanji code filter and converter"
 * Copyright (c) 1998-2002 HappySize, Inc. All rights reserved.
 *
 * LICENSE NOTICES
 *
 * This file is part of "streamable kanji code filter and converter",
 * which is distributed under the terms of GNU Lesser General Public 
 * License (version 2) as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with "streamable kanji code filter and converter";
 * if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA
 *
 * The author of this file:
 *
 */
/*
 * The source code included in this files was separated from mbfilter.c
 * by rui hrokawa <hirokawa@php.net> on 8 aug 2011.
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mbfilter.h"

#include "mbfilter_utf8_mobile.h"
#include "mbfilter_sjis_mobile.h"

extern int mbfl_filt_ident_utf8(int c, mbfl_identify_filter *filter);
extern const int mbfl_docomo2uni_pua[4][3];
extern const int mbfl_kddi2uni_pua[6][3];
extern const int mbfl_sb2uni_pua[6][3];
extern const int mbfl_kddi2uni_pua_b[8][3];

extern const unsigned char mblen_table_utf8[];

static const char *mbfl_encoding_utf8_docomo_aliases[] = {"utf8-mobile#docomo", NULL};
static const char *mbfl_encoding_utf8_kddi_aliases[] = {"utf8-mobile#kddi", NULL};
static const char *mbfl_encoding_utf8_kddi_b_aliases[] = {"utf8-mobile#kddi-b", NULL};
static const char *mbfl_encoding_utf8_sb_aliases[] = {"utf8-mobile#softbank", NULL};

const mbfl_encoding mbfl_encoding_utf8_docomo = {
	mbfl_no_encoding_utf8_docomo,
	"UTF-8-Mobile#DOCOMO",
	"UTF-8",
	(const char *(*)[])&mbfl_encoding_utf8_docomo_aliases,
	mblen_table_utf8,
	MBFL_ENCTYPE_MBCS
};

const mbfl_encoding mbfl_encoding_utf8_kddi = {
	mbfl_no_encoding_utf8_kddi,
	"UTF-8-Mobile#KDDI",
	"UTF-8",
	(const char *(*)[])&mbfl_encoding_utf8_kddi_aliases,
	mblen_table_utf8,
	MBFL_ENCTYPE_MBCS
};

const mbfl_encoding mbfl_encoding_utf8_kddi_b = {
	mbfl_no_encoding_utf8_kddi_b,
	"UTF-8-Mobile#KDDI-B",
	"UTF-8",
	(const char *(*)[])&mbfl_encoding_utf8_kddi_b_aliases,
	mblen_table_utf8,
	MBFL_ENCTYPE_MBCS
};

const mbfl_encoding mbfl_encoding_utf8_sb = {
	mbfl_no_encoding_utf8_sb,
	"UTF-8-Mobile#SOFTBANK",
	"UTF-8",
	(const char *(*)[])&mbfl_encoding_utf8_sb_aliases,
	mblen_table_utf8,
	MBFL_ENCTYPE_MBCS
};

const struct mbfl_identify_vtbl vtbl_identify_utf8_docomo = {
	mbfl_no_encoding_utf8_docomo,
	mbfl_filt_ident_common_ctor,
	mbfl_filt_ident_common_dtor,
	mbfl_filt_ident_utf8
};

const struct mbfl_identify_vtbl vtbl_identify_utf8_kddi = {
	mbfl_no_encoding_utf8_kddi,
	mbfl_filt_ident_common_ctor,
	mbfl_filt_ident_common_dtor,
	mbfl_filt_ident_utf8
};

const struct mbfl_identify_vtbl vtbl_identify_utf8_kddi_b = {
	mbfl_no_encoding_utf8_kddi_b,
	mbfl_filt_ident_common_ctor,
	mbfl_filt_ident_common_dtor,
	mbfl_filt_ident_utf8
};

const struct mbfl_identify_vtbl vtbl_identify_utf8_sb = {
	mbfl_no_encoding_utf8_sb,
	mbfl_filt_ident_common_ctor,
	mbfl_filt_ident_common_dtor,
	mbfl_filt_ident_utf8
};

const struct mbfl_convert_vtbl vtbl_utf8_docomo_wchar = {
	mbfl_no_encoding_utf8_docomo,
	mbfl_no_encoding_wchar,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_utf8_mobile_wchar,
	mbfl_filt_conv_common_flush
};

const struct mbfl_convert_vtbl vtbl_wchar_utf8_docomo = {
	mbfl_no_encoding_wchar,
	mbfl_no_encoding_utf8_docomo,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_wchar_utf8_mobile,
	mbfl_filt_conv_common_flush
};

const struct mbfl_convert_vtbl vtbl_utf8_kddi_wchar = {
	mbfl_no_encoding_utf8_kddi,
	mbfl_no_encoding_wchar,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_utf8_mobile_wchar,
	mbfl_filt_conv_common_flush
};

const struct mbfl_convert_vtbl vtbl_wchar_utf8_kddi = {
	mbfl_no_encoding_wchar,
	mbfl_no_encoding_utf8_kddi,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_wchar_utf8_mobile,
	mbfl_filt_conv_common_flush
};

const struct mbfl_convert_vtbl vtbl_utf8_kddi_b_wchar = {
	mbfl_no_encoding_utf8_kddi_b,
	mbfl_no_encoding_wchar,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_utf8_mobile_wchar,
	mbfl_filt_conv_common_flush
};

const struct mbfl_convert_vtbl vtbl_wchar_utf8_kddi_b = {
	mbfl_no_encoding_wchar,
	mbfl_no_encoding_utf8_kddi_b,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_wchar_utf8_mobile,
	mbfl_filt_conv_common_flush
};

const struct mbfl_convert_vtbl vtbl_utf8_sb_wchar = {
	mbfl_no_encoding_utf8_sb,
	mbfl_no_encoding_wchar,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_utf8_mobile_wchar,
	mbfl_filt_conv_common_flush
};

const struct mbfl_convert_vtbl vtbl_wchar_utf8_sb = {
	mbfl_no_encoding_wchar,
	mbfl_no_encoding_utf8_sb,
	mbfl_filt_conv_common_ctor,
	mbfl_filt_conv_common_dtor,
	mbfl_filt_conv_wchar_utf8_mobile,
	mbfl_filt_conv_common_flush
};

#define CK(statement)	do { if ((statement) < 0) return (-1); } while (0)

/*
 * UTF-8 => wchar
 */
int mbfl_filt_conv_utf8_mobile_wchar(int c, mbfl_convert_filter *filter)
{
	int s, w = 0, flag = 0;
	int s1 = 0, s2 = 0, c1 = 0, c2 = 0, snd = 0;
	int sjis_encoded = 0;

	if (c < 0x80) {
		if (c >= 0) {
			CK((*filter->output_function)(c, filter->data));
		}
		filter->status = 0;
	} else if (c < 0xc0) {
		int status = filter->status & 0xff;
		switch (status) {
		case 0x10: /* 2byte code 2nd char: 0x80-0xbf */
		case 0x21: /* 3byte code 3rd char: 0x80-0xbf */
		case 0x32: /* 4byte code 4th char: 0x80-0xbf */
			filter->status = 0;
			s = filter->cache | (c & 0x3f);
			filter->cache = 0;
			if ((status == 0x10 && s >= 0x80) ||
			    (status == 0x21 && s >= 0x800 && (s < 0xd800 || s > 0xdfff)) ||
			    (status == 0x32 && s >= 0x10000 && s < 0x200000)) {
				
				if (filter->from->no_encoding == mbfl_no_encoding_utf8_docomo &&
					mbfilter_conv_r_map_tbl(s, &s1, mbfl_docomo2uni_pua, 4) > 0) {
					s = mbfilter_sjis_emoji_docomo2unicode(s1, &snd);
				} else if (filter->from->no_encoding == mbfl_no_encoding_utf8_kddi &&
						   mbfilter_conv_r_map_tbl(s, &s1, mbfl_kddi2uni_pua, 6) > 0) {
					s = mbfilter_sjis_emoji_kddi2unicode(s1, &snd);
				} else if (filter->from->no_encoding == mbfl_no_encoding_utf8_kddi_b &&
						   mbfilter_conv_r_map_tbl(s, &s1, mbfl_kddi2uni_pua_b, 8) > 0) {
					s = mbfilter_sjis_emoji_kddi2unicode(s1, &snd);
				} else if (filter->from->no_encoding == mbfl_no_encoding_utf8_sb &&
						   mbfilter_conv_r_map_tbl(s, &s1, mbfl_sb2uni_pua, 6) > 0) {
					s = mbfilter_sjis_emoji_sb2unicode(s1, &snd);
				}

				if (snd > 0) {
					CK((*filter->output_function)(snd, filter->data));
				}
				CK((*filter->output_function)(s, filter->data));
			} else {
				w = s & MBFL_WCSGROUP_MASK;
				flag = 1;
			}
			break;
		case 0x20: /* 3byte code 2nd char: 0:0xa0-0xbf,D:0x80-9F,1-C,E-F:0x80-0x9f */
			s = filter->cache | ((c & 0x3f) << 6);
			c1 = (s >> 12) & 0xf;
			if ((c1 == 0x0 && c >= 0xa0) || 
				(c1 == 0xd && c < 0xa0) || 
				(c1 > 0x0 && c1 != 0xd)) {
				filter->cache = s;
				filter->status++;
			} else {
				w = s & MBFL_WCSGROUP_MASK;
				flag = 1;
			}
			break;
		case 0x31: /* 4byte code 3rd char: 0x80-0xbf */
			filter->cache |= ((c & 0x3f) << 6);
			filter->status++;
			break;
		case 0x30: /* 4byte code 2nd char: 0:0x90-0xbf,1-3:0x80-0xbf,4:0x80-0x8f */
			s = filter->cache | ((c & 0x3f) << 12);
			c1 = (s >> 18) & 0x7;
			if ((c1 == 0x0 && c >= 0x90) ||
				(c1 > 0x0 && c1 < 0x4) ||
				(c1 == 0x4 && c < 0x90)) {
				filter->cache = s;
				filter->status++;
			} else {
				w = s & MBFL_WCSGROUP_MASK;
				flag = 1;
			}
			break;
		default:
			w = c & MBFL_WCSGROUP_MASK;
			flag = 1;
			break;
		}
	} else if (c < 0xc2) { /* invalid: 0xc0,0xc1 */
		w = c & MBFL_WCSGROUP_MASK;
		flag = 1;
	} else if (c < 0xe0) { /* 2byte code first char: 0xc2-0xdf */
		if (filter->status == 0x0) {
			filter->status = 0x10;
			filter->cache = (c & 0x1f) << 6;
		} else {
			w = c & MBFL_WCSGROUP_MASK;
			flag = 1;
		}
	} else if (c < 0xf0) { /* 3byte code first char: 0xe0-0xef */
		if (filter->status == 0x0) {
			filter->status = 0x20;
			filter->cache = (c & 0xf) << 12;
		} else {
			w = c & MBFL_WCSGROUP_MASK;
			flag = 1;
		}
	} else if (c < 0xf5) { /* 4byte code first char: 0xf0-0xf4 */
		if (filter->status == 0x0) {
			filter->status = 0x30;
			filter->cache = (c & 0x7) << 18;
		} else {
			w = c & MBFL_WCSGROUP_MASK;
			flag = 1;
		}
	} else {
		w = c & MBFL_WCSGROUP_MASK;
		flag = 1;
	}

	if (flag) {
		w |= MBFL_WCSGROUP_THROUGH;
		CK((*filter->output_function)(w, filter->data));
		filter->status = 0;
		filter->cache = 0;
	}

	return c;
}

/*
 * wchar => UTF-8
 */
int mbfl_filt_conv_wchar_utf8_mobile(int c, mbfl_convert_filter *filter)
{
	if (c >= 0 && c < 0x110000) {
		int s1, s2, c1, c2;

		if ((filter->to->no_encoding == mbfl_no_encoding_utf8_docomo &&
			 mbfilter_unicode2sjis_emoji_docomo(c, &s1, filter) > 0 &&
			 mbfilter_conv_map_tbl(s1, &c1, mbfl_docomo2uni_pua, 4) > 0) || 
			(filter->to->no_encoding == mbfl_no_encoding_utf8_kddi &&
			 mbfilter_unicode2sjis_emoji_kddi(c, &s1, filter) > 0 &&
			 mbfilter_conv_map_tbl(s1, &c1, mbfl_kddi2uni_pua, 6) > 0) ||
			(filter->to->no_encoding == mbfl_no_encoding_utf8_kddi_b &&
			 mbfilter_unicode2sjis_emoji_kddi(c, &s1, filter) > 0 &&
			 mbfilter_conv_map_tbl(s1, &c1, mbfl_kddi2uni_pua_b, 8) > 0) ||
			(filter->to->no_encoding == mbfl_no_encoding_utf8_sb &&
			 mbfilter_unicode2sjis_emoji_sb(c, &s1, filter) > 0 &&
			 mbfilter_conv_map_tbl(s1, &c1, mbfl_sb2uni_pua, 6) > 0)) {
			c = c1;
		}

		if (filter->status == 1 && filter->cache > 0) {
			return c;
		}

		if (c < 0x80) {
			CK((*filter->output_function)(c, filter->data));
		} else if (c < 0x800) {
			CK((*filter->output_function)(((c >> 6) & 0x1f) | 0xc0, filter->data));
			CK((*filter->output_function)((c & 0x3f) | 0x80, filter->data));
		} else if (c < 0x10000) {
			CK((*filter->output_function)(((c >> 12) & 0x0f) | 0xe0, filter->data));
			CK((*filter->output_function)(((c >> 6) & 0x3f) | 0x80, filter->data));
			CK((*filter->output_function)((c & 0x3f) | 0x80, filter->data));
		} else {
			CK((*filter->output_function)(((c >> 18) & 0x07) | 0xf0, filter->data));
			CK((*filter->output_function)(((c >> 12) & 0x3f) | 0x80, filter->data));
			CK((*filter->output_function)(((c >> 6) & 0x3f) | 0x80, filter->data));
			CK((*filter->output_function)((c & 0x3f) | 0x80, filter->data));
		}
	} else {
		if (filter->illegal_mode != MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			CK(mbfl_filt_conv_illegal_output(c, filter));
		}
	}

	return c;
}

