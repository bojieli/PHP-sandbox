/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999 The PHP Group                         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jani Lehtim�ki <jkl@njet.net>                               |
   |   
   +----------------------------------------------------------------------+
 */

/* $Id$ */


/* {{{ includes 
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "php.h"
#include "php_string.h"
#include "php_var.h"

/* }}} */
/* {{{ php_var_dump */

void php_var_dump(pval **struc, int level)
{
	ulong index;
	char *key;
	int i, c = 0;
	pval **data;
	char buf[512];
	HashTable *myht;

	switch ((*struc)->type) {
		case IS_BOOL:
			i = sprintf(buf, "%*cbool(%s)\n", level, ' ', ((*struc)->value.lval?"true":"false"));
			PHPWRITE(&buf[1], i - 1);
			break;

		case IS_LONG:
			i = sprintf(buf, "%*cint(%ld)\n", level, ' ', (*struc)->value.lval);
			PHPWRITE(&buf[1], i - 1);
			break;

		case IS_DOUBLE: {
				ELS_FETCH();
				i = sprintf(buf, "%*cfloat(%.*G)\n", level, ' ', (int) EG(precision), (*struc)->value.dval);
				PHPWRITE(&buf[1], i - 1);
			}
			break;

		case IS_STRING:
			i = sprintf(buf, "%*cstring(%d) \"", level, ' ', (*struc)->value.str.len);
			PHPWRITE(&buf[1], i - 1);
			PHPWRITE((*struc)->value.str.val, (*struc)->value.str.len);
			strcpy(buf, "\"\n");
			PHPWRITE(buf, strlen(buf));
			break;

		case IS_ARRAY:
			myht = HASH_OF(*struc);
			i = sprintf(buf, "%*carray(%d) {\n", level, ' ', zend_hash_num_elements(myht));
			PHPWRITE(&buf[1], i - 1);
			goto head_done;

		case IS_OBJECT:
			myht = HASH_OF(*struc);
			i = sprintf(buf, "%*cobject of %s (%d) {\n", level, ' ', (*struc)->value.obj.ce->name, zend_hash_num_elements(myht));
			PHPWRITE(&buf[1], i - 1);
		  head_done:

			zend_hash_internal_pointer_reset(myht);
			for (;; zend_hash_move_forward(myht)) {
				if ((i = zend_hash_get_current_key(myht, &key, &index)) == HASH_KEY_NON_EXISTANT)
					break;
				if (c > 0) {
					strcpy(buf, "\n");
					PHPWRITE(buf, strlen(buf));
				}
				c++;
				if (zend_hash_get_current_data(myht, (void **) (&data)) != SUCCESS || !data || (data == struc)) {
					if (i == HASH_KEY_IS_STRING)
						efree(key);
					continue;
				}
				switch (i) {
					case HASH_KEY_IS_LONG:{
							pval *d = emalloc(sizeof(pval));

							d->type = IS_LONG;
							d->value.lval = index;
							php_var_dump(&d, level + 2);
							efree(d);
						}
						break;

					case HASH_KEY_IS_STRING:{
							pval *d = emalloc(sizeof(pval));

							d->type = IS_STRING;
							d->value.str.val = key;
							d->value.str.len = strlen(key);
							php_var_dump(&d, level + 2);
							efree(key);
							efree(d);
						}
						break;
				}
				php_var_dump(data, level + 2);
			}
			i = sprintf(buf, "%*c}\n", level, ' ');
			PHPWRITE(&buf[1], i - 1);
			break;
			
		case IS_RESOURCE: {
			int type;
			zend_list_find((*struc)->value.lval, &type);
			i = sprintf(buf, "%*cresource(%ld) of type %d\n", level, ' ',
						(*struc)->value.lval, type);
			PHPWRITE(&buf[1], i - 1);
			break;
		}

		default:
			i = sprintf(buf, "%*ci:0\n", level, ' ');
			PHPWRITE(&buf[1], i - 1);
	}
}

/* }}} */


/* {{{ php_var_dump */
PHP_FUNCTION(var_dump)
{
	zval ***args;
	int argc;
	int	i;
	
	argc = ARG_COUNT(ht);
	
	args = (zval ***)emalloc(argc * sizeof(zval **));
	if (ARG_COUNT(ht) == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}
	
	for (i=0; i<argc; i++)
		php_var_dump(args[i], 1);
	
	efree(args);
}
/* }}} */


/* {{{ php_var_dump */


#define STR_CAT(P,S,I) {\
	pval *__p = (P);\
	ulong __i = __p->value.str.len;\
	__p->value.str.len += (I);\
	if (__p->value.str.val) {\
		__p->value.str.val = (char *)erealloc(__p->value.str.val, __p->value.str.len + 1);\
	} else {\
		__p->value.str.val = emalloc(__p->value.str.len + 1);\
		*__p->value.str.val = 0;\
	}\
	strcat(__p->value.str.val + __i, (S));\
}

