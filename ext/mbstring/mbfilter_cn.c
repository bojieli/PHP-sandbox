/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2001 The PHP Group                                     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Rui Hirokawa <hirokawa@php.net>                              |
   +----------------------------------------------------------------------+
 */

/*
 * "streamable simplified chinese code filter and converter"
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_globals.h"

#if defined(HAVE_MBSTR_CN)
#include "mbfilter.h"
#include "mbfilter_cn.h"

#include "unicode_table_cn.h"

#define CK(statement)	do { if ((statement) < 0) return (-1); } while (0)


/*
 * EUC-CN => wchar
 */
int
mbfl_filt_conv_euccn_wchar(int c, mbfl_convert_filter *filter TSRMLS_DC)
{
	int c1, w;

	switch (filter->status) {
	case 0:
		if (c >= 0 && c < 0x80) {	/* latin */
			CK((*filter->output_function)(c, filter->data TSRMLS_CC));
		} else if (c > 0xa0 && c < 0xff) {	/* dbcs lead byte */
			filter->status = 1;
			filter->cache = c;
		} else {
			w = c & MBFL_WCSGROUP_MASK;
			w |= MBFL_WCSGROUP_THROUGH;
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		}
		break;

	case 1:		/* dbcs second byte */
		filter->status = 0;
		c1 = filter->cache;
		if (c1 > 0xa0 && c1 < 0xff && c > 0xa0 && c < 0xff) {
			w = (c1 - 0x81)*192 + (c - 0x40);
			if (w >= 0 && w < cp936_ucs_table_size) {
				w = cp936_ucs_table[w];
			} else {
				w = 0;
			}
			if (w <= 0) {
				w = (c1 << 8) | c;
				w &= MBFL_WCSPLANE_MASK;
				w |= MBFL_WCSPLANE_GB2312;
			}
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		} else if ((c >= 0 && c < 0x21) || c == 0x7f) {		/* CTLs */
			CK((*filter->output_function)(c, filter->data TSRMLS_CC));
		} else {
			w = (c1 << 8) | c;
			w &= MBFL_WCSGROUP_MASK;
			w |= MBFL_WCSGROUP_THROUGH;
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		}
		break;

	default:
		filter->status = 0;
		break;
	}

	return c;
}

/*
 * wchar => EUC-CN
 */
int
mbfl_filt_conv_wchar_euccn(int c, mbfl_convert_filter *filter TSRMLS_DC)
{
	int c1, c2, s;

	s = 0;
	if (c >= ucs_a1_cp936_table_min && c < ucs_a1_cp936_table_max) {
		s = ucs_a1_cp936_table[c - ucs_a1_cp936_table_min];
	} else if (c >= ucs_a2_cp936_table_min && c < ucs_a2_cp936_table_max) {
		s = ucs_a2_cp936_table[c - ucs_a2_cp936_table_min];
	} else if (c >= ucs_a3_cp936_table_min && c < ucs_a3_cp936_table_max) {
		s = ucs_a3_cp936_table[c - ucs_a3_cp936_table_min];
	} else if (c >= ucs_i_cp936_table_min && c < ucs_i_cp936_table_max) {
		s = ucs_i_cp936_table[c - ucs_i_cp936_table_min];
	} else if (c >= ucs_hff_cp936_table_min && c < ucs_hff_cp936_table_max) {
		s = ucs_hff_cp936_table[c - ucs_hff_cp936_table_min];
	}
	c1 = (s >> 8) & 0xff;
	c2 = s & 0xff;
	
	if (c1 < 0xa1 || c2 < 0xa1) { /* exclude CP936 extension */
		s = c;
	}

	if (s <= 0) {
		c1 = c & ~MBFL_WCSPLANE_MASK;
		if (c1 == MBFL_WCSPLANE_GB2312) {
			s = c & MBFL_WCSPLANE_MASK;
		}
		if (c == 0) {
			s = 0;
		} else if (s <= 0) {
			s = -1;
		}
	}
	if (s >= 0) {
		if (s < 0x80) {	/* latin */
			CK((*filter->output_function)(s, filter->data TSRMLS_CC));
		} else {
			CK((*filter->output_function)((s >> 8) & 0xff, filter->data TSRMLS_CC));
			CK((*filter->output_function)(s & 0xff, filter->data TSRMLS_CC));
		}
	} else {
		if (filter->illegal_mode != MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			CK(mbfl_filt_conv_illegal_output(c, filter TSRMLS_CC));
		}
	}

	return c;
}

/*
 * CP936 => wchar
 */
