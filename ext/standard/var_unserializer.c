/* Generated by re2c 0.9.2 on Tue Mar 23 23:12:19 2004 */
#line 1 "/usr/src/php5/ext/standard/var_unserializer.re"
/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2004 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Sascha Schumann <sascha@schumann.cx>                         |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php.h"
#include "ext/standard/php_var.h"
#include "php_incomplete_class.h"

/* {{{ reference-handling for unserializer: var_* */
#define VAR_ENTRIES_MAX 1024

typedef struct {
	zval *data[VAR_ENTRIES_MAX];
	int used_slots;
	void *next;
} var_entries;

static inline void var_push(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = var_hashx->first, *prev = NULL;

	while (var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
		prev = var_hash;
		var_hash = var_hash->next;
	}

	if (!var_hash) {
		var_hash = emalloc(sizeof(var_entries));
		var_hash->used_slots = 0;
		var_hash->next = 0;

		if (!var_hashx->first)
			var_hashx->first = var_hash;
		else
			prev->next = var_hash;
	}

	var_hash->data[var_hash->used_slots++] = *rval;
}

PHPAPI void var_replace(php_unserialize_data_t *var_hashx, zval *ozval, zval **nzval)
{
	int i;
	var_entries *var_hash = var_hashx->first;
	
	while (var_hash) {
		for (i = 0; i < var_hash->used_slots; i++) {
			if (var_hash->data[i] == ozval) {
				var_hash->data[i] = *nzval;
				return;
			}
		}
		var_hash = var_hash->next;
	}
}

static int var_access(php_unserialize_data_t *var_hashx, int id, zval ***store)
{
	var_entries *var_hash = var_hashx->first;
	
	while (id >= VAR_ENTRIES_MAX && var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
		var_hash = var_hash->next;
		id -= VAR_ENTRIES_MAX;
	}

	if (!var_hash) return !SUCCESS;

	if (id >= var_hash->used_slots) return !SUCCESS;

	*store = &var_hash->data[id];

	return SUCCESS;
}

PHPAPI void var_destroy(php_unserialize_data_t *var_hashx)
{
	void *next;
	var_entries *var_hash = var_hashx->first;
	
	while (var_hash) {
		next = var_hash->next;
		efree(var_hash);
		var_hash = next;
	}
}

/* }}} */

#define YYFILL(n) do { } while (0)
#define YYCTYPE unsigned char
#define YYCURSOR cursor
#define YYLIMIT limit
#define YYMARKER marker


#line 118 "/usr/src/php5/ext/standard/var_unserializer.re"




static inline int parse_iv2(const char *p, const char **q)
{
	char cursor;
	int result = 0;
	int neg = 0;

	switch (*p) {
		case '-':
			neg++;
			/* fall-through */
		case '+':
			p++;
	}
	
	while (1) {
		cursor = *p;
		if (cursor >= '0' && cursor <= '9') {
			result = result * 10 + cursor - '0';
		} else {
			break;
		}
		p++;
	}
	if (q) *q = p;
	if (neg) return -result;
	return result;
}

static inline int parse_iv(const char *p)
{
	return parse_iv2(p, NULL);
}

#define UNSERIALIZE_PARAMETER zval **rval, const char **p, const char *max, php_unserialize_data_t *var_hash TSRMLS_DC
#define UNSERIALIZE_PASSTHRU rval, p, max, var_hash TSRMLS_CC

static inline int process_nested_data(UNSERIALIZE_PARAMETER, HashTable *ht, int elements)
{
	while (elements-- > 0) {
		zval *key, *data;

		ALLOC_INIT_ZVAL(key);

		if (!php_var_unserialize(&key, p, max, NULL TSRMLS_CC)) {
			zval_dtor(key);
			FREE_ZVAL(key);
			return 0;
		}

		ALLOC_INIT_ZVAL(data);

		if (!php_var_unserialize(&data, p, max, var_hash TSRMLS_CC)) {
			zval_dtor(key);
			FREE_ZVAL(key);
			zval_dtor(data);
			FREE_ZVAL(data);
			return 0;
		}

		switch (Z_TYPE_P(key)) {
			case IS_LONG:
				zend_hash_index_update(ht, Z_LVAL_P(key), &data, sizeof(data), NULL);
				break;
			case IS_STRING:
				zend_hash_update(ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, &data, sizeof(data), NULL);
				break;

		}
		
		zval_dtor(key);
		FREE_ZVAL(key);
	}

	return 1;
}

