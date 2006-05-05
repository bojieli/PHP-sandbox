/*
  +----------------------------------------------------------------------+
  | PHP Version 6                                                        |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Andrei Zmievski <andrei@php.net>                            |
  |          Wez Furlong <wez@php.net>                                   |
  +----------------------------------------------------------------------+
*/

#include "php_unicode.h"

PHPAPI zend_class_entry *u_const_ce;

#define REGISTER_U_CONST(constant) \
	php_register_u_constant(#constant, (long)constant TSRMLS_CC)

static void php_register_u_constant(const char *name, long value TSRMLS_DC)
{
	const char *p = name;

	if (*p == 'U') p++;
	if (*p == '_') p++;

	zend_declare_class_constant_long(u_const_ce, (char *)p, strlen(p), value TSRMLS_CC);
}

/* {{{ Character property constants */
static void php_register_property_constants(TSRMLS_D)
{
	REGISTER_U_CONST(UCHAR_ALPHABETIC);
	REGISTER_U_CONST(UCHAR_ASCII_HEX_DIGIT);
	REGISTER_U_CONST(UCHAR_BIDI_CONTROL);
	REGISTER_U_CONST(UCHAR_BIDI_MIRRORED);
	REGISTER_U_CONST(UCHAR_DASH);
	REGISTER_U_CONST(UCHAR_DEFAULT_IGNORABLE_CODE_POINT);
	REGISTER_U_CONST(UCHAR_DEPRECATED);
	REGISTER_U_CONST(UCHAR_DIACRITIC);
	REGISTER_U_CONST(UCHAR_EXTENDER);
	REGISTER_U_CONST(UCHAR_FULL_COMPOSITION_EXCLUSION);
	REGISTER_U_CONST(UCHAR_GRAPHEME_BASE);
	REGISTER_U_CONST(UCHAR_GRAPHEME_EXTEND);
	REGISTER_U_CONST(UCHAR_GRAPHEME_LINK);
	REGISTER_U_CONST(UCHAR_HEX_DIGIT);
	REGISTER_U_CONST(UCHAR_HYPHEN);
	REGISTER_U_CONST(UCHAR_ID_CONTINUE);
	REGISTER_U_CONST(UCHAR_ID_START);
	REGISTER_U_CONST(UCHAR_IDEOGRAPHIC);
	REGISTER_U_CONST(UCHAR_IDS_BINARY_OPERATOR);
	REGISTER_U_CONST(UCHAR_IDS_TRINARY_OPERATOR);
	REGISTER_U_CONST(UCHAR_JOIN_CONTROL);
	REGISTER_U_CONST(UCHAR_LOGICAL_ORDER_EXCEPTION);
	REGISTER_U_CONST(UCHAR_LOWERCASE);
	REGISTER_U_CONST(UCHAR_MATH);
	REGISTER_U_CONST(UCHAR_NONCHARACTER_CODE_POINT);
	REGISTER_U_CONST(UCHAR_QUOTATION_MARK);
	REGISTER_U_CONST(UCHAR_RADICAL);
	REGISTER_U_CONST(UCHAR_SOFT_DOTTED);
	REGISTER_U_CONST(UCHAR_TERMINAL_PUNCTUATION);
	REGISTER_U_CONST(UCHAR_UNIFIED_IDEOGRAPH);
	REGISTER_U_CONST(UCHAR_UPPERCASE);
	REGISTER_U_CONST(UCHAR_WHITE_SPACE);
	REGISTER_U_CONST(UCHAR_XID_CONTINUE);
	REGISTER_U_CONST(UCHAR_XID_START);
	REGISTER_U_CONST(UCHAR_CASE_SENSITIVE);
	REGISTER_U_CONST(UCHAR_S_TERM);
	REGISTER_U_CONST(UCHAR_VARIATION_SELECTOR);
	REGISTER_U_CONST(UCHAR_NFD_INERT);
	REGISTER_U_CONST(UCHAR_NFKD_INERT);
	REGISTER_U_CONST(UCHAR_NFC_INERT);
	REGISTER_U_CONST(UCHAR_NFKC_INERT);
	REGISTER_U_CONST(UCHAR_SEGMENT_STARTER);
	REGISTER_U_CONST(UCHAR_PATTERN_SYNTAX);
	REGISTER_U_CONST(UCHAR_PATTERN_WHITE_SPACE);
	REGISTER_U_CONST(UCHAR_POSIX_ALNUM);
	REGISTER_U_CONST(UCHAR_POSIX_BLANK);
	REGISTER_U_CONST(UCHAR_POSIX_GRAPH);
	REGISTER_U_CONST(UCHAR_POSIX_PRINT);
	REGISTER_U_CONST(UCHAR_POSIX_XDIGIT);
	REGISTER_U_CONST(UCHAR_BIDI_CLASS);
	REGISTER_U_CONST(UCHAR_INT_START);
	REGISTER_U_CONST(UCHAR_BLOCK);
	REGISTER_U_CONST(UCHAR_CANONICAL_COMBINING_CLASS);
	REGISTER_U_CONST(UCHAR_DECOMPOSITION_TYPE);
	REGISTER_U_CONST(UCHAR_EAST_ASIAN_WIDTH);
	REGISTER_U_CONST(UCHAR_GENERAL_CATEGORY);
	REGISTER_U_CONST(UCHAR_JOINING_GROUP);
	REGISTER_U_CONST(UCHAR_JOINING_TYPE);
	REGISTER_U_CONST(UCHAR_LINE_BREAK);
	REGISTER_U_CONST(UCHAR_NUMERIC_TYPE);
	REGISTER_U_CONST(UCHAR_SCRIPT);
	REGISTER_U_CONST(UCHAR_HANGUL_SYLLABLE_TYPE);
	REGISTER_U_CONST(UCHAR_NFD_QUICK_CHECK);
	REGISTER_U_CONST(UCHAR_NFKD_QUICK_CHECK);
	REGISTER_U_CONST(UCHAR_NFC_QUICK_CHECK);
	REGISTER_U_CONST(UCHAR_NFKC_QUICK_CHECK);
	REGISTER_U_CONST(UCHAR_LEAD_CANONICAL_COMBINING_CLASS);
	REGISTER_U_CONST(UCHAR_TRAIL_CANONICAL_COMBINING_CLASS);
	REGISTER_U_CONST(UCHAR_GRAPHEME_CLUSTER_BREAK);
	REGISTER_U_CONST(UCHAR_SENTENCE_BREAK);
	REGISTER_U_CONST(UCHAR_WORD_BREAK);
	REGISTER_U_CONST(UCHAR_GENERAL_CATEGORY_MASK);
	REGISTER_U_CONST(UCHAR_NUMERIC_VALUE);
	REGISTER_U_CONST(UCHAR_AGE);
	REGISTER_U_CONST(UCHAR_BIDI_MIRRORING_GLYPH);
	REGISTER_U_CONST(UCHAR_CASE_FOLDING);
	REGISTER_U_CONST(UCHAR_ISO_COMMENT);
	REGISTER_U_CONST(UCHAR_LOWERCASE_MAPPING);
	REGISTER_U_CONST(UCHAR_NAME);
	REGISTER_U_CONST(UCHAR_SIMPLE_CASE_FOLDING);
	REGISTER_U_CONST(UCHAR_SIMPLE_LOWERCASE_MAPPING);
	REGISTER_U_CONST(UCHAR_SIMPLE_TITLECASE_MAPPING);
	REGISTER_U_CONST(UCHAR_SIMPLE_UPPERCASE_MAPPING);
	REGISTER_U_CONST(UCHAR_TITLECASE_MAPPING);
	REGISTER_U_CONST(UCHAR_UNICODE_1_NAME);
	REGISTER_U_CONST(UCHAR_UPPERCASE_MAPPING);
	REGISTER_U_CONST(UCHAR_INVALID_CODE);
}
/* }}} */

/* {{{ General category constants */
static void php_register_general_category_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_UNASSIGNED);
	REGISTER_U_CONST(U_GENERAL_OTHER_TYPES);
	REGISTER_U_CONST(U_UPPERCASE_LETTER);
	REGISTER_U_CONST(U_LOWERCASE_LETTER);
	REGISTER_U_CONST(U_TITLECASE_LETTER);
	REGISTER_U_CONST(U_MODIFIER_LETTER);
	REGISTER_U_CONST(U_OTHER_LETTER);
	REGISTER_U_CONST(U_NON_SPACING_MARK);
	REGISTER_U_CONST(U_ENCLOSING_MARK);
	REGISTER_U_CONST(U_COMBINING_SPACING_MARK);
	REGISTER_U_CONST(U_DECIMAL_DIGIT_NUMBER);
	REGISTER_U_CONST(U_LETTER_NUMBER);
	REGISTER_U_CONST(U_OTHER_NUMBER);
	REGISTER_U_CONST(U_SPACE_SEPARATOR);
	REGISTER_U_CONST(U_LINE_SEPARATOR);
	REGISTER_U_CONST(U_PARAGRAPH_SEPARATOR);
	REGISTER_U_CONST(U_CONTROL_CHAR);
	REGISTER_U_CONST(U_FORMAT_CHAR);
	REGISTER_U_CONST(U_PRIVATE_USE_CHAR);
	REGISTER_U_CONST(U_SURROGATE);
	REGISTER_U_CONST(U_DASH_PUNCTUATION);
	REGISTER_U_CONST(U_START_PUNCTUATION);
	REGISTER_U_CONST(U_END_PUNCTUATION);
	REGISTER_U_CONST(U_CONNECTOR_PUNCTUATION);
	REGISTER_U_CONST(U_OTHER_PUNCTUATION);
	REGISTER_U_CONST(U_MATH_SYMBOL);
	REGISTER_U_CONST(U_CURRENCY_SYMBOL);
	REGISTER_U_CONST(U_MODIFIER_SYMBOL);
	REGISTER_U_CONST(U_OTHER_SYMBOL);
	REGISTER_U_CONST(U_INITIAL_PUNCTUATION);
	REGISTER_U_CONST(U_FINAL_PUNCTUATION);
}
/* }}} */