/* }}} */
/* {{{ php_var_serialize */

void php_var_serialize(pval *buf, pval **struc)
{
	char s[256];
	ulong slen;
	int i;
	HashTable *myht;

	switch ((*struc)->type) {
		case IS_BOOL:
			slen = sprintf(s, "b:%ld;", (*struc)->value.lval);
			STR_CAT(buf, s, slen);
			return;

		case IS_LONG:
			slen = sprintf(s, "i:%ld;", (*struc)->value.lval);
			STR_CAT(buf, s, slen);
			return;

		case IS_DOUBLE: {
				ELS_FETCH();
				slen = sprintf(s, "d:%.*G;",(int) EG(precision), (*struc)->value.dval);
				STR_CAT(buf, s, slen);
			}
			return;

		case IS_STRING:{
				char *p;
				
				i = buf->value.str.len;
				slen = sprintf(s, "s:%d:\"", (*struc)->value.str.len);
				STR_CAT(buf, s, slen + (*struc)->value.str.len + 2);
				p = buf->value.str.val + i + slen;
				if ((*struc)->value.str.len > 0) {
					memcpy(p, (*struc)->value.str.val, (*struc)->value.str.len);
					p += (*struc)->value.str.len;
				}
				*p++ = '\"';
				*p++ = ';';
				*p = 0;
			}
			return;

		case IS_ARRAY:
		case IS_OBJECT:
			myht = HASH_OF(*struc);
			i = zend_hash_num_elements(myht);
			if ((*struc)->type == IS_ARRAY) {
				slen = sprintf(s, "a:%d:{", i);
			} else {
				slen = sprintf(s, "O:%d:\"%s\":%d:{",(*struc)->value.obj.ce->name_length,(*struc)->value.obj.ce->name, i);
			}
			STR_CAT(buf, s, slen);
			if (i > 0) {
				char *key;
				pval **data,*d;
				ulong index;
				
				zend_hash_internal_pointer_reset(myht);
				for (;; zend_hash_move_forward(myht)) {
					if ((i = zend_hash_get_current_key(myht, &key, &index)) == HASH_KEY_NON_EXISTANT) {
						break;
					}
					if (zend_hash_get_current_data(myht, (void **) (&data)) != SUCCESS || !data || ((*data) == (*struc))) {
						if (i == HASH_KEY_IS_STRING)
							efree(key);
						continue;
					}

					switch (i) {
						case HASH_KEY_IS_LONG:
							d = emalloc(sizeof(pval));	
							d->type = IS_LONG;
							d->value.lval = index;
							php_var_serialize(buf, &d);
							efree(d);
							break;
						case HASH_KEY_IS_STRING:
							d = emalloc(sizeof(pval));	
							d->type = IS_STRING;
							d->value.str.val = key;
							d->value.str.len = strlen(key);
							php_var_serialize(buf, &d);
							efree(key);
							efree(d);
							break;
					}
					php_var_serialize(buf, data);
				}
			}
			STR_CAT(buf, "}", 1);
			return;

		default:
			STR_CAT(buf, "i:0;", 4);
			return;
	} 
}

/* }}} */
/* {{{ php_var_dump */