int
mbfl_filt_conv_cp936_wchar(int c, mbfl_convert_filter *filter TSRMLS_DC)
{
	int c1, w;

	switch (filter->status) {
	case 0:
		if (c >= 0 && c < 0x80) {	/* latin */
			CK((*filter->output_function)(c, filter->data TSRMLS_CC));
		} else if (c == 0x80) {	/* euro sign */
			CK((*filter->output_function)(0x20ac, filter->data TSRMLS_CC));
		} else if (c > 0x80 && c < 0xff) {	/* dbcs lead byte */
			filter->status = 1;
			filter->cache = c;
		} else {
			w = c & MBFL_WCSGROUP_MASK;
			w |= MBFL_WCSGROUP_THROUGH;
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		}
		break;

	case 1:		/* dbcs second byte */
		filter->status = 0;
		c1 = filter->cache;
		if ( c1 < 0xff && c1 > 0x80 && c > 0x39 && c < 0xff && c != 0x7f) {
			w = (c1 - 0x81)*192 + (c - 0x40);
			if (w >= 0 && w < cp936_ucs_table_size) {
				w = cp936_ucs_table[w];
			} else {
				w = 0;
			}
			if (w <= 0) {
				w = (c1 << 8) | c;
				w &= MBFL_WCSPLANE_MASK;
				w |= MBFL_WCSPLANE_WINCP936;
			}
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		} else if ((c >= 0 && c < 0x21) || c == 0x7f) {		/* CTLs */
			CK((*filter->output_function)(c, filter->data TSRMLS_CC));
		} else {
			w = (c1 << 8) | c;
			w &= MBFL_WCSGROUP_MASK;
			w |= MBFL_WCSGROUP_THROUGH;
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		}
		break;

	default:
		filter->status = 0;
		break;
	}

	return c;
}

/*
 * wchar => CP936
 */
int
mbfl_filt_conv_wchar_cp936(int c, mbfl_convert_filter *filter TSRMLS_DC)
{
	int c1, s;

	s = 0;
	if (c >= ucs_a1_cp936_table_min && c < ucs_a1_cp936_table_max) {
		s = ucs_a1_cp936_table[c - ucs_a1_cp936_table_min];
	} else if (c >= ucs_a2_cp936_table_min && c < ucs_a2_cp936_table_max) {
		s = ucs_a2_cp936_table[c - ucs_a2_cp936_table_min];
	} else if (c >= ucs_a3_cp936_table_min && c < ucs_a3_cp936_table_max) {
		s = ucs_a3_cp936_table[c - ucs_a3_cp936_table_min];
	} else if (c >= ucs_i_cp936_table_min && c < ucs_i_cp936_table_max) {
		s = ucs_i_cp936_table[c - ucs_i_cp936_table_min];
	} else if (c >= ucs_ci_cp936_table_min && c < ucs_ci_cp936_table_max) {
		s = ucs_ci_cp936_table[c - ucs_ci_cp936_table_min];
	} else if (c >= ucs_cf_cp936_table_min && c < ucs_cf_cp936_table_max) {
		s = ucs_cf_cp936_table[c - ucs_cf_cp936_table_min];
	} else if (c >= ucs_sfv_cp936_table_min && c < ucs_sfv_cp936_table_max) {
		s = ucs_sfv_cp936_table[c - ucs_sfv_cp936_table_min];
	} else if (c >= ucs_hff_cp936_table_min && c < ucs_hff_cp936_table_max) {
		s = ucs_hff_cp936_table[c - ucs_hff_cp936_table_min];
	}
	if (s <= 0) {
		c1 = c & ~MBFL_WCSPLANE_MASK;
		if (c1 == MBFL_WCSPLANE_WINCP936) {
			s = c & MBFL_WCSPLANE_MASK;
		}
		if (c == 0) {
			s = 0;
		} else if (s <= 0) {
			s = -1;
		}
	}
	if (s >= 0) {
		if (s < 0x80) {	/* latin */
			CK((*filter->output_function)(s, filter->data TSRMLS_CC));
		} else {
			CK((*filter->output_function)((s >> 8) & 0xff, filter->data TSRMLS_CC));
			CK((*filter->output_function)(s & 0xff, filter->data TSRMLS_CC));
		}
	} else {
		if (filter->illegal_mode != MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			CK(mbfl_filt_conv_illegal_output(c, filter TSRMLS_CC));
		}
	}

	return c;
}


/*
 * HZ => wchar
 */