static inline int finish_nested_data(UNSERIALIZE_PARAMETER)
{
	if (*((*p)++) == '}') 
		return 1;

#if SOMETHING_NEW_MIGHT_LEAD_TO_CRASH_ENABLE_IF_YOU_ARE_BRAVE
	zval_ptr_dtor(rval);
#endif
	return 0;
}

static inline int object_common1(UNSERIALIZE_PARAMETER, zend_class_entry *ce)
{
	int elements;

	elements = parse_iv2((*p) + 2, p);

	(*p) += 2;
	
	object_init_ex(*rval, ce);
	return elements;
}

static inline int object_common2(UNSERIALIZE_PARAMETER, int elements)
{
	zval *retval_ptr = NULL;
	zval fname;

	if (!process_nested_data(UNSERIALIZE_PASSTHRU, Z_OBJPROP_PP(rval), elements)) {
		return 0;
	}

	if(Z_OBJCE_PP(rval) != PHP_IC_ENTRY) {
		INIT_PZVAL(&fname);
		ZVAL_STRINGL(&fname, "__wakeup", sizeof("__wakeup") - 1, 0);
		call_user_function_ex(CG(function_table), rval, &fname, &retval_ptr, 0, 0, 1, NULL TSRMLS_CC);
	}

	if (retval_ptr)
		zval_ptr_dtor(&retval_ptr);

	return finish_nested_data(UNSERIALIZE_PASSTHRU);

}

