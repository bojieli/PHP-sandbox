/* Generated by re2c 0.9.12 on Thu Dec 14 15:59:31 2006 */
#line 1 "ext/standard/var_unserializer.re"
/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
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
	long used_slots;
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

static inline void var_push_dtor(php_unserialize_data_t *var_hashx, zval **rval)
{
	var_entries *var_hash = var_hashx->first_dtor, *prev = NULL;

	while (var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
		prev = var_hash;
		var_hash = var_hash->next;
	}

	if (!var_hash) {
		var_hash = emalloc(sizeof(var_entries));
		var_hash->used_slots = 0;
		var_hash->next = 0;

		if (!var_hashx->first_dtor)
			var_hashx->first_dtor = var_hash;
		else
			prev->next = var_hash;
	}

	(*rval)->refcount++;
	var_hash->data[var_hash->used_slots++] = *rval;
}

PHPAPI void var_replace(php_unserialize_data_t *var_hashx, zval *ozval, zval **nzval)
{
	long i;
	var_entries *var_hash = var_hashx->first;
	
	while (var_hash) {
		for (i = 0; i < var_hash->used_slots; i++) {
			if (var_hash->data[i] == ozval) {
				var_hash->data[i] = *nzval;
				/* do not break here */
			}
		}
		var_hash = var_hash->next;
	}
}

static int var_access(php_unserialize_data_t *var_hashx, long id, zval ***store)
{
	var_entries *var_hash = var_hashx->first;
	
	while (id >= VAR_ENTRIES_MAX && var_hash && var_hash->used_slots == VAR_ENTRIES_MAX) {
		var_hash = var_hash->next;
		id -= VAR_ENTRIES_MAX;
	}

	if (!var_hash) return !SUCCESS;

	if (id < 0 || id >= var_hash->used_slots) return !SUCCESS;

	*store = &var_hash->data[id];

	return SUCCESS;
}

PHPAPI void var_destroy(php_unserialize_data_t *var_hashx)
{
	void *next;
	long i;
	var_entries *var_hash = var_hashx->first;
	
	while (var_hash) {
		next = var_hash->next;
		efree(var_hash);
		var_hash = next;
	}

	var_hash = var_hashx->first_dtor;
	
	while (var_hash) {
		for (i = 0; i < var_hash->used_slots; i++) {
			zval_ptr_dtor(&var_hash->data[i]);
		}
		next = var_hash->next;
		efree(var_hash);
		var_hash = next;
	}
}

/* }}} */

static char *unserialize_str(const unsigned char **p, int len)
{
	int i, j;
	char *str = emalloc(len+1);

	for (i = 0; i < len; i++) {
		if (**p != '\\') {
			str[i] = (char)**p;
		} else {
			unsigned char ch = 0;

			for (j = 0; j < 2; j++) {
				(*p)++;
				if (**p >= '0' && **p <= '9') {
					ch = (ch << 4) + (**p -'0');
				} else if (**p >= 'a' && **p <= 'f') {
					ch = (ch << 4) + (**p -'a'+10);
				} else if (**p >= 'A' && **p <= 'F') {
					ch = (ch << 4) + (**p -'A'+10);
				} else {
					efree(str);
					return NULL;
				}
			}
			str[i] = (char)ch;
		}
		(*p)++;
	}
	str[i] = 0;
	return str;
}

#define YYFILL(n) do { } while (0)
#define YYCTYPE unsigned char
#define YYCURSOR cursor
#define YYLIMIT limit
#define YYMARKER marker


#line 187 "ext/standard/var_unserializer.re"