/* {{{ Character direction constants */
static void php_register_char_direction_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_LEFT_TO_RIGHT);
	REGISTER_U_CONST(U_RIGHT_TO_LEFT);
	REGISTER_U_CONST(U_EUROPEAN_NUMBER);
	REGISTER_U_CONST(U_EUROPEAN_NUMBER_SEPARATOR);
	REGISTER_U_CONST(U_EUROPEAN_NUMBER_TERMINATOR);
	REGISTER_U_CONST(U_ARABIC_NUMBER);
	REGISTER_U_CONST(U_COMMON_NUMBER_SEPARATOR);
	REGISTER_U_CONST(U_BLOCK_SEPARATOR);
	REGISTER_U_CONST(U_SEGMENT_SEPARATOR);
	REGISTER_U_CONST(U_WHITE_SPACE_NEUTRAL);
	REGISTER_U_CONST(U_OTHER_NEUTRAL);
	REGISTER_U_CONST(U_LEFT_TO_RIGHT_EMBEDDING);
	REGISTER_U_CONST(U_LEFT_TO_RIGHT_OVERRIDE);
	REGISTER_U_CONST(U_RIGHT_TO_LEFT_ARABIC);
	REGISTER_U_CONST(U_RIGHT_TO_LEFT_EMBEDDING);
	REGISTER_U_CONST(U_RIGHT_TO_LEFT_OVERRIDE);
	REGISTER_U_CONST(U_POP_DIRECTIONAL_FORMAT);
	REGISTER_U_CONST(U_DIR_NON_SPACING_MARK);
	REGISTER_U_CONST(U_BOUNDARY_NEUTRAL);
}
/* }}} */