PHPAPI int php_var_unserialize(UNSERIALIZE_PARAMETER)
{
	const unsigned char *cursor, *limit, *marker, *start;
	zval **rval_ref;

	limit = cursor = *p;
	
	if (var_hash && cursor[0] != 'R') {
		var_push(var_hash, rval);
	}

	start = cursor;

	
	

#line 7 "re2c-output.c"
{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	128, 128, 128, 128, 128, 128, 128, 128, 
	128, 128,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	  0,   0,   0,   0,   0,   0,   0,   0, 
	};
	goto yy0;
yy1:	++YYCURSOR;
yy0:
	if((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
	if(yych <= 'd'){
		if(yych <= 'R'){
			if(yych <= 'N'){
				if(yych <= 'M')	goto yy16;
				goto yy6;
			} else {
				if(yych <= 'O')	goto yy13;
				if(yych <= 'Q')	goto yy16;
				goto yy3;
			}
		} else {
			if(yych <= 'a'){
				if(yych <= '`')	goto yy16;
				goto yy11;
			} else {
				if(yych <= 'b')	goto yy7;
				if(yych <= 'c')	goto yy16;
				goto yy9;
			}
		}
	} else {
		if(yych <= 'q'){
			if(yych <= 'i'){
				if(yych <= 'h')	goto yy16;
				goto yy8;
			} else {
				if(yych == 'o')	goto yy12;
				goto yy16;
			}
		} else {
			if(yych <= '|'){
				if(yych <= 'r')	goto yy5;
				if(yych <= 's')	goto yy10;
				goto yy16;
			} else {
				if(yych <= '}')	goto yy14;
				if(yych <= '\277')	goto yy16;
				goto yy2;
			}
		}
	}
yy2:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy4;
	}
yy3:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy87;
	goto yy4;
yy4:
#line 461 "/usr/src/php5/ext/standard/var_unserializer.re"
{ return 0; }
#line 102 "re2c-output.c"
yy5:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy81;
	goto yy4;
yy6:	yych = *++YYCURSOR;
	if(yych == ';')	goto yy79;
	goto yy4;
yy7:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy73;
	goto yy4;
yy8:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy67;
	goto yy4;
yy9:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy45;
	goto yy4;
yy10:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy38;
	goto yy4;
yy11:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy31;
	goto yy4;
yy12:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy24;
	goto yy4;
yy13:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy17;
	goto yy4;
yy14:	yych = *++YYCURSOR;
	goto yy15;
yy15:
#line 455 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	/* this is the case where we have less data than planned */
	php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unexpected end of serialized data");
	return 0; /* not sure if it should be 0 or 1 here? */
}
#line 147 "re2c-output.c"
yy16:	yych = *++YYCURSOR;
	goto yy4;
yy17:	yych = *++YYCURSOR;
	if(yybm[0+yych] & 128)	goto yy19;
	if(yych != '+')	goto yy2;
	goto yy18;
yy18:	yych = *++YYCURSOR;
	if(yybm[0+yych] & 128)	goto yy19;
	goto yy2;
yy19:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy20;
yy20:	if(yybm[0+yych] & 128)	goto yy19;
	if(yych != ':')	goto yy2;
	goto yy21;
yy21:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy2;
	goto yy22;
yy22:	yych = *++YYCURSOR;
	goto yy23;
yy23:
#line 376 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	int len;
	int elements;
	int len2;
	char *class_name;
	zend_class_entry *ce;
	zend_class_entry **pce;
	int incomplete_class = 0;
	
	zval *user_func;
	zval *retval_ptr;
	zval **args[1];
	zval *arg_func_name;
	
	INIT_PZVAL(*rval);
	len2 = len = parse_iv(start + 2);
	if (len == 0)
		return 0;

	class_name = estrndup(YYCURSOR, len);
	YYCURSOR += len;

	do {
		/* Try to find class directly */
		if (zend_lookup_class(class_name, len2, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
			break;
		}
		
		/* Check for unserialize callback */
		if ((PG(unserialize_callback_func) == NULL) || (PG(unserialize_callback_func)[0] == '\0')) {
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			break;
		}
		
		/* Call unserialize callback */
		MAKE_STD_ZVAL(user_func);
		ZVAL_STRING(user_func, PG(unserialize_callback_func), 1);
		args[0] = &arg_func_name;
		MAKE_STD_ZVAL(arg_func_name);
		ZVAL_STRING(arg_func_name, class_name, 1);
		if (call_user_function_ex(CG(function_table), NULL, user_func, &retval_ptr, 1, args, 0, NULL TSRMLS_CC) != SUCCESS) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "defined (%s) but not found", user_func->value.str.val);
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
			zval_ptr_dtor(&user_func);
			zval_ptr_dtor(&arg_func_name);
			break;
		}
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		
		/* The callback function may have defined the class */
		if (zend_lookup_class(class_name, len2, &pce TSRMLS_CC) == SUCCESS) {
			ce = *pce;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Function %s() hasn't defined the class it was called for", user_func->value.str.val);
			incomplete_class = 1;
			ce = PHP_IC_ENTRY;
		}

		zval_ptr_dtor(&user_func);
		zval_ptr_dtor(&arg_func_name);
		break;
	} while (1);
	
	*p = YYCURSOR;
	elements = object_common1(UNSERIALIZE_PASSTHRU, ce);

	if (incomplete_class) {
		php_store_class_name(*rval, class_name, len2);
	}
	efree(class_name);

	return object_common2(UNSERIALIZE_PASSTHRU, elements);
}
#line 249 "re2c-output.c"
yy24:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy2;
		goto yy25;
	} else {
		if(yych <= '-')	goto yy25;
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy26;
		goto yy2;
	}
yy25:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
	goto yy26;
yy26:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy27;
yy27:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy26;
	if(yych >= ';')	goto yy2;
	goto yy28;
yy28:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy2;
	goto yy29;
yy29:	yych = *++YYCURSOR;
	goto yy30;
yy30:
#line 368 "/usr/src/php5/ext/standard/var_unserializer.re"
{

	INIT_PZVAL(*rval);
	
	return object_common2(UNSERIALIZE_PASSTHRU,
			object_common1(UNSERIALIZE_PASSTHRU, ZEND_STANDARD_CLASS_DEF_PTR));
}
#line 286 "re2c-output.c"
yy31:	yych = *++YYCURSOR;
	if(yych == '+')	goto yy32;
	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy33;
	goto yy2;
yy32:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
	goto yy33;
yy33:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy34;
yy34:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy33;
	if(yych >= ';')	goto yy2;
	goto yy35;