int
mbfl_filt_conv_hz_wchar(int c, mbfl_convert_filter *filter TSRMLS_DC)
{
	int c1, s, w;

	switch (filter->status & 0xf) {
/*	case 0x00:	 ASCII */
/*	case 0x10:	 GB2312 */
	case 0:
		if (c == 0x7e) {
			filter->status += 2;
		} else if (filter->status == 0x10 && c > 0x20 && c < 0x7f) {	/* DBCS first char */
			filter->cache = c;
			filter->status += 1;
		} else if (c >= 0 && c < 0x80) {		/* latin, CTLs */
			CK((*filter->output_function)(c, filter->data TSRMLS_CC));
		} else {
			w = c & MBFL_WCSGROUP_MASK;
			w |= MBFL_WCSGROUP_THROUGH;
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		}
		break;

/*	case 0x11:	 GB2312 second char */
	case 1:
		filter->status &= ~0xf;
		c1 = filter->cache;
		if (c1 > 0x20 && c1 < 0x7f && c > 0x20 && c < 0x7f) {
			s = (c1 - 1)*192 + c + 0x40; /* GB2312 */
			if (s >= 0 && s < cp936_ucs_table_size) {
				w = cp936_ucs_table[s];
			} else {
				w = 0;
			}
			if (w <= 0) {
				w = (c1 << 8) | c;
				w &= MBFL_WCSPLANE_MASK;
				w |= MBFL_WCSPLANE_GB2312;
			}
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		} else if ((c >= 0 && c < 0x21) || c == 0x7f) {		/* CTLs */
			CK((*filter->output_function)(c, filter->data TSRMLS_CC));
		} else {
			w = (c1 << 8) | c;
			w &= MBFL_WCSGROUP_MASK;
			w |= MBFL_WCSGROUP_THROUGH;
			CK((*filter->output_function)(w, filter->data TSRMLS_CC));
		}
		break;

	/* '~' */
	case 2:
		if (c == 0x7d) {		/* '}' */
			filter->status = 0x0;
		} else if (c == 0x7b) {		/* '{' */
			filter->status = 0x10;
		} else if (c == 0x7e) { /* '~' */
			filter->status = 0x0;
			CK((*filter->output_function)(0x007e, filter->data TSRMLS_CC));
		}
		break;

	default:
		filter->status = 0;
		break;
	}

	return c;
}

/*
 * wchar => HZ
 */
int
mbfl_filt_conv_wchar_hz(int c, mbfl_convert_filter *filter TSRMLS_DC)
{
	int s;

	s = 0;
	if (c >= ucs_a1_cp936_table_min && c < ucs_a1_cp936_table_max) {
		s = ucs_a1_cp936_table[c - ucs_a1_cp936_table_min];
	} else if (c >= ucs_a2_cp936_table_min && c < ucs_a2_cp936_table_max) {
		s = ucs_a2_cp936_table[c - ucs_a2_cp936_table_min];
	} else if (c >= ucs_a3_cp936_table_min && c < ucs_a3_cp936_table_max) {
		s = ucs_a3_cp936_table[c - ucs_a3_cp936_table_min];
	} else if (c >= ucs_i_cp936_table_min && c < ucs_i_cp936_table_max) {
		s = ucs_i_cp936_table[c - ucs_i_cp936_table_min];
	} else if (c >= ucs_hff_cp936_table_min && c < ucs_hff_cp936_table_max) {
		s = ucs_hff_cp936_table[c - ucs_hff_cp936_table_min];
	}
	if (s & 0x8000) {
		s -= 0x8080;
	}

	if (s <= 0) {
		if (c == 0) {
			s = 0;
		} else if (s <= 0) {
			s = -1;
		}
	} else if ((s >= 0x80 && s < 0x2121) || (s > 0x8080)) {
		s = -1;
	}
	if (s >= 0) {
		if (s < 0x80) { /* ASCII */
			if ((filter->status & 0xff00) != 0) {
				CK((*filter->output_function)(0x7e, filter->data TSRMLS_CC));		/* '~' */
				CK((*filter->output_function)(0x7d, filter->data TSRMLS_CC));		/* '}' */
			}
			filter->status = 0;
			if (s == 0x7e){
				CK((*filter->output_function)(0x7e, filter->data TSRMLS_CC));
			}
			CK((*filter->output_function)(s, filter->data TSRMLS_CC));
		} else { /* GB 2312-80 */
			if ((filter->status & 0xff00) != 0x200) {
				CK((*filter->output_function)(0x7e, filter->data TSRMLS_CC));		/* '~' */
				CK((*filter->output_function)(0x7b, filter->data TSRMLS_CC));		/* '{' */
			}
			filter->status = 0x200;
			CK((*filter->output_function)((s >> 8) & 0x7f, filter->data TSRMLS_CC));
			CK((*filter->output_function)(s & 0x7f, filter->data TSRMLS_CC));
		}
	} else {
		if (filter->illegal_mode != MBFL_OUTPUTFILTER_ILLEGAL_MODE_NONE) {
			CK(mbfl_filt_conv_illegal_output(c, filter TSRMLS_CC));
		}
	}

	return c;
}

int
mbfl_filt_conv_any_hz_flush(mbfl_convert_filter *filter TSRMLS_DC)
{
	/* back to latin */
	if ((filter->status & 0xff00) != 0) {
		CK((*filter->output_function)(0x7e, filter->data TSRMLS_CC));		/* ~ */
		CK((*filter->output_function)(0x7d, filter->data TSRMLS_CC));		/* '{' */
	}
	filter->status &= 0xff;
	return 0;
}

#endif /* HAVE_MBSTR_CN */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