/* {{{ Unicode block constants */
static void php_register_block_constants(TSRMLS_D)
{
	REGISTER_U_CONST(UBLOCK_NO_BLOCK);
	REGISTER_U_CONST(UBLOCK_BASIC_LATIN);
	REGISTER_U_CONST(UBLOCK_LATIN_1_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_LATIN_EXTENDED_A);
	REGISTER_U_CONST(UBLOCK_LATIN_EXTENDED_B);
	REGISTER_U_CONST(UBLOCK_IPA_EXTENSIONS);
	REGISTER_U_CONST(UBLOCK_SPACING_MODIFIER_LETTERS);
	REGISTER_U_CONST(UBLOCK_COMBINING_DIACRITICAL_MARKS);
	REGISTER_U_CONST(UBLOCK_GREEK);
	REGISTER_U_CONST(UBLOCK_CYRILLIC);
	REGISTER_U_CONST(UBLOCK_ARMENIAN);
	REGISTER_U_CONST(UBLOCK_HEBREW);
	REGISTER_U_CONST(UBLOCK_ARABIC);
	REGISTER_U_CONST(UBLOCK_SYRIAC);
	REGISTER_U_CONST(UBLOCK_THAANA);
	REGISTER_U_CONST(UBLOCK_DEVANAGARI);
	REGISTER_U_CONST(UBLOCK_BENGALI);
	REGISTER_U_CONST(UBLOCK_GURMUKHI);
	REGISTER_U_CONST(UBLOCK_GUJARATI);
	REGISTER_U_CONST(UBLOCK_ORIYA);
	REGISTER_U_CONST(UBLOCK_TAMIL);
	REGISTER_U_CONST(UBLOCK_TELUGU);
	REGISTER_U_CONST(UBLOCK_KANNADA);
	REGISTER_U_CONST(UBLOCK_MALAYALAM);
	REGISTER_U_CONST(UBLOCK_SINHALA);
	REGISTER_U_CONST(UBLOCK_THAI);
	REGISTER_U_CONST(UBLOCK_LAO);
	REGISTER_U_CONST(UBLOCK_TIBETAN);
	REGISTER_U_CONST(UBLOCK_MYANMAR);
	REGISTER_U_CONST(UBLOCK_GEORGIAN);
	REGISTER_U_CONST(UBLOCK_HANGUL_JAMO);
	REGISTER_U_CONST(UBLOCK_ETHIOPIC);
	REGISTER_U_CONST(UBLOCK_CHEROKEE);
	REGISTER_U_CONST(UBLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS);
	REGISTER_U_CONST(UBLOCK_OGHAM);
	REGISTER_U_CONST(UBLOCK_RUNIC);
	REGISTER_U_CONST(UBLOCK_KHMER);
	REGISTER_U_CONST(UBLOCK_MONGOLIAN);
	REGISTER_U_CONST(UBLOCK_LATIN_EXTENDED_ADDITIONAL);
	REGISTER_U_CONST(UBLOCK_GREEK_EXTENDED);
	REGISTER_U_CONST(UBLOCK_GENERAL_PUNCTUATION);
	REGISTER_U_CONST(UBLOCK_SUPERSCRIPTS_AND_SUBSCRIPTS);
	REGISTER_U_CONST(UBLOCK_CURRENCY_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_COMBINING_MARKS_FOR_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_LETTERLIKE_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_NUMBER_FORMS);
	REGISTER_U_CONST(UBLOCK_ARROWS);
	REGISTER_U_CONST(UBLOCK_MATHEMATICAL_OPERATORS);
	REGISTER_U_CONST(UBLOCK_MISCELLANEOUS_TECHNICAL);
	REGISTER_U_CONST(UBLOCK_CONTROL_PICTURES);
	REGISTER_U_CONST(UBLOCK_OPTICAL_CHARACTER_RECOGNITION);
	REGISTER_U_CONST(UBLOCK_ENCLOSED_ALPHANUMERICS);
	REGISTER_U_CONST(UBLOCK_BOX_DRAWING);
	REGISTER_U_CONST(UBLOCK_BLOCK_ELEMENTS);
	REGISTER_U_CONST(UBLOCK_GEOMETRIC_SHAPES);
	REGISTER_U_CONST(UBLOCK_MISCELLANEOUS_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_DINGBATS);
	REGISTER_U_CONST(UBLOCK_BRAILLE_PATTERNS);
	REGISTER_U_CONST(UBLOCK_CJK_RADICALS_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_KANGXI_RADICALS);
	REGISTER_U_CONST(UBLOCK_IDEOGRAPHIC_DESCRIPTION_CHARACTERS);
	REGISTER_U_CONST(UBLOCK_CJK_SYMBOLS_AND_PUNCTUATION);
	REGISTER_U_CONST(UBLOCK_HIRAGANA);
	REGISTER_U_CONST(UBLOCK_KATAKANA);
	REGISTER_U_CONST(UBLOCK_BOPOMOFO);
	REGISTER_U_CONST(UBLOCK_HANGUL_COMPATIBILITY_JAMO);
	REGISTER_U_CONST(UBLOCK_KANBUN);
	REGISTER_U_CONST(UBLOCK_BOPOMOFO_EXTENDED);
	REGISTER_U_CONST(UBLOCK_ENCLOSED_CJK_LETTERS_AND_MONTHS);
	REGISTER_U_CONST(UBLOCK_CJK_COMPATIBILITY);
	REGISTER_U_CONST(UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A);
	REGISTER_U_CONST(UBLOCK_CJK_UNIFIED_IDEOGRAPHS);
	REGISTER_U_CONST(UBLOCK_YI_SYLLABLES);
	REGISTER_U_CONST(UBLOCK_YI_RADICALS);
	REGISTER_U_CONST(UBLOCK_HANGUL_SYLLABLES);
	REGISTER_U_CONST(UBLOCK_HIGH_SURROGATES);
	REGISTER_U_CONST(UBLOCK_HIGH_PRIVATE_USE_SURROGATES);
	REGISTER_U_CONST(UBLOCK_LOW_SURROGATES);
	REGISTER_U_CONST(UBLOCK_PRIVATE_USE_AREA);
	REGISTER_U_CONST(UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS);
	REGISTER_U_CONST(UBLOCK_ALPHABETIC_PRESENTATION_FORMS);
	REGISTER_U_CONST(UBLOCK_ARABIC_PRESENTATION_FORMS_A);
	REGISTER_U_CONST(UBLOCK_COMBINING_HALF_MARKS);
	REGISTER_U_CONST(UBLOCK_CJK_COMPATIBILITY_FORMS);
	REGISTER_U_CONST(UBLOCK_SMALL_FORM_VARIANTS);
	REGISTER_U_CONST(UBLOCK_ARABIC_PRESENTATION_FORMS_B);
	REGISTER_U_CONST(UBLOCK_SPECIALS);
	REGISTER_U_CONST(UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS);
	REGISTER_U_CONST(UBLOCK_OLD_ITALIC);
	REGISTER_U_CONST(UBLOCK_GOTHIC);
	REGISTER_U_CONST(UBLOCK_DESERET);
	REGISTER_U_CONST(UBLOCK_BYZANTINE_MUSICAL_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_MUSICAL_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_MATHEMATICAL_ALPHANUMERIC_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B);
	REGISTER_U_CONST(UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_TAGS);
	REGISTER_U_CONST(UBLOCK_CYRILLIC_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_TAGALOG);
	REGISTER_U_CONST(UBLOCK_HANUNOO);
	REGISTER_U_CONST(UBLOCK_BUHID);
	REGISTER_U_CONST(UBLOCK_TAGBANWA);
	REGISTER_U_CONST(UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A);
	REGISTER_U_CONST(UBLOCK_SUPPLEMENTAL_ARROWS_A);
	REGISTER_U_CONST(UBLOCK_SUPPLEMENTAL_ARROWS_B);
	REGISTER_U_CONST(UBLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B);
	REGISTER_U_CONST(UBLOCK_SUPPLEMENTAL_MATHEMATICAL_OPERATORS);
	REGISTER_U_CONST(UBLOCK_KATAKANA_PHONETIC_EXTENSIONS);
	REGISTER_U_CONST(UBLOCK_VARIATION_SELECTORS);
	REGISTER_U_CONST(UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_A);
	REGISTER_U_CONST(UBLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B);
	REGISTER_U_CONST(UBLOCK_LIMBU);
	REGISTER_U_CONST(UBLOCK_TAI_LE);
	REGISTER_U_CONST(UBLOCK_KHMER_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_PHONETIC_EXTENSIONS);
	REGISTER_U_CONST(UBLOCK_MISCELLANEOUS_SYMBOLS_AND_ARROWS);
	REGISTER_U_CONST(UBLOCK_YIJING_HEXAGRAM_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_LINEAR_B_SYLLABARY);
	REGISTER_U_CONST(UBLOCK_LINEAR_B_IDEOGRAMS);
	REGISTER_U_CONST(UBLOCK_AEGEAN_NUMBERS);
	REGISTER_U_CONST(UBLOCK_UGARITIC);
	REGISTER_U_CONST(UBLOCK_SHAVIAN);
	REGISTER_U_CONST(UBLOCK_OSMANYA);
	REGISTER_U_CONST(UBLOCK_CYPRIOT_SYLLABARY);
	REGISTER_U_CONST(UBLOCK_TAI_XUAN_JING_SYMBOLS);
	REGISTER_U_CONST(UBLOCK_VARIATION_SELECTORS_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_ANCIENT_GREEK_MUSICAL_NOTATION);
	REGISTER_U_CONST(UBLOCK_ANCIENT_GREEK_NUMBERS);
	REGISTER_U_CONST(UBLOCK_ARABIC_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_BUGINESE);
	REGISTER_U_CONST(UBLOCK_CJK_STROKES);
	REGISTER_U_CONST(UBLOCK_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_COPTIC);
	REGISTER_U_CONST(UBLOCK_ETHIOPIC_EXTENDED);
	REGISTER_U_CONST(UBLOCK_ETHIOPIC_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_GEORGIAN_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_GLAGOLITIC);
	REGISTER_U_CONST(UBLOCK_KHAROSHTHI);
	REGISTER_U_CONST(UBLOCK_MODIFIER_TONE_LETTERS);
	REGISTER_U_CONST(UBLOCK_NEW_TAI_LUE);
	REGISTER_U_CONST(UBLOCK_OLD_PERSIAN);
	REGISTER_U_CONST(UBLOCK_PHONETIC_EXTENSIONS_SUPPLEMENT);
	REGISTER_U_CONST(UBLOCK_SUPPLEMENTAL_PUNCTUATION);
	REGISTER_U_CONST(UBLOCK_SYLOTI_NAGRI);
	REGISTER_U_CONST(UBLOCK_TIFINAGH);
	REGISTER_U_CONST(UBLOCK_VERTICAL_FORMS);
}
/* }}} */