static inline long parse_iv2(const unsigned char *p, const unsigned char **q)
{
	char cursor;
	long result = 0;
	int neg = 0;

	switch (*p) {
		case '-':
			neg++;
			/* fall-through */
		case '+':
			p++;
	}
	
	while (1) {
		cursor = (char)*p;
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

static inline long parse_iv(const unsigned char *p)
{
	return parse_iv2(p, NULL);
}

/* no need to check for length - re2c already did */
static inline size_t parse_uiv(const unsigned char *p)
{
	unsigned char cursor;
	size_t result = 0;

	if (*p == '+') {
		p++;
	}
	
	while (1) {
		cursor = *p;
		if (cursor >= '0' && cursor <= '9') {
			result = result * 10 + (size_t)(cursor - (unsigned char)'0');
		} else {
			break;
		}
		p++;
	}
	return result;
}

#define UNSERIALIZE_PARAMETER zval **rval, const unsigned char **p, const unsigned char *max, php_unserialize_data_t *var_hash TSRMLS_DC
#define UNSERIALIZE_PASSTHRU rval, p, max, var_hash TSRMLS_CC

static inline int process_nested_data(UNSERIALIZE_PARAMETER, HashTable *ht, long elements)
{
	while (elements-- > 0) {
		zval *key, *data, **old_data;

		ALLOC_INIT_ZVAL(key);

		if (!php_var_unserialize(&key, p, max, NULL TSRMLS_CC)) {
			zval_dtor(key);
			FREE_ZVAL(key);
			return 0;
		}

		if (Z_TYPE_P(key) != IS_LONG && Z_TYPE_P(key) != IS_STRING) {
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
				if (zend_hash_index_find(ht, Z_LVAL_P(key), (void **)&old_data)==SUCCESS) {
					var_push_dtor(var_hash, old_data);
				}
				zend_hash_index_update(ht, Z_LVAL_P(key), &data, sizeof(data), NULL);
				break;
			case IS_STRING:
				if (zend_hash_find(ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, (void **)&old_data)==SUCCESS) {
					var_push_dtor(var_hash, old_data);
				}
				zend_hash_update(ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, &data, sizeof(data), NULL);
				break;
		}
		
		zval_dtor(key);
		FREE_ZVAL(key);

		if (elements && *(*p-1) != ';' &&  *(*p-1) != '}') {
			(*p)--;
			return 0;
		}
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

static inline int object_custom(UNSERIALIZE_PARAMETER, zend_class_entry *ce)
{
	long datalen;

	if(ce->unserialize == NULL) {
		zend_error(E_WARNING, "Class %s has no unserializer", ce->name);
		return 0;
	}

	datalen = parse_iv2((*p) + 2, p);

	(*p) += 2;

	if(datalen < 0 || (*p) + datalen >= max) {
		zend_error(E_WARNING, "Insufficient data for unserializing - %ld required, %d present", datalen, max - (*p));
		return 0;
	}

	if(ce->unserialize(rval, ce, (const unsigned char*)*p, datalen, (zend_unserialize_data *)var_hash TSRMLS_CC) != SUCCESS) {
		return 0;
	}

	(*p) += datalen;

	return finish_nested_data(UNSERIALIZE_PASSTHRU);
}

static inline long object_common1(UNSERIALIZE_PARAMETER, zend_class_entry *ce)
{
	long elements;
	
	elements = parse_iv2((*p) + 2, p);

	(*p) += 2;
	
	object_init_ex(*rval, ce);
	return elements;
}

static inline int object_common2(UNSERIALIZE_PARAMETER, long elements)
{
	zval *retval_ptr = NULL;
	zval fname;

	if (!process_nested_data(UNSERIALIZE_PASSTHRU, Z_OBJPROP_PP(rval), elements)) {
		return 0;
	}

	if (Z_OBJCE_PP(rval) != PHP_IC_ENTRY &&
	    zend_hash_exists(&Z_OBJCE_PP(rval)->function_table, "__wakeup", sizeof("__wakeup"))) {
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

	
	
{
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

#line 426 "ext/standard/var_unserializer.c"
{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	goto yy0;
	++YYCURSOR;
yy0:
	if((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
	switch(yych){
	case 'C':	case 'O':	goto yy13;
	case 'N':	goto yy5;
	case 'R':	goto yy2;
	case 'S':	goto yy10;
	case 'a':	goto yy11;
	case 'b':	goto yy6;
	case 'd':	goto yy8;
	case 'i':	goto yy7;
	case 'o':	goto yy12;
	case 'r':	goto yy4;
	case 's':	goto yy9;
	case '}':	goto yy14;
	default:	goto yy16;
	}
yy2:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy95;
	goto yy3;
yy3:
#line 687 "ext/standard/var_unserializer.re"
{ return 0; }
#line 457 "ext/standard/var_unserializer.c"
yy4:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy89;
	goto yy3;
yy5:	yych = *++YYCURSOR;
	if(yych == ';')	goto yy87;
	goto yy3;
yy6:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy83;
	goto yy3;
yy7:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy77;
	goto yy3;
yy8:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy53;
	goto yy3;
yy9:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy46;
	goto yy3;
yy10:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy39;
	goto yy3;
yy11:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy32;
	goto yy3;
yy12:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy25;
	goto yy3;
yy13:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy17;
	goto yy3;
yy14:	++YYCURSOR;
	goto yy15;
yy15:
#line 681 "ext/standard/var_unserializer.re"
{
	/* this is the case where we have less data than planned */
	php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unexpected end of serialized data");
	return 0; /* not sure if it should be 0 or 1 here? */
}
#line 506 "ext/standard/var_unserializer.c"
yy16:	yych = *++YYCURSOR;
	goto yy3;
yy17:	yych = *++YYCURSOR;
	if(yybm[0+yych] & 128) {
		goto yy20;
	}
	if(yych == '+')	goto yy19;
	goto yy18;
yy18:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy3;
	}
yy19:	yych = *++YYCURSOR;
	if(yybm[0+yych] & 128) {
		goto yy20;
	}
	goto yy18;
yy20:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	goto yy21;
yy21:	if(yybm[0+yych] & 128) {
		goto yy20;
	}
	if(yych != ':')	goto yy18;
	goto yy22;
yy22:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy18;
	goto yy23;
yy23:	++YYCURSOR;
	goto yy24;
yy24:
#line 569 "ext/standard/var_unserializer.re"
{
	size_t len, len2, len3, maxlen;
	long elements;
	char *class_name;
	zend_class_entry *ce;
	zend_class_entry **pce;
	int incomplete_class = 0;

	int custom_object = 0;

	zval *user_func;
	zval *retval_ptr;
	zval **args[1];
	zval *arg_func_name;

	if(*start == 'C') {
		custom_object = 1;
	}
	
	INIT_PZVAL(*rval);
	len2 = len = parse_uiv(start + 2);
	maxlen = max - YYCURSOR;
	if (maxlen < len || len == 0) {
		*p = start + 2;
		return 0;
	}

	class_name = (char*)YYCURSOR;

	YYCURSOR += len;

	if (*(YYCURSOR) != '"') {
		*p = YYCURSOR;
		return 0;
	}
	if (*(YYCURSOR+1) != ':') {
		*p = YYCURSOR+1;
		return 0;
	}

	len3 = strspn(class_name, "0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\177\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377");
	if (len3 != len)
	{
		*p = YYCURSOR + len3 - len;
		return 0;
	}

	class_name = estrndup(class_name, len);

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

	if(custom_object) {
		efree(class_name);
		return object_custom(UNSERIALIZE_PASSTHRU, ce);
	}
	
	elements = object_common1(UNSERIALIZE_PASSTHRU, ce);

	if (incomplete_class) {
		php_store_class_name(*rval, class_name, len2);
	}
	efree(class_name);

	return object_common2(UNSERIALIZE_PASSTHRU, elements);
}
#line 651 "ext/standard/var_unserializer.c"
yy25:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy18;
		goto yy26;
	} else {
		if(yych <= '-')	goto yy26;
		if(yych <= '/')	goto yy18;
		if(yych <= '9')	goto yy27;
		goto yy18;
	}
yy26:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy27;
yy27:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	goto yy28;
yy28:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy27;
	if(yych >= ';')	goto yy18;
	goto yy29;
yy29:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy18;
	goto yy30;
yy30:	++YYCURSOR;
	goto yy31;
yy31:
#line 561 "ext/standard/var_unserializer.re"
{

	INIT_PZVAL(*rval);
	
	return object_common2(UNSERIALIZE_PASSTHRU,
			object_common1(UNSERIALIZE_PASSTHRU, ZEND_STANDARD_CLASS_DEF_PTR));
}
#line 688 "ext/standard/var_unserializer.c"
yy32:	yych = *++YYCURSOR;
	if(yych == '+')	goto yy33;
	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy34;
	goto yy18;
yy33:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy34;
yy34:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	goto yy35;
yy35:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy34;
	if(yych >= ';')	goto yy18;
	goto yy36;
yy36:	yych = *++YYCURSOR;
	if(yych != '{')	goto yy18;
	goto yy37;
yy37:	++YYCURSOR;
	goto yy38;
yy38:
#line 539 "ext/standard/var_unserializer.re"
{
	long elements = parse_iv(start + 2);
	/* use iv() not uiv() in order to check data range */
	*p = YYCURSOR;

	if (elements < 0) {
		return 0;
	}

	INIT_PZVAL(*rval);
	Z_TYPE_PP(rval) = IS_ARRAY;
	ALLOC_HASHTABLE(Z_ARRVAL_PP(rval));

	zend_hash_init(Z_ARRVAL_PP(rval), elements + 1, NULL, ZVAL_PTR_DTOR, 0);

	if (!process_nested_data(UNSERIALIZE_PASSTHRU, Z_ARRVAL_PP(rval), elements)) {
		return 0;
	}

	return finish_nested_data(UNSERIALIZE_PASSTHRU);
}
#line 734 "ext/standard/var_unserializer.c"
yy39:	yych = *++YYCURSOR;
	if(yych == '+')	goto yy40;
	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy41;
	goto yy18;
yy40:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy41;
yy41:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	goto yy42;
yy42:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy41;
	if(yych >= ';')	goto yy18;
	goto yy43;
yy43:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy18;
	goto yy44;
yy44:	++YYCURSOR;
	goto yy45;
yy45:
#line 510 "ext/standard/var_unserializer.re"
{
	size_t len, maxlen;
	char *str;

	len = parse_uiv(start + 2);
	maxlen = max - YYCURSOR;
	if (maxlen < len) {
		*p = start + 2;
		return 0;
	}

	if ((str = unserialize_str(&YYCURSOR, len)) == NULL) {
		return 0;
	}

	if (*(YYCURSOR) != '"') {
		efree(str);
		*p = YYCURSOR;
		return 0;
	}

	YYCURSOR += 2;
	*p = YYCURSOR;

	INIT_PZVAL(*rval);
	ZVAL_STRINGL(*rval, str, len, 0);
	return 1;
}
#line 787 "ext/standard/var_unserializer.c"
yy46:	yych = *++YYCURSOR;
	if(yych == '+')	goto yy47;
	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy48;
	goto yy18;
yy47:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy48;
yy48:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	goto yy49;
yy49:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy48;
	if(yych >= ';')	goto yy18;
	goto yy50;
yy50:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy18;
	goto yy51;
yy51:	++YYCURSOR;
	goto yy52;
yy52:
#line 482 "ext/standard/var_unserializer.re"
{
	size_t len, maxlen;
	char *str;

	len = parse_uiv(start + 2);
	maxlen = max - YYCURSOR;
	if (maxlen < len) {
		*p = start + 2;
		return 0;
	}

	str = (char*)YYCURSOR;

	YYCURSOR += len;

	if (*(YYCURSOR) != '"') {
		*p = YYCURSOR;
		return 0;
	}

	YYCURSOR += 2;
	*p = YYCURSOR;

	INIT_PZVAL(*rval);
	ZVAL_STRINGL(*rval, str, len, 1);
	return 1;
}
#line 839 "ext/standard/var_unserializer.c"
yy53:	yych = *++YYCURSOR;
	if(yych <= '/'){
		if(yych <= ','){
			if(yych == '+')	goto yy57;
			goto yy18;
		} else {
			if(yych <= '-')	goto yy55;
			if(yych <= '.')	goto yy60;
			goto yy18;
		}
	} else {
		if(yych <= 'I'){
			if(yych <= '9')	goto yy58;
			if(yych <= 'H')	goto yy18;
			goto yy56;
		} else {
			if(yych != 'N')	goto yy18;
			goto yy54;
		}
	}
yy54:	yych = *++YYCURSOR;
	if(yych == 'A')	goto yy76;
	goto yy18;
yy55:	yych = *++YYCURSOR;
	if(yych <= '/'){
		if(yych == '.')	goto yy60;
		goto yy18;
	} else {
		if(yych <= '9')	goto yy58;
		if(yych != 'I')	goto yy18;
		goto yy56;
	}
yy56:	yych = *++YYCURSOR;
	if(yych == 'N')	goto yy72;
	goto yy18;
yy57:	yych = *++YYCURSOR;
	if(yych == '.')	goto yy60;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy58;
yy58:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	goto yy59;
yy59:	if(yych <= ':'){
		if(yych <= '.'){
			if(yych <= '-')	goto yy18;
			goto yy70;
		} else {
			if(yych <= '/')	goto yy18;
			if(yych <= '9')	goto yy58;
			goto yy18;
		}
	} else {
		if(yych <= 'E'){
			if(yych <= ';')	goto yy63;
			if(yych <= 'D')	goto yy18;
			goto yy65;
		} else {
			if(yych == 'e')	goto yy65;
			goto yy18;
		}
	}
yy60:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy61;
yy61:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	goto yy62;
yy62:	if(yych <= ';'){
		if(yych <= '/')	goto yy18;
		if(yych <= '9')	goto yy61;
		if(yych <= ':')	goto yy18;
		goto yy63;
	} else {
		if(yych <= 'E'){
			if(yych <= 'D')	goto yy18;
			goto yy65;
		} else {
			if(yych == 'e')	goto yy65;
			goto yy18;
		}
	}
yy63:	++YYCURSOR;
	goto yy64;
yy64:
#line 475 "ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_DOUBLE(*rval, zend_strtod((const char *)start + 2, NULL));
	return 1;
}
#line 935 "ext/standard/var_unserializer.c"
yy65:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy18;
		goto yy66;
	} else {
		if(yych <= '-')	goto yy66;
		if(yych <= '/')	goto yy18;
		if(yych <= '9')	goto yy67;
		goto yy18;
	}
yy66:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych == '+')	goto yy69;
		goto yy18;
	} else {
		if(yych <= '-')	goto yy69;
		if(yych <= '/')	goto yy18;
		if(yych >= ':')	goto yy18;
		goto yy67;
	}
yy67:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy68;
yy68:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy67;
	if(yych == ';')	goto yy63;
	goto yy18;
yy69:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy67;
	goto yy18;
yy70:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	goto yy71;
yy71:	if(yych <= ';'){
		if(yych <= '/')	goto yy18;
		if(yych <= '9')	goto yy70;
		if(yych <= ':')	goto yy18;
		goto yy63;
	} else {
		if(yych <= 'E'){
			if(yych <= 'D')	goto yy18;
			goto yy65;
		} else {
			if(yych == 'e')	goto yy65;
			goto yy18;
		}
	}
yy72:	yych = *++YYCURSOR;
	if(yych != 'F')	goto yy18;
	goto yy73;
yy73:	yych = *++YYCURSOR;
	if(yych != ';')	goto yy18;
	goto yy74;
yy74:	++YYCURSOR;
	goto yy75;
yy75:
#line 460 "ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);

	if (!strncmp(start + 2, "NAN", 3)) {
		ZVAL_DOUBLE(*rval, php_get_nan());
	} else if (!strncmp(start + 2, "INF", 3)) {
		ZVAL_DOUBLE(*rval, php_get_inf());
	} else if (!strncmp(start + 2, "-INF", 4)) {
		ZVAL_DOUBLE(*rval, -php_get_inf());
	}

	return 1;
}
#line 1010 "ext/standard/var_unserializer.c"
yy76:	yych = *++YYCURSOR;
	if(yych == 'N')	goto yy73;
	goto yy18;
yy77:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy18;
		goto yy78;
	} else {
		if(yych <= '-')	goto yy78;
		if(yych <= '/')	goto yy18;
		if(yych <= '9')	goto yy79;
		goto yy18;
	}
yy78:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy79;
yy79:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy80;
yy80:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy79;
	if(yych != ';')	goto yy18;
	goto yy81;
yy81:	++YYCURSOR;
	goto yy82;
yy82:
#line 453 "ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_LONG(*rval, parse_iv(start + 2));
	return 1;
}
#line 1046 "ext/standard/var_unserializer.c"
yy83:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= '2')	goto yy18;
	goto yy84;
