/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2000 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 0.92 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        | 
   | available at through the world-wide-web at                           |
   | http://www.zend.com/license/0_92.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/


#ifndef _OPERATORS_H
#define _OPERATORS_H

#define MAX_LENGTH_OF_LONG 18
#define MAX_LENGTH_OF_DOUBLE 32

ZEND_API int add_function(zval *result, zval *op1, zval *op2);
ZEND_API int sub_function(zval *result, zval *op1, zval *op2);
ZEND_API int mul_function(zval *result, zval *op1, zval *op2);
ZEND_API int div_function(zval *result, zval *op1, zval *op2);
ZEND_API int mod_function(zval *result, zval *op1, zval *op2);
ZEND_API int boolean_or_function(zval *result, zval *op1, zval *op2);
ZEND_API int boolean_xor_function(zval *result, zval *op1, zval *op2);
ZEND_API int boolean_and_function(zval *result, zval *op1, zval *op2);
ZEND_API int boolean_not_function(zval *result, zval *op1);
ZEND_API int bitwise_not_function(zval *result, zval *op1);
ZEND_API int bitwise_or_function(zval *result, zval *op1, zval *op2);
ZEND_API int bitwise_and_function(zval *result, zval *op1, zval *op2);
ZEND_API int bitwise_xor_function(zval *result, zval *op1, zval *op2);
ZEND_API int shift_left_function(zval *result, zval *op1, zval *op2);
ZEND_API int shift_right_function(zval *result, zval *op1, zval *op2);
ZEND_API int concat_function(zval *result, zval *op1, zval *op2);

ZEND_API int is_equal_function(zval *result, zval *op1, zval *op2);
ZEND_API int is_identical_function(zval *result, zval *op1, zval *op2);
ZEND_API int is_not_identical_function(zval *result, zval *op1, zval *op2);
ZEND_API int is_not_equal_function(zval *result, zval *op1, zval *op2);
ZEND_API int is_smaller_function(zval *result, zval *op1, zval *op2);
ZEND_API int is_smaller_or_equal_function(zval *result, zval *op1, zval *op2);
ZEND_API inline int is_numeric_string(char *str, int length, long *lval, double *dval);

ZEND_API int increment_function(zval *op1);
ZEND_API int decrement_function(zval *op2);

BEGIN_EXTERN_C()
ZEND_API void convert_scalar_to_number(zval *op);
ZEND_API void _convert_to_string(zval *op ZEND_FILE_LINE_DC);
ZEND_API void convert_to_long(zval *op);
ZEND_API void convert_to_double(zval *op);
ZEND_API void convert_to_long_base(zval *op, int base);
ZEND_API void convert_to_null(zval *op);
ZEND_API void convert_to_boolean(zval *op);
ZEND_API void convert_to_array(zval *op);
ZEND_API void convert_to_object(zval *op);
ZEND_API int add_char_to_string(zval *result, zval *op1, zval *op2);
ZEND_API int add_string_to_string(zval *result, zval *op1, zval *op2);
#define convert_to_string(op)			_convert_to_string((op) ZEND_FILE_LINE_CC)
END_EXTERN_C()

ZEND_API int zval_is_true(zval *op);
ZEND_API int compare_function(zval *result, zval *op1, zval *op2);
ZEND_API int numeric_compare_function(zval *result, zval *op1, zval *op2);
ZEND_API int string_compare_function(zval *result, zval *op1, zval *op2);

ZEND_API void zend_str_tolower(char *str, unsigned int length);
ZEND_API int zend_binary_zval_strcmp(zval *s1, zval *s2);
ZEND_API int zend_binary_zval_strncmp(zval *s1, zval *s2, zval *s3);
ZEND_API int zend_binary_zval_strcasecmp(zval *s1, zval *s2);
ZEND_API int zend_binary_strcmp(char *s1, uint len1, char *s2, uint len2);
ZEND_API int zend_binary_strncmp(char *s1, uint len1, char *s2, uint len2, uint length);
ZEND_API int zend_binary_strcasecmp(char *s1, uint len1, char *s2, uint len2);