/* {{{ East Asian width constants */
static void php_register_east_asian_width_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_EA_NEUTRAL);
	REGISTER_U_CONST(U_EA_AMBIGUOUS);
	REGISTER_U_CONST(U_EA_HALFWIDTH);
	REGISTER_U_CONST(U_EA_FULLWIDTH);
	REGISTER_U_CONST(U_EA_NARROW);
	REGISTER_U_CONST(U_EA_WIDE);
}
/* }}} */

/* {{{ Decomposition type constants */
static void php_register_decomposition_type_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_DT_NONE);
	REGISTER_U_CONST(U_DT_CANONICAL);
	REGISTER_U_CONST(U_DT_COMPAT);
	REGISTER_U_CONST(U_DT_CIRCLE);
	REGISTER_U_CONST(U_DT_FINAL);
	REGISTER_U_CONST(U_DT_FONT);
	REGISTER_U_CONST(U_DT_FRACTION);
	REGISTER_U_CONST(U_DT_INITIAL);
	REGISTER_U_CONST(U_DT_ISOLATED);
	REGISTER_U_CONST(U_DT_MEDIAL);
	REGISTER_U_CONST(U_DT_NARROW);
	REGISTER_U_CONST(U_DT_NOBREAK);
	REGISTER_U_CONST(U_DT_SMALL);
	REGISTER_U_CONST(U_DT_SQUARE);
	REGISTER_U_CONST(U_DT_SUB);
	REGISTER_U_CONST(U_DT_SUPER);
	REGISTER_U_CONST(U_DT_VERTICAL);
	REGISTER_U_CONST(U_DT_WIDE);
}
/* }}} */

