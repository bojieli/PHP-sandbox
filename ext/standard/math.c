/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_01.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jim Winstead (jimw@php.net)                                 |
   |          Stig S�ther Bakken <ssb@guardian.no>                        |
   |          Zeev Suraski <zeev@zend.com>                                |
   | PHP 4.0 patches by Thies C. Arntzen (thies@digicol.de)               |
   +----------------------------------------------------------------------+
*/

/* $Id: */

#include "php.h"
#include "phpmath.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


char *_php_math_number_format(double, int, char ,char);

/* {{{ proto int abs(int number)
   Return the absolute value of the number */
PHP_FUNCTION(abs) 
{
	zval **value;
	
	if (ARG_COUNT(ht)!=1||zend_get_parameters_ex(1,&value)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_scalar_to_number_ex(value);
	
	if ((*value)->type == IS_DOUBLE) {
		RETURN_DOUBLE(fabs((*value)->value.dval));
	} else if ((*value)->type == IS_LONG) {
		RETURN_LONG((*value)->value.lval < 0 ? -(*value)->value.lval : (*value)->value.lval);
	}

	RETURN_FALSE;
}

/* }}} */ 
/* {{{ proto int ceil(double number)
   Returns the next highest integer value of the number */

PHP_FUNCTION(ceil) 
{
	zval **value;
	
	if (ARG_COUNT(ht)!=1||zend_get_parameters_ex(1,&value)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_scalar_to_number_ex(value);

	if ((*value)->type == IS_DOUBLE) {
		RETURN_LONG((long)ceil((*value)->value.dval));
	} else if ((*value)->type == IS_LONG) {
		RETURN_LONG((*value)->value.lval);
	}

	RETURN_FALSE;
}

/* }}} */
/* {{{ proto int floor(double number)
   Returns the next lowest integer value from the number */

PHP_FUNCTION(floor) {
	zval **value;
	
	if (ARG_COUNT(ht)!=1||zend_get_parameters_ex(1,&value)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_scalar_to_number_ex(value);

	if ((*value)->type == IS_DOUBLE) {
		RETURN_LONG((long)floor((*value)->value.dval));
	} else if ((*value)->type == IS_LONG) {
		RETURN_LONG((*value)->value.lval);
	}

	RETURN_FALSE;
}

/* }}} */
/* {{{ proto int round(double number)
   Returns the rounded value of the number */

#ifndef HAVE_RINT
/* emulate rint */
inline double rint(double n)
{
	double i, f;
	f = modf(n, &i);
	if (f > .5)
		i++;
	else if (f < -.5)
		i--;
	return i;
}
#endif

PHP_FUNCTION(round)
{
	zval **value;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &value) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_scalar_to_number_ex(value);

	if ((*value)->type == IS_DOUBLE) {
		double d;
		d=rint((*value)->value.dval);
		if(d==0.0) d=0.0; /* workaround for rint() returning -0 instead of 0 */
		RETURN_DOUBLE(d);
	} else if ((*value)->type == IS_LONG) {
		RETURN_DOUBLE((double)(*value)->value.lval);
	}
	RETURN_FALSE;
}

/* }}} */
/* {{{ proto double sin(double number)
   Returns the sine of the number in radians */

PHP_FUNCTION(sin)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = sin((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double cos(double number)
   Returns the cosine of the number in radians */

PHP_FUNCTION(cos)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = cos((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}
/* }}} */
/* {{{ proto double tan(double number)
   Returns the tangent of the number in radians */
PHP_FUNCTION(tan)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = tan((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double asin(double number)
   Returns the arc sine of the number in radians */

PHP_FUNCTION(asin)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = asin((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double acos(double number)
   Return the arc cosine of the number in radians */

PHP_FUNCTION(acos)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = acos((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double atan(double number)
   Returns the arc tangent of the number in radians */

PHP_FUNCTION(atan)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = atan((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double atan2(double y, double x)
   Returns the arc tangent of y/x, with the resulting quadrant determined by the signs of y and x */

PHP_FUNCTION(atan2)
{
	zval **num1, **num2;

	if (ARG_COUNT(ht) != 2 || zend_get_parameters_ex(2, &num1, &num2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num1);
	convert_to_double_ex(num2);
	return_value->value.dval = atan2((*num1)->value.dval,(*num2)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double pi(void)
   Returns an approximation of pi */

PHP_FUNCTION(pi)
{
	return_value->value.dval = M_PI;
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double pow(double base, double exponent)
   Returns base raised to the power of exponent */

PHP_FUNCTION(pow)
{
	zval **num1, **num2;
	
	if (ARG_COUNT(ht) != 2 || zend_get_parameters_ex(2,&num1,&num2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num1);
	convert_to_double_ex(num2);
	RETURN_DOUBLE(pow((*num1)->value.dval, (*num2)->value.dval));
}

/* }}} */
/* {{{ proto double exp(double number)
   Returns e raised to the power of the number */

PHP_FUNCTION(exp)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = exp((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double log(double number)
   Returns the natural logarithm of the number */

PHP_FUNCTION(log)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = log((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double log10(double number)
   Returns the base-10 logarithm of the number */

PHP_FUNCTION(log10)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = log10((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double sqrt(double number)
   Returns the square root of the number */

PHP_FUNCTION(sqrt)
{
	zval **num;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &num) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(num);
	return_value->value.dval = sqrt((*num)->value.dval);
	return_value->type = IS_DOUBLE;
}

/* }}} */
/* {{{ proto double deg2rad(double number)
   Converts the number in degrees to the radian equivalent */

PHP_FUNCTION(deg2rad)
{
	zval **deg;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &deg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(deg);
	RETVAL_DOUBLE(((*deg)->value.dval / 180.0) * M_PI);
}

/* }}} */
/* {{{ proto double rad2deg(double number)
   Converts the radian number to the equivalent number in degrees */

PHP_FUNCTION(rad2deg)
{
	zval **rad;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &rad) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_double_ex(rad);
	RETVAL_DOUBLE(((*rad)->value.dval / M_PI) * 180);
}

/* }}} */
/* {{{ _php_math_basetolong */

/*
 * Convert a string representation of a base(2-36) number to a long.
 */
static long
_php_math_basetolong(zval *arg, int base) {
	long mult = 1, num = 0, digit;
	int i;
	char c, *s;

	if (arg->type != IS_STRING || base < 2 || base > 36) {
		return 0;
	}

	s = arg->value.str.val;

	for (i = arg->value.str.len - 1; i >= 0; i--, mult *= base) {
		c = toupper(s[i]);
		if (c >= '0' && c <= '9') {
			digit = (c - '0');
		} else if (c >= 'A' && c <= 'Z') {
			digit = (c - 'A' + 10);
		} else {
			continue;
		}
		if (digit >= base) {
			continue;
		}
		num += mult * digit;
	}

	return num;
}

/* }}} */
/* {{{ _php_math_longtobase */

/*
 * Convert a long to a string containing a base(2-36) representation of
 * the number.
 */
static char *
_php_math_longtobase(zval *arg, int base)
{
	static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char *result, *ptr, *ret;
	int len, digit;
	long value;

	if (arg->type != IS_LONG || base < 2 || base > 36) {
		return empty_string;
	}

	value = arg->value.lval;

	/* allocates space for the longest possible result with the lowest base */
	len = (sizeof(arg->value.lval) * 8) + 1;
	result = emalloc((sizeof(arg->value.lval) * 8) + 1);

	ptr = result + len - 1;
	*ptr-- = '\0';

	do {
		digit = value % base;
		*ptr = digits[digit];
		value /= base;
	}
	while (ptr-- > result && value);
	ptr++;
	ret = estrdup(ptr);
	efree(result);

	return ret;
}	

/* }}} */
/* {{{ proto int bindec(string binary_number)
   Returns the decimal equivalent of the binary number */

PHP_FUNCTION(bindec)
{
	zval **arg;
	long ret;
	
	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(arg);
	ret = _php_math_basetolong(*arg, 2);

	RETVAL_LONG(ret);
}

/* }}} */
/* {{{ proto int hexdec(string hexadimal_number)
   Returns the decimal equivalent of the hexadecimal number */

PHP_FUNCTION(hexdec)
{
	zval **arg;
	long ret;
	
	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(arg);

	ret = _php_math_basetolong(*arg, 16);
	RETVAL_LONG(ret);
}

/* }}} */
/* {{{ proto int octdec(string octal_number)
   Returns the decimal equivalent of an octal string */

PHP_FUNCTION(octdec)
{
	zval **arg;
	long ret;
	
	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(arg);

	ret = _php_math_basetolong(*arg, 8);
	RETVAL_LONG(ret);
}

/* }}} */
/* {{{ proto string decbin(int decimal_number)
   Returns a string containing a binary representation of the number */

PHP_FUNCTION(decbin)
{
	zval **arg;
	char *result;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(arg);

	result = _php_math_longtobase(*arg, 2);
	return_value->type = IS_STRING;
	return_value->value.str.len = strlen(result);
	return_value->value.str.val = result;
}

/* }}} */
/* {{{ proto string decoct(int decimal_number)
   Returns a string containing an octal representation of the given number */

PHP_FUNCTION(decoct)
{
	zval **arg;
	char *result;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(arg);

	result = _php_math_longtobase(*arg, 8);
	return_value->type = IS_STRING;
	return_value->value.str.len = strlen(result);
	return_value->value.str.val = result;
}

/* }}} */
/* {{{ proto string dechex(int decimal_number)
   Returns a string containing a hexadecimal representation of the given number */

PHP_FUNCTION(dechex)
{
	zval **arg;
	char *result;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(arg);

	result = _php_math_longtobase(*arg, 16);
	return_value->type = IS_STRING;
	return_value->value.str.len = strlen(result);
	return_value->value.str.val = result;
}

/* }}} */
/* {{{ proto string base_convert(string number, int frombase, int tobase)
   Converts a number in a string from any base <= 36 to any base <= 36.
*/

PHP_FUNCTION(base_convert)
{
	zval **number, **frombase, **tobase, temp;
	char *result;

	if (ARG_COUNT(ht) != 3 || zend_get_parameters_ex(3, &number, &frombase, &tobase) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string_ex(number);
	convert_to_long_ex(frombase);
	convert_to_long_ex(tobase);
	if ((*frombase)->value.lval < 2 || (*frombase)->value.lval > 36) {
		php_error(E_WARNING, "base_convert: invalid `from base' (%d)",(*frombase)->value.lval);
		RETURN_FALSE;
	}
	if ((*tobase)->value.lval < 2 || (*tobase)->value.lval > 36) {
		php_error(E_WARNING, "base_convert: invalid `to base' (%d)",(*tobase)->value.lval);
		RETURN_FALSE;
	}
	temp.type = IS_LONG;
	temp.value.lval = _php_math_basetolong(*number, (*frombase)->value.lval);
	result = _php_math_longtobase(&temp, (*tobase)->value.lval);
	RETVAL_STRING(result, 0);
} 

/* }}} */
/* {{{ _php_math_number_format */

char *_php_math_number_format(double d,int dec,char dec_point,char thousand_sep)
{
	char *tmpbuf,*resbuf;
	char *s,*t;  /* source, target */
	int tmplen,reslen=0;
	int count=0;
	int is_negative=0;
	
	if (d<0) {
		is_negative=1;
		d = -d;
	}
	dec = MAX(0,dec);
	tmpbuf = (char *) emalloc(32+dec);
	
	tmplen=sprintf(tmpbuf,"%.*f",dec,d);

	if (!isdigit((int)tmpbuf[0])) {
		return tmpbuf;
	}

	for (t=tmpbuf; *t; t++) {
		if (*t=='.') {
			*t = dec_point;
		}
	}
	if (dec) {
		reslen = dec+1 + (tmplen-dec-1) + (tmplen-1-dec-1)/3;
	} else {
		reslen = tmplen+(tmplen-1)/3;
	}
	if (is_negative) {
		reslen++;
	}
	resbuf = (char *) emalloc(reslen+1);
	
	s = tmpbuf+tmplen-1;
	t = resbuf+reslen;
	*t-- = 0;
	
	if (dec) {
		while (*s!=dec_point) {
			*t-- = *s--;
		}
		*t-- = *s--;  /* copy that dot */
	}
	
	while(s>=tmpbuf) {
		*t-- = *s--;
		if ((++count%3)==0 && s>=tmpbuf) {
			*t-- = thousand_sep;
		}
	}
	if (is_negative) {
		*t-- = '-';
	}
	efree(tmpbuf);
	return resbuf;
}

/* }}} */
/* {{{ proto string number_format(double number [, int num_decimal_places [, string  dec_seperator, string thousands_seperator)]])
   Formats a number with grouped thousands */

PHP_FUNCTION(number_format)
{
	zval **num,**dec,**t_s,**d_p;
	char thousand_sep=',', dec_point='.';
	
	switch(ARG_COUNT(ht)) {
	case 1:
		if (zend_get_parameters_ex(1, &num)==FAILURE) {
			RETURN_FALSE;
		}
		convert_to_double_ex(num);
		RETURN_STRING(_php_math_number_format((*num)->value.dval,0,dec_point,thousand_sep),0);
		break;
	case 2:
		if (zend_get_parameters_ex(2, &num, &dec)==FAILURE) {
			RETURN_FALSE;
		}
		convert_to_double_ex(num);
		convert_to_long_ex(dec);
		RETURN_STRING(_php_math_number_format((*num)->value.dval,(*dec)->value.lval,dec_point,thousand_sep),0);
		break;
	case 4:
		if (zend_get_parameters_ex(4, &num, &dec, &d_p, &t_s)==FAILURE) {
			RETURN_FALSE;
		}
		convert_to_double_ex(num);
		convert_to_long_ex(dec);
		convert_to_string_ex(d_p);
		convert_to_string_ex(t_s);
		if ((*d_p)->value.str.len==1) {
			dec_point=(*d_p)->value.str.val[0];
		}
		if ((*t_s)->value.str.len==1) {
			thousand_sep=(*t_s)->value.str.val[0];
		}
		RETURN_STRING(_php_math_number_format((*num)->value.dval,(*dec)->value.lval,dec_point,thousand_sep),0);
		break;
	default:
		WRONG_PARAM_COUNT;
		break;
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