ZEND_API void zendi_smart_strcmp(zval *result, zval *s1, zval *s2);

#define convert_to_ex_master(ppzv, lower_type, upper_type)	\
	if ((*ppzv)->type!=IS_##upper_type) {					\
		if (!(*ppzv)->is_ref) {								\
			SEPARATE_ZVAL(ppzv);							\
		}													\
		convert_to_##lower_type(*ppzv);						\
	}

#define convert_to_writable_ex_master(ppzv, lower_type, upper_type)	\
	if ((*ppzv)->type!=IS_##upper_type) {							\
		SEPARATE_ZVAL(ppzv);										\
		convert_to_##lower_type(*ppzv);								\
	}


#define convert_to_boolean_ex(ppzv)	convert_to_ex_master(ppzv, boolean, BOOL)
#define convert_to_long_ex(ppzv)	convert_to_ex_master(ppzv, long, LONG)
#define convert_to_double_ex(ppzv)	convert_to_ex_master(ppzv, double, DOUBLE)
#define convert_to_string_ex(ppzv)	convert_to_ex_master(ppzv, string, STRING)
#define convert_to_array_ex(ppzv)	convert_to_ex_master(ppzv, array, ARRAY)
#define convert_to_object_ex(ppzv)	convert_to_ex_master(ppzv, object, OBJECT)
#define convert_to_null_ex(ppzv)	convert_to_ex_master(ppzv, null, NULL)

#define convert_to_writable_boolean_ex(ppzv)	convert_to_writable_ex_master(ppzv, boolean, BOOL)
#define convert_to_writable_long_ex(ppzv)		convert_to_writable_ex_master(ppzv, long, LONG)
#define convert_to_writable_double_ex(ppzv)		convert_to_writable_ex_master(ppzv, double, DOUBLE)
#define convert_to_writable_string_ex(ppzv)		convert_to_writable_ex_master(ppzv, string, STRING)
#define convert_to_writable_array_ex(ppzv)		convert_to_writable_ex_master(ppzv, array, ARRAY)
#define convert_to_writable_object_ex(ppzv)		convert_to_writable_ex_master(ppzv, object, OBJECT)
#define convert_to_writable_null_ex(ppzv)		convert_to_writable_ex_master(ppzv, null, NULL)


#define convert_scalar_to_number_ex(ppzv)							\
	if ((*ppzv)->type!=IS_LONG && (*ppzv)->type!=IS_DOUBLE) {		\
		if (!(*ppzv)->is_ref) {										\
			SEPARATE_ZVAL(ppzv);									\
		}															\
		convert_scalar_to_number(*ppzv);							\
	}



#define Z_LVAL(zval)		(zval).value.lval
#define Z_DVAL(zval)		(zval).value.dval
#define Z_STRVAL(zval)		(zval).value.str.val
#define Z_STRLEN(zval)		(zval).value.str.len
#define Z_ARRVAL(zval)		(zval).value.ht

#define Z_LVAL_P(zval_p)		Z_LVAL(*zval_p)
#define Z_DVAL_P(zval_p)		Z_DVAL(*zval_p)
#define Z_STRVAL_P(zval_p)		Z_STRVAL(*zval_p)
#define Z_STRLEN_P(zval_p)		Z_STRLEN(*zval_p)
#define Z_ARRVAL_P(zval_p)		Z_ARRVAL(*zval_p)

#define Z_LVAL_PP(zval_pp)		Z_LVAL(**zval_pp)
#define Z_DVAL_PP(zval_pp)		Z_DVAL(**zval_pp)
#define Z_STRVAL_PP(zval_pp)	Z_STRVAL(**zval_pp)
#define Z_STRLEN_PP(zval_pp)	Z_STRLEN(**zval_pp)
#define Z_ARRVAL_PP(zval_pp)	Z_ARRVAL(**zval_pp)

#define Z_TYPE(zval)		(zval).type
#define Z_TYPE_P(zval_p)	Z_TYPE(*zval_p)
#define Z_TYPE_PP(zval_pp)	Z_TYPE(**zval_pp)

#endif