/* {{{ Joining type constants */
static void php_register_joining_type_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_JT_NON_JOINING);
	REGISTER_U_CONST(U_JT_JOIN_CAUSING);
	REGISTER_U_CONST(U_JT_DUAL_JOINING);
	REGISTER_U_CONST(U_JT_LEFT_JOINING);
	REGISTER_U_CONST(U_JT_RIGHT_JOINING);
	REGISTER_U_CONST(U_JT_TRANSPARENT);
}
/* }}} */

/* {{{ Joining group constants */
static void php_register_joining_group_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_JG_NO_JOINING_GROUP);
	REGISTER_U_CONST(U_JG_AIN);
	REGISTER_U_CONST(U_JG_ALAPH);
	REGISTER_U_CONST(U_JG_ALEF);
	REGISTER_U_CONST(U_JG_BEH);
	REGISTER_U_CONST(U_JG_BETH);
	REGISTER_U_CONST(U_JG_DAL);
	REGISTER_U_CONST(U_JG_DALATH_RISH);
	REGISTER_U_CONST(U_JG_E);
	REGISTER_U_CONST(U_JG_FEH);
	REGISTER_U_CONST(U_JG_FINAL_SEMKATH);
	REGISTER_U_CONST(U_JG_GAF);
	REGISTER_U_CONST(U_JG_GAMAL);
	REGISTER_U_CONST(U_JG_HAH);
	REGISTER_U_CONST(U_JG_HAMZA_ON_HEH_GOAL);
	REGISTER_U_CONST(U_JG_HE);
	REGISTER_U_CONST(U_JG_HEH);
	REGISTER_U_CONST(U_JG_HEH_GOAL);
	REGISTER_U_CONST(U_JG_HETH);
	REGISTER_U_CONST(U_JG_KAF);
	REGISTER_U_CONST(U_JG_KAPH);
	REGISTER_U_CONST(U_JG_KNOTTED_HEH);
	REGISTER_U_CONST(U_JG_LAM);
	REGISTER_U_CONST(U_JG_LAMADH);
	REGISTER_U_CONST(U_JG_MEEM);
	REGISTER_U_CONST(U_JG_MIM);
	REGISTER_U_CONST(U_JG_NOON);
	REGISTER_U_CONST(U_JG_NUN);
	REGISTER_U_CONST(U_JG_PE);
	REGISTER_U_CONST(U_JG_QAF);
	REGISTER_U_CONST(U_JG_QAPH);
	REGISTER_U_CONST(U_JG_REH);
	REGISTER_U_CONST(U_JG_REVERSED_PE);
	REGISTER_U_CONST(U_JG_SAD);
	REGISTER_U_CONST(U_JG_SADHE);
	REGISTER_U_CONST(U_JG_SEEN);
	REGISTER_U_CONST(U_JG_SEMKATH);
	REGISTER_U_CONST(U_JG_SHIN);
	REGISTER_U_CONST(U_JG_SWASH_KAF);
	REGISTER_U_CONST(U_JG_SYRIAC_WAW);
	REGISTER_U_CONST(U_JG_TAH);
	REGISTER_U_CONST(U_JG_TAW);
	REGISTER_U_CONST(U_JG_TEH_MARBUTA);
	REGISTER_U_CONST(U_JG_TETH);
	REGISTER_U_CONST(U_JG_WAW);
	REGISTER_U_CONST(U_JG_YEH);
	REGISTER_U_CONST(U_JG_YEH_BARREE);
	REGISTER_U_CONST(U_JG_YEH_WITH_TAIL);
	REGISTER_U_CONST(U_JG_YUDH);
	REGISTER_U_CONST(U_JG_YUDH_HE);
	REGISTER_U_CONST(U_JG_ZAIN);
	REGISTER_U_CONST(U_JG_FE);
	REGISTER_U_CONST(U_JG_KHAPH);
	REGISTER_U_CONST(U_JG_ZHAIN);
}
/* }}} */