yy35:	yych = *++YYCURSOR;
	if(yych != '{')	goto yy2;
	goto yy36;
yy36:	yych = *++YYCURSOR;
	goto yy37;
yy37:
#line 350 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	int elements = parse_iv(start + 2);

	*p = YYCURSOR;

	INIT_PZVAL(*rval);
	Z_TYPE_PP(rval) = IS_ARRAY;
	ALLOC_HASHTABLE(Z_ARRVAL_PP(rval));

	zend_hash_init(Z_ARRVAL_PP(rval), elements + 1, NULL, ZVAL_PTR_DTOR, 0);

	if (!process_nested_data(UNSERIALIZE_PASSTHRU, Z_ARRVAL_PP(rval), elements)) {
		return 0;
	}

	return finish_nested_data(UNSERIALIZE_PASSTHRU);
}
#line 328 "re2c-output.c"
yy38:	yych = *++YYCURSOR;
	if(yych == '+')	goto yy39;
	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy40;
	goto yy2;
yy39:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
	goto yy40;
yy40:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy41;
yy41:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy40;
	if(yych >= ';')	goto yy2;
	goto yy42;
yy42:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy2;
	goto yy43;
yy43:	yych = *++YYCURSOR;
	goto yy44;
yy44:
#line 330 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	int len;
	char *str;

	len = parse_iv(start + 2);

	if (len == 0) {
		str = empty_string;
	} else {
		str = estrndup(YYCURSOR, len);
	}

	YYCURSOR += len + 2;
	*p = YYCURSOR;

	INIT_PZVAL(*rval);
	ZVAL_STRINGL(*rval, str, len, 0);
	return 1;
}
#line 372 "re2c-output.c"
yy45:	yych = *++YYCURSOR;
	if(yych <= '/'){
		if(yych <= ','){
			if(yych != '+')	goto yy2;
			goto yy46;
		} else {
			if(yych <= '-')	goto yy47;
			if(yych <= '.')	goto yy50;
			goto yy2;
		}
	} else {
		if(yych <= 'I'){
			if(yych <= '9')	goto yy48;
			if(yych <= 'H')	goto yy2;
			goto yy52;
		} else {
			if(yych == 'N')	goto yy51;
			goto yy2;
		}
	}
yy46:	yych = *++YYCURSOR;
	if(yych == '.')	goto yy50;
	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy48;
	goto yy2;
yy47:	yych = *++YYCURSOR;
	if(yych <= '/'){
		if(yych == '.')	goto yy50;
		goto yy2;
	} else {
		if(yych <= '9')	goto yy48;
		if(yych == 'I')	goto yy52;
		goto yy2;
	}
yy48:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy49;
yy49:	if(yych <= ':'){
		if(yych <= '.'){
			if(yych <= '-')	goto yy2;
			goto yy65;
		} else {
			if(yych <= '/')	goto yy2;
			if(yych <= '9')	goto yy48;
			goto yy2;
		}
	} else {
		if(yych <= 'E'){
			if(yych <= ';')	goto yy55;
			if(yych <= 'D')	goto yy2;
			goto yy60;
		} else {
			if(yych == 'e')	goto yy60;
			goto yy2;
		}
	}
yy50:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy58;
	goto yy2;
yy51:	yych = *++YYCURSOR;
	if(yych == 'A')	goto yy57;
	goto yy2;
yy52:	yych = *++YYCURSOR;
	if(yych != 'N')	goto yy2;
	goto yy53;
yy53:	yych = *++YYCURSOR;
	if(yych != 'F')	goto yy2;
	goto yy54;
yy54:	yych = *++YYCURSOR;
	if(yych != ';')	goto yy2;
	goto yy55;
yy55:	yych = *++YYCURSOR;
	goto yy56;
yy56:
#line 323 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_DOUBLE(*rval, atof(start + 2));
	return 1;
}
#line 456 "re2c-output.c"
yy57:	yych = *++YYCURSOR;
	if(yych == 'N')	goto yy54;
	goto yy2;