int php_var_unserialize(pval **rval, const char **p, const char *max)
{
	const char *q;
	char *str;
	int i;
	char cur;
	HashTable *myht;
	ELS_FETCH();

	switch (cur = **p) {
		case 'b': /* bool */
		case 'i':
			if (*((*p) + 1) != ':') {
				return 0;
			}
			q = *p;
			while (**p && **p != ';') {
				(*p)++;
			}
			if (**p != ';') {
				return 0;
			}
			(*p)++;
			if (cur == 'b') {
				(*rval)->type = IS_BOOL;
			} else {
				(*rval)->type = IS_LONG;
			}
			INIT_PZVAL(*rval);
			(*rval)->value.lval = atol(q + 2);
			return 1;

		case 'd':
			if (*((*p) + 1) != ':') {
				return 0;
			}
			q = *p;
			while (**p && **p != ';') {
				(*p)++;
			}
			if (**p != ';') {
				return 0;
			}
			(*p)++;
			(*rval)->type = IS_DOUBLE;
			INIT_PZVAL(*rval);
			(*rval)->value.dval = atof(q + 2);
			return 1;

		case 's':
			if (*((*p) + 1) != ':') {
				return 0;
			}
			(*p) += 2;
			q = *p;
			while (**p && **p != ':') {
				(*p)++;
			}
			if (**p != ':') {
				return 0;
			}
			i = atoi(q);
			if (i < 0 || (*p + 3 + i) > max || *((*p) + 1) != '\"' ||
				*((*p) + 2 + i) != '\"' || *((*p) + 3 + i) != ';') {
				return 0;
			}
			(*p) += 2;

			if (i == 0) {
			  	str = empty_string;
			} else  {
			  	str = estrndup(*p,i);
			}
			(*p) += i + 2;
			(*rval)->type = IS_STRING;
			(*rval)->value.str.val = str;
			(*rval)->value.str.len = i;
			INIT_PZVAL(*rval);
			return 1;

		case 'a':
		case 'o':
		case 'O':
			INIT_PZVAL(*rval);

			if (cur == 'a') {
				(*rval)->type = IS_ARRAY;
				(*rval)->value.ht = (HashTable *) emalloc(sizeof(HashTable));
				myht = (*rval)->value.ht;
			} else {
				zend_class_entry *ce;

				if (cur == 'O') { /* php4 serialized - we get the class-name */
					char *class_name;

					if (*((*p) + 1) != ':') {
						return 0;
					}
					(*p) += 2;
					q = *p;
					while (**p && **p != ':') {
						(*p)++;
					}
					if (**p != ':') {
						return 0;
					}
					i = atoi(q);
					if (i < 0 || (*p + 3 + i) > max || *((*p) + 1) != '\"' ||
						*((*p) + 2 + i) != '\"' || *((*p) + 3 + i) != ':') {
						return 0;
					}
					(*p) += 2;
					class_name = emalloc(i + 1);
					if (i > 0) {
						memcpy(class_name, *p, i);
					}
					class_name[i] = 0;
					(*p) += i;
					
					if (zend_hash_find(EG(class_table), class_name, i+1, (void **) &ce)==FAILURE) {
						php_error(E_NOTICE, "Unserializing non-existant class: %s! No methods will be available!", class_name);
						ce = &zend_standard_class_def;
					}

					efree(class_name);
				} else { /* old php 3.0 data 'o' */
					ce = &zend_standard_class_def;
				}

				object_init_ex(*rval,ce);
				myht = (*rval)->value.obj.properties;
			}

			(*p) += 2;
			i = atoi(*p);

			if (cur == 'a') { /* object_init_ex will init the HashTable for objects! */
				zend_hash_init(myht, i + 1, NULL, ZVAL_PTR_DTOR, 0);
			}

			while (**p && **p != ':') {
				(*p)++;
			}
			if (**p != ':' || *((*p) + 1) != '{') {
				return 0;
			}
			for ((*p) += 2; **p && **p != '}' && i > 0; i--) {
				pval *key = emalloc(sizeof(pval));
				pval *data = emalloc(sizeof(pval));
				
				if (!php_var_unserialize(&key, p, max)) {
				  zval_dtor(key);
				  efree(key);
					efree(data);
					return 0;
				}
				if (!php_var_unserialize(&data, p, max)) {
					zval_dtor(key);
				    efree(key);
					zval_dtor(data);
					efree(data);
					return 0;
				}
				switch (key->type) {
					case IS_LONG:
						zend_hash_index_update(myht, key->value.lval, &data, sizeof(data), NULL);
						break;
					case IS_STRING:
						zend_hash_update(myht, key->value.str.val, key->value.str.len + 1, &data, sizeof(data), NULL);
						break;
				}
				pval_destructor(key);
				efree(key);
			}
			return *((*p)++) == '}';

		default:
			return 0;
	}
}

/* }}} */
/* {{{ proto string serialize(mixed variable)
   Returns a string representation of variable (which can later be unserialized) */
PHP_FUNCTION(serialize)
{
	pval **struc;

	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &struc) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	return_value->type = IS_STRING;
	return_value->value.str.val = NULL;
	return_value->value.str.len = 0;
	php_var_serialize(return_value, struc);
}

/* }}} */
/* {{{ proto mixed unserialize(string variable_representation)
   Takes a string representation of variable and recreates it */


PHP_FUNCTION(unserialize)
{
	pval **buf;
	
	if (ARG_COUNT(ht) != 1 || zend_get_parameters_ex(1, &buf) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if ((*buf)->type == IS_STRING) {
		const char *p = (*buf)->value.str.val;

		if ((*buf)->value.str.len == 0) {
			RETURN_FALSE;
		}

		if (!php_var_unserialize(&return_value, &p, p + (*buf)->value.str.len)) {
			zval_dtor(return_value);
			php_error(E_NOTICE, "unserialize() failed at offset %d of %d bytes",p-(*buf)->value.str.val,(*buf)->value.str.len);
			RETURN_FALSE;
		}
	} else {
		php_error(E_NOTICE, "argument passed to unserialize() is not an string");
		RETURN_FALSE;
	}
}

/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