/* {{{ Grapheme cluster break constants */
static void php_register_grapheme_cluster_break_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_GCB_OTHER);
	REGISTER_U_CONST(U_GCB_CONTROL);
	REGISTER_U_CONST(U_GCB_CR);
	REGISTER_U_CONST(U_GCB_EXTEND);
	REGISTER_U_CONST(U_GCB_L);
	REGISTER_U_CONST(U_GCB_LF);
	REGISTER_U_CONST(U_GCB_LV);
	REGISTER_U_CONST(U_GCB_LVT);
	REGISTER_U_CONST(U_GCB_T);
	REGISTER_U_CONST(U_GCB_V);
}
/* }}} */

/* {{{ Work break constants */
static void php_register_word_break_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_WB_OTHER);
	REGISTER_U_CONST(U_WB_ALETTER);
	REGISTER_U_CONST(U_WB_FORMAT);
	REGISTER_U_CONST(U_WB_KATAKANA);
	REGISTER_U_CONST(U_WB_MIDLETTER);
	REGISTER_U_CONST(U_WB_MIDNUM);
	REGISTER_U_CONST(U_WB_NUMERIC);
	REGISTER_U_CONST(U_WB_EXTENDNUMLET);
}
/* }}} */

/* {{{ Sentence break constants */
static void php_register_sentence_break_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_SB_OTHER);
	REGISTER_U_CONST(U_SB_ATERM);
	REGISTER_U_CONST(U_SB_CLOSE);
	REGISTER_U_CONST(U_SB_FORMAT);
	REGISTER_U_CONST(U_SB_LOWER);
	REGISTER_U_CONST(U_SB_NUMERIC);
	REGISTER_U_CONST(U_SB_OLETTER);
	REGISTER_U_CONST(U_SB_SEP);
	REGISTER_U_CONST(U_SB_SP);
	REGISTER_U_CONST(U_SB_STERM);
	REGISTER_U_CONST(U_SB_UPPER);
}
/* }}} */