yy58:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy59;
yy59:	if(yych <= ';'){
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy58;
		if(yych <= ':')	goto yy2;
		goto yy55;
	} else {
		if(yych <= 'E'){
			if(yych <= 'D')	goto yy2;
			goto yy60;
		} else {
			if(yych != 'e')	goto yy2;
			goto yy60;
		}
	}
yy60:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy2;
		goto yy61;
	} else {
		if(yych <= '-')	goto yy61;
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy62;
		goto yy2;
	}
yy61:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych == '+')	goto yy64;
		goto yy2;
	} else {
		if(yych <= '-')	goto yy64;
		if(yych <= '/')	goto yy2;
		if(yych >= ':')	goto yy2;
		goto yy62;
	}
yy62:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy63;
yy63:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy62;
	if(yych == ';')	goto yy55;
	goto yy2;
yy64:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy62;
	goto yy2;
yy65:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	goto yy66;
yy66:	if(yych <= ';'){
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy65;
		if(yych <= ':')	goto yy2;
		goto yy55;
	} else {
		if(yych <= 'E'){
			if(yych <= 'D')	goto yy2;
			goto yy60;
		} else {
			if(yych == 'e')	goto yy60;
			goto yy2;
		}
	}
yy67:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy2;
		goto yy68;
	} else {
		if(yych <= '-')	goto yy68;
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy69;
		goto yy2;
	}
yy68:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
	goto yy69;
yy69:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy70;
yy70:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy69;
	if(yych != ';')	goto yy2;
	goto yy71;
yy71:	yych = *++YYCURSOR;
	goto yy72;
yy72:
#line 316 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_LONG(*rval, parse_iv(start + 2));
	return 1;
}
#line 560 "re2c-output.c"
yy73:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy2;
		goto yy74;
	} else {
		if(yych <= '-')	goto yy74;
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy75;
		goto yy2;
	}
yy74:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
	goto yy75;
yy75:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy76;
yy76:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy75;
	if(yych != ';')	goto yy2;
	goto yy77;
yy77:	yych = *++YYCURSOR;
	goto yy78;
yy78:
#line 309 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_BOOL(*rval, parse_iv(start + 2));
	return 1;
}
#line 593 "re2c-output.c"
yy79:	yych = *++YYCURSOR;
	goto yy80;
yy80:
#line 302 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_NULL(*rval);
	return 1;
}
#line 604 "re2c-output.c"
yy81:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy2;
		goto yy82;
	} else {
		if(yych <= '-')	goto yy82;
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy83;
		goto yy2;
	}
yy82:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
	goto yy83;
yy83:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy84;
yy84:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy83;
	if(yych != ';')	goto yy2;
	goto yy85;
yy85:	yych = *++YYCURSOR;
	goto yy86;
yy86:
#line 281 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	int id;

 	*p = YYCURSOR;
	if (!var_hash) return 0;

	id = parse_iv(start + 2) - 1;
	if (id == -1 || var_access(var_hash, id, &rval_ref) != SUCCESS) {
		return 0;
	}

	if (*rval != NULL) {
		zval_ptr_dtor(rval);
	}
	*rval = *rval_ref;
	(*rval)->refcount++;
	(*rval)->is_ref = 0;
	
	return 1;
}
#line 651 "re2c-output.c"
yy87:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy2;
		goto yy88;
	} else {
		if(yych <= '-')	goto yy88;
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy89;
		goto yy2;
	}
yy88:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
	goto yy89;
yy89:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy90;
yy90:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy89;
	if(yych != ';')	goto yy2;
	goto yy91;
yy91:	yych = *++YYCURSOR;
	goto yy92;
yy92:
#line 260 "/usr/src/php5/ext/standard/var_unserializer.re"
{
	int id;

 	*p = YYCURSOR;
	if (!var_hash) return 0;

	id = parse_iv(start + 2) - 1;
	if (id == -1 || var_access(var_hash, id, &rval_ref) != SUCCESS) {
		return 0;
	}

	if (*rval != NULL) {
		zval_ptr_dtor(rval);
	}
	*rval = *rval_ref;
	(*rval)->refcount++;
	(*rval)->is_ref = 1;
	
	return 1;
}
#line 698 "re2c-output.c"
}
#line 463 "/usr/src/php5/ext/standard/var_unserializer.re"


	return 0;
}
