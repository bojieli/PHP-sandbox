/* Generated by re2c 0.5 on Fri Nov  9 14:39:34 2001 */
#line 1 "/home/sas/src/php4/ext/standard/var_unserializer.re"
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

void var_replace(php_unserialize_data_t *var_hashx, zval *ozval, zval **nzval)
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

void var_destroy(php_unserialize_data_t *var_hashx)
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


#line 97




static inline int parse_iv2(const char *p, const char **q)
{
	char cursor;
	int result = 0;

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

	zval_ptr_dtor(rval);
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

	INIT_PZVAL(&fname);
	ZVAL_STRINGL(&fname, "__wakeup", sizeof("__wakeup") - 1, 0);
	call_user_function_ex(CG(function_table), rval, &fname, &retval_ptr, 0, 0, 1, NULL TSRMLS_CC);

	if (retval_ptr)
		zval_ptr_dtor(&retval_ptr);

	return finish_nested_data(UNSERIALIZE_PASSTHRU);

}

PHPAPI int php_var_unserialize(UNSERIALIZE_PARAMETER)
{
	const unsigned char *cursor, *limit, *marker, *start;
	zval **rval_ref;

	cursor = *p;
	
	if (var_hash && cursor[0] != 'R') {
		var_push(var_hash, rval);
	}

	start = cursor;

	
	
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
	if((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
	if(yych <= 'c'){
		if(yych <= 'Q'){
			if(yych <= 'M')	goto yy13;
			if(yych <= 'N')	goto yy5;
			if(yych <= 'O')	goto yy12;
			goto yy13;
		} else {
			if(yych <= '`'){
				if(yych <= 'R')	goto yy3;
				goto yy13;
			} else {
				if(yych <= 'a')	goto yy10;
				if(yych <= 'b')	goto yy6;
				goto yy13;
			}
		}
	} else {
		if(yych <= 'n'){
			if(yych <= 'd')	goto yy8;
			if(yych == 'i')	goto yy7;
			goto yy13;
		} else {
			if(yych <= 'r'){
				if(yych <= 'o')	goto yy11;
				goto yy13;
			} else {
				if(yych <= 's')	goto yy9;
				if(yych <= '\277')	goto yy13;
			}
		}
	}
yy2:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy4;
	}
yy3:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy65;
yy4:
#line 356
	{ return 0; }
yy5:	yych = *++YYCURSOR;
	if(yych == ';')	goto yy63;
	goto yy4;
yy6:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy58;
	goto yy4;
yy7:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy53;
	goto yy4;
yy8:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy38;
	goto yy4;
yy9:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy32;
	goto yy4;
yy10:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy26;
	goto yy4;
yy11:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy20;
	goto yy4;
yy12:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == ':')	goto yy14;
	goto yy4;
yy13:	yych = *++YYCURSOR;
	goto yy4;
yy14:	yych = *++YYCURSOR;
	if(yybm[0+yych] & 128)	goto yy15;
	goto yy2;
yy15:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy16:	if(yybm[0+yych] & 128)	goto yy15;
	if(yych != ':')	goto yy2;
yy17:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy2;
yy18:	yych = *++YYCURSOR;
yy19:
#line 317
	{
	int len;
	int elements;
	int len2;
	char *class_name;
	zend_class_entry *ce;
	int incomplete_class = 0;
	
	INIT_PZVAL(*rval);
	len2 = len = parse_iv(start + 2);
	if (len == 0)
		return 0;

	class_name = estrndup(YYCURSOR, len);
	YYCURSOR += len;

	while (len-- > 0) {
		if (class_name[len] >= 'A' && class_name[len] <= 'Z') {
			class_name[len] = class_name[len] - 'A' + 'a';
		}
	}

	if (zend_hash_find(EG(class_table), class_name, len2 + 1, (void **) &ce) != SUCCESS) {
		incomplete_class = 1;
		ce = PHP_IC_ENTRY;
	} else
		efree(class_name);

	*p = YYCURSOR;
	elements = object_common1(UNSERIALIZE_PASSTHRU, ce);

	if (incomplete_class) {
		php_store_class_name(*rval, class_name, len2);
		efree(class_name);
	}

	return object_common2(UNSERIALIZE_PASSTHRU, elements);
}
yy20:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy21:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy22:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy21;
	if(yych >= ';')	goto yy2;
yy23:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy2;
yy24:	yych = *++YYCURSOR;
yy25:
#line 309
	{

	INIT_PZVAL(*rval);
	
	return object_common2(UNSERIALIZE_PASSTHRU,
			object_common1(UNSERIALIZE_PASSTHRU, &zend_standard_class_def));
}
yy26:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy27:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy28:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy27;
	if(yych >= ';')	goto yy2;
yy29:	yych = *++YYCURSOR;
	if(yych != '{')	goto yy2;
yy30:	yych = *++YYCURSOR;
yy31:
#line 291
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
yy32:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy33:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy34:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy33;
	if(yych >= ';')	goto yy2;
yy35:	yych = *++YYCURSOR;
	if(yych != '"')	goto yy2;
yy36:	yych = *++YYCURSOR;
yy37:
#line 271
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
yy38:	yych = *++YYCURSOR;
	if(yych == '.')	goto yy41;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy39:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy40:	if(yych <= '/'){
		if(yych == '.')	goto yy50;
		goto yy2;
	} else {
		if(yych <= '9')	goto yy39;
		if(yych == ';')	goto yy44;
		goto yy2;
	}
yy41:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy42:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy43:	if(yych <= ';'){
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy42;
		if(yych <= ':')	goto yy2;
	} else {
		if(yych <= 'E'){
			if(yych <= 'D')	goto yy2;
			goto yy46;
		} else {
			if(yych == 'e')	goto yy46;
			goto yy2;
		}
	}
yy44:	yych = *++YYCURSOR;
yy45:
#line 264
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_DOUBLE(*rval, atof(start + 2));
	return 1;
}
yy46:	yych = *++YYCURSOR;
	if(yych <= ','){
		if(yych != '+')	goto yy2;
	} else {
		if(yych <= '-')	goto yy47;
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy48;
		goto yy2;
	}