/* {{{ Line break constants */
static void php_register_line_break_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_LB_UNKNOWN);
	REGISTER_U_CONST(U_LB_AMBIGUOUS);
	REGISTER_U_CONST(U_LB_ALPHABETIC);
	REGISTER_U_CONST(U_LB_BREAK_BOTH);
	REGISTER_U_CONST(U_LB_BREAK_AFTER);
	REGISTER_U_CONST(U_LB_BREAK_BEFORE);
	REGISTER_U_CONST(U_LB_MANDATORY_BREAK);
	REGISTER_U_CONST(U_LB_CONTINGENT_BREAK);
	REGISTER_U_CONST(U_LB_CLOSE_PUNCTUATION);
	REGISTER_U_CONST(U_LB_COMBINING_MARK);
	REGISTER_U_CONST(U_LB_CARRIAGE_RETURN);
	REGISTER_U_CONST(U_LB_EXCLAMATION);
	REGISTER_U_CONST(U_LB_GLUE);
	REGISTER_U_CONST(U_LB_HYPHEN);
	REGISTER_U_CONST(U_LB_IDEOGRAPHIC);
	REGISTER_U_CONST(U_LB_INSEPARABLE);
	REGISTER_U_CONST(U_LB_INFIX_NUMERIC);
	REGISTER_U_CONST(U_LB_LINE_FEED);
	REGISTER_U_CONST(U_LB_NONSTARTER);
	REGISTER_U_CONST(U_LB_NUMERIC);
	REGISTER_U_CONST(U_LB_OPEN_PUNCTUATION);
	REGISTER_U_CONST(U_LB_POSTFIX_NUMERIC);
	REGISTER_U_CONST(U_LB_PREFIX_NUMERIC);
	REGISTER_U_CONST(U_LB_QUOTATION);
	REGISTER_U_CONST(U_LB_COMPLEX_CONTEXT);
	REGISTER_U_CONST(U_LB_SURROGATE);
	REGISTER_U_CONST(U_LB_SPACE);
	REGISTER_U_CONST(U_LB_BREAK_SYMBOLS);
	REGISTER_U_CONST(U_LB_ZWSPACE);
	REGISTER_U_CONST(U_LB_NEXT_LINE);
	REGISTER_U_CONST(U_LB_WORD_JOINER);
	REGISTER_U_CONST(U_LB_H2);
	REGISTER_U_CONST(U_LB_H3);
	REGISTER_U_CONST(U_LB_JL);
	REGISTER_U_CONST(U_LB_JT);
	REGISTER_U_CONST(U_LB_JV);
}
/* }}} */