yy84:	yych = *++YYCURSOR;
	if(yych != ';')	goto yy18;
	goto yy85;
yy85:	++YYCURSOR;
	goto yy86;
yy86:
#line 446 "ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_BOOL(*rval, parse_iv(start + 2));
	return 1;
}
#line 1064 "ext/standard/var_unserializer.c"
yy87:	++YYCURSOR;
	goto yy88;
yy88:
#line 439 "ext/standard/var_unserializer.re"
{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_NULL(*rval);
	return 1;
}
#line 1075 "ext/standard/var_unserializer.c"
yy89:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy18;
		goto yy90;
	} else {
		if(yych <= '-')	goto yy90;
		if(yych <= '/')	goto yy18;
		if(yych <= '9')	goto yy91;
		goto yy18;
	}
yy90:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy91;
yy91:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy92;
yy92:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy91;
	if(yych != ';')	goto yy18;
	goto yy93;
yy93:	++YYCURSOR;
	goto yy94;
yy94:
#line 416 "ext/standard/var_unserializer.re"
{
	long id;

 	*p = YYCURSOR;
	if (!var_hash) return 0;

	id = parse_iv(start + 2) - 1;
	if (id == -1 || var_access(var_hash, id, &rval_ref) != SUCCESS) {
		return 0;
	}

	if (*rval == *rval_ref) return 0;

	if (*rval != NULL) {
		zval_ptr_dtor(rval);
	}
	*rval = *rval_ref;
	(*rval)->refcount++;
	(*rval)->is_ref = 0;
	
	return 1;
}
#line 1124 "ext/standard/var_unserializer.c"
yy95:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy18;
		goto yy96;
	} else {
		if(yych <= '-')	goto yy96;
		if(yych <= '/')	goto yy18;
		if(yych <= '9')	goto yy97;
		goto yy18;
	}
yy96:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy18;
	if(yych >= ':')	goto yy18;
	goto yy97;
yy97:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy98;
yy98:	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy97;
	if(yych != ';')	goto yy18;
	goto yy99;
yy99:	++YYCURSOR;
	goto yy100;
yy100:
#line 395 "ext/standard/var_unserializer.re"
{
	long id;

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
#line 1171 "ext/standard/var_unserializer.c"
}
}
#line 689 "ext/standard/var_unserializer.re"


	return 0;
}