yy47:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy48:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy49:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy48;
	if(yych == ';')	goto yy44;
	goto yy2;
yy50:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy51:	++YYCURSOR;
	if((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
yy52:	if(yych <= ';'){
		if(yych <= '/')	goto yy2;
		if(yych <= '9')	goto yy51;
		if(yych <= ':')	goto yy2;
		goto yy44;
	} else {
		if(yych <= 'E'){
			if(yych <= 'D')	goto yy2;
			goto yy46;
		} else {
			if(yych == 'e')	goto yy46;
			goto yy2;
		}
	}
yy53:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy54:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy55:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy54;
	if(yych != ';')	goto yy2;
yy56:	yych = *++YYCURSOR;
yy57:
#line 257
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_LONG(*rval, parse_iv(start + 2));
	return 1;
}
yy58:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy59:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy60:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy59;
	if(yych != ';')	goto yy2;
yy61:	yych = *++YYCURSOR;
yy62:
#line 250
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_BOOL(*rval, parse_iv(start + 2));
	return 1;
}
yy63:	yych = *++YYCURSOR;
yy64:
#line 243
	{
	*p = YYCURSOR;
	INIT_PZVAL(*rval);
	ZVAL_NULL(*rval);
	return 1;
}
yy65:	yych = *++YYCURSOR;
	if(yych <= '/')	goto yy2;
	if(yych >= ':')	goto yy2;
yy66:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy67:	if(yych <= '/')	goto yy2;
	if(yych <= '9')	goto yy66;
	if(yych != ';')	goto yy2;
yy68:	yych = *++YYCURSOR;
yy69:
#line 224
	{
	int id;

 	*p = YYCURSOR;
	if (!var_hash) return 0;

	id = parse_iv(start + 2) - 1;
	if (id == -1 || var_access(var_hash, id, &rval_ref) != SUCCESS) {
		return 0;
	}

	zval_ptr_dtor(rval);
	*rval = *rval_ref;
	(*rval)->refcount++;
	(*rval)->is_ref = 1;
	
	return 1;
}
}
#line 358


	return 0;
}