/* {{{ Numeric type constants */
static void php_register_numeric_type_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_NT_NONE);
	REGISTER_U_CONST(U_NT_DECIMAL);
	REGISTER_U_CONST(U_NT_DIGIT);
	REGISTER_U_CONST(U_NT_NUMERIC);
}
/* }}} */

/* {{{ Hangul syllable type constants */
static void php_register_hangul_syllable_constants(TSRMLS_D)
{
	REGISTER_U_CONST(U_HST_NOT_APPLICABLE);
	REGISTER_U_CONST(U_HST_LEADING_JAMO);
	REGISTER_U_CONST(U_HST_VOWEL_JAMO);
	REGISTER_U_CONST(U_HST_TRAILING_JAMO);
	REGISTER_U_CONST(U_HST_LV_SYLLABLE);
	REGISTER_U_CONST(U_HST_LVT_SYLLABLE);
}
/* }}} */

/* {{{ Miscellaneous constants */
static void php_register_misc_constants(TSRMLS_D)
{
	zend_declare_class_constant_double(u_const_ce, "NO_NUMERIC_VALUE",
									   sizeof("NO_NUMERIC_VALUE")-1, U_NO_NUMERIC_VALUE TSRMLS_CC);

	/* Min and max codepoint values */
	REGISTER_U_CONST(UCHAR_MIN_VALUE);
	REGISTER_U_CONST(UCHAR_MAX_VALUE);

	/* Property name constants */
	REGISTER_U_CONST(U_SHORT_PROPERTY_NAME);
	REGISTER_U_CONST(U_LONG_PROPERTY_NAME);

	/* Case folding constants */
	REGISTER_U_CONST(U_FOLD_CASE_DEFAULT);
	REGISTER_U_CONST(U_FOLD_CASE_EXCLUDE_SPECIAL_I);
}
/* }}} */

void php_register_unicode_constants(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "U", NULL);
	u_const_ce = zend_register_internal_class(&ce TSRMLS_CC);
	u_const_ce->ce_flags = ZEND_ACC_FINAL_CLASS;

	php_register_property_constants(TSRMLS_C);
	php_register_general_category_constants(TSRMLS_C);
	php_register_char_direction_constants(TSRMLS_C);
	php_register_block_constants(TSRMLS_C);
	php_register_east_asian_width_constants(TSRMLS_C);
	php_register_decomposition_type_constants(TSRMLS_C);
	php_register_joining_type_constants(TSRMLS_C);
	php_register_joining_group_constants(TSRMLS_C);
	php_register_grapheme_cluster_break_constants(TSRMLS_C);
	php_register_word_break_constants(TSRMLS_C);
	php_register_sentence_break_constants(TSRMLS_C);
	php_register_line_break_constants(TSRMLS_C);
	php_register_numeric_type_constants(TSRMLS_C);
	php_register_hangul_syllable_constants(TSRMLS_C);
	php_register_misc_constants(TSRMLS_C);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
