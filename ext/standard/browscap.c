/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Zeev Suraski <zeev@zend.com>                                 |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"
#include "php_regex.h"
#include "php_browscap.h"
#include "php_ini.h"

#include "zend_globals.h"

static HashTable browser_hash;
static zval *current_section;

#define DEFAULT_SECTION_NAME "Default Browser Capability Settings"

/* OBJECTS_FIXME: This whole extension needs going through. The use of objects looks pretty broken here */

static void browscap_entry_dtor(zval *pvalue)
{
	if (Z_TYPE_P(pvalue) == IS_ARRAY) {
		zend_hash_destroy(Z_ARRVAL_P(pvalue));
		free(Z_ARRVAL_P(pvalue));
	}
}

/* {{{ convert_browscap_pattern
 */
static void convert_browscap_pattern(zval *pattern)
{
	register int i, j;
	char *t;

	t = (char *) malloc(Z_STRLEN_P(pattern)*2 + 3);
	t[0] = '^';

	for (i=0, j=1; i<Z_STRLEN_P(pattern); i++, j++) {
		switch (Z_STRVAL_P(pattern)[i]) {
			case '?':
				t[j] = '.';
				break;
			case '*':
				t[j++] = '.';
				t[j] = '*';
				break;
			case '.':
				t[j++] = '\\';
				t[j] = '.';
				break;
			default:
				t[j] = Z_STRVAL_P(pattern)[i];
				break;
		}
	}

	if (j && (t[j-1] == '.')) {
		t[j++] = '*';
	}

	t[j++]='$';
	t[j]=0;
	Z_STRVAL_P(pattern) = t;
	Z_STRLEN_P(pattern) = j;
}
/* }}} */

/* {{{ php_browscap_parser_cb
 */
static void php_browscap_parser_cb(zval *arg1, zval *arg2, int callback_type, void *arg)
{
	if (!arg1) {
		return;
	}

	switch (callback_type) {
		case ZEND_INI_PARSER_ENTRY:
			if (current_section && arg2) {
				zval *new_property;
				char *new_key;

				new_property = (zval *) malloc(sizeof(zval));
				INIT_PZVAL(new_property);
				Z_STRVAL_P(new_property) = Z_STRLEN_P(arg2)?zend_strndup(Z_STRVAL_P(arg2), Z_STRLEN_P(arg2)):"";
				Z_STRLEN_P(new_property) = Z_STRLEN_P(arg2);
				Z_TYPE_P(new_property) = IS_STRING;

				new_key = zend_strndup(Z_STRVAL_P(arg1), Z_STRLEN_P(arg1));
				zend_str_tolower(new_key, Z_STRLEN_P(arg1));
				zend_hash_update(Z_ARRVAL_P(current_section), new_key, Z_STRLEN_P(arg1)+1, &new_property, sizeof(zval *), NULL);
				free(new_key);
			}
			break;
		case ZEND_INI_PARSER_SECTION: {
				zval *processed;
				zval *unprocessed;
				HashTable *section_properties;

				/*printf("'%s' (%d)\n",$1.value.str.val,$1.value.str.len+1);*/
				current_section = (zval *) malloc(sizeof(zval));
				INIT_PZVAL(current_section);
				processed = (zval *) malloc(sizeof(zval));
				INIT_PZVAL(processed);
				unprocessed = (zval *) malloc(sizeof(zval));
				INIT_PZVAL(unprocessed);

				section_properties = (HashTable *) malloc(sizeof(HashTable));
				zend_hash_init(section_properties, 0, NULL, (dtor_func_t) browscap_entry_dtor, 1);
				current_section->value.ht = section_properties;
				zend_hash_update(&browser_hash, Z_STRVAL_P(arg1), Z_STRLEN_P(arg1)+1, (void *) &current_section, sizeof(zval *), NULL);

				Z_STRVAL_P(processed) = Z_STRVAL_P(arg1);
				Z_STRLEN_P(processed) = Z_STRLEN_P(arg1);
				Z_TYPE_P(processed) = IS_STRING;
				Z_STRVAL_P(unprocessed) = Z_STRVAL_P(arg1);
				Z_STRLEN_P(unprocessed) = Z_STRLEN_P(arg1);
				Z_TYPE_P(unprocessed) = IS_STRING;
				Z_STRVAL_P(unprocessed) = zend_strndup(Z_STRVAL_P(unprocessed), Z_STRLEN_P(unprocessed));

				convert_browscap_pattern(processed);
				zend_hash_update(section_properties, "browser_name_regex", sizeof("browser_name_regex"), (void *) &processed, sizeof(zval *), NULL);
				zend_hash_update(section_properties, "browser_name_pattern", sizeof("browser_name_pattern"), (void *) &unprocessed, sizeof(zval *), NULL);
			}
			break;
	}
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(browscap)
{
	char *browscap = INI_STR("browscap");

	if (browscap) {
		zend_file_handle fh;
		memset(&fh, 0, sizeof(fh));

		if (zend_hash_init(&browser_hash, 0, NULL, (dtor_func_t) browscap_entry_dtor, 1)==FAILURE) {
			return FAILURE;
		}

		fh.handle.fp = VCWD_FOPEN(browscap, "r");
		fh.opened_path = NULL;
		fh.free_filename = 0;
		if (!fh.handle.fp) {
			php_error_docref(NULL TSRMLS_CC, E_CORE_WARNING, "Cannot open '%s' for reading", browscap);
			return FAILURE;
		}
		fh.filename = browscap;
		Z_TYPE(fh) = ZEND_HANDLE_FP;
		zend_parse_ini_file(&fh, 1, (zend_ini_parser_cb_t) php_browscap_parser_cb, &browser_hash);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(browscap)
{
	if (INI_STR("browscap")) {
		zend_hash_destroy(&browser_hash);
	}
	return SUCCESS;
}
/* }}} */

/* {{{ browser_reg_compare
 */
static int browser_reg_compare(zval **browser, int num_args, va_list args, zend_hash_key *key)
{
	zval **browser_name, **current;
	regex_t r;
	char *lookup_browser_name = va_arg(args, char *);
	zval **found_browser_entry = va_arg(args, zval **);

	if (zend_hash_find(Z_ARRVAL_PP(browser), "browser_name_regex", sizeof("browser_name_regex"), (void **) &browser_name) == FAILURE) {
		return 0;
	}

	if (*found_browser_entry) {
		/* We've already found it, so don't compare to the default browser,
		   because it will match anything. */
		if (!strcmp(Z_STRVAL_PP(browser_name), "^.*$")) {
			return 0;
		}
		/* If we've found a possible browser, check it's length. Longer user
		   agent strings are assumed to be more precise, so use them. */
		else if (zend_hash_find(Z_ARRVAL_PP(found_browser_entry), "browser_name_regex", sizeof("browser_name_regex"), (void**) &current) == FAILURE) {
			return 0;
		}
		else if (Z_STRLEN_PP(current) > Z_STRLEN_PP(browser_name)) {
			return 0;
		}
	}
	if (regcomp(&r, Z_STRVAL_PP(browser_name), REG_NOSUB)!=0) {
		return 0;
	}
	if (regexec(&r, lookup_browser_name, 0, NULL, 0)==0) {
		*found_browser_entry = *browser;
	}
	regfree(&r);
	return 0;
}
/* }}} */

/* {{{ proto mixed get_browser([string browser_name [, bool return_array]])
   Get information about the capabilities of a browser. If browser_name is omitted
   or null, HTTP_USER_AGENT is used. Returns an object by default; if return_array
   is true, returns an array. */
PHP_FUNCTION(get_browser)
{
	zval **agent_name = NULL, **agent, **retarr;
	zval *found_browser_entry, *tmp_copy;
	char *lookup_browser_name;
	zend_bool return_array = 0;

	if (!INI_STR("browscap")) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "browscap ini directive not set.");
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() > 2 || zend_get_parameters_ex(ZEND_NUM_ARGS(), &agent_name, &retarr) == FAILURE) {
		ZEND_WRONG_PARAM_COUNT();
	}
	
	if (agent_name == NULL || Z_TYPE_PP(agent_name) == IS_NULL) {
		zend_is_auto_global("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);
		if (!PG(http_globals)[TRACK_VARS_SERVER]
			|| zend_hash_find(PG(http_globals)[TRACK_VARS_SERVER]->value.ht, "HTTP_USER_AGENT", sizeof("HTTP_USER_AGENT"), (void **) &agent_name)==FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "HTTP_USER_AGENT variable is not set, cannot determine user agent name");
			RETURN_FALSE;
		}
	}

	convert_to_string_ex(agent_name);

	if (ZEND_NUM_ARGS() == 2) {
		convert_to_boolean_ex(retarr);
		return_array = Z_LVAL_PP(retarr);
	}

	if (zend_hash_find(&browser_hash, Z_STRVAL_PP(agent_name), Z_STRLEN_PP(agent_name)+1, (void **) &agent)==FAILURE) {
		lookup_browser_name = Z_STRVAL_PP(agent_name);
		found_browser_entry = NULL;
		zend_hash_apply_with_arguments(&browser_hash, (apply_func_args_t) browser_reg_compare, 2, lookup_browser_name, &found_browser_entry);

		if (found_browser_entry) {
			agent = &found_browser_entry;
		} else if (zend_hash_find(&browser_hash, DEFAULT_SECTION_NAME, sizeof(DEFAULT_SECTION_NAME), (void **) &agent)==FAILURE) {
			RETURN_FALSE;
		}
	}

	if (return_array) {
		array_init(return_value);
		zend_hash_copy(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
	}
	else {
		object_init(return_value);
		zend_hash_copy(Z_OBJPROP_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
	}

	while (zend_hash_find(Z_ARRVAL_PP(agent), "parent", sizeof("parent"), (void **) &agent_name)==SUCCESS) {
		if (zend_hash_find(&browser_hash, Z_STRVAL_PP(agent_name), Z_STRLEN_PP(agent_name)+1, (void **)&agent)==FAILURE) {
			break;
		}

		if (return_array) {
			zend_hash_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *), 0);
		}
		else {
			zend_hash_merge(Z_OBJPROP_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *), 0);
		}
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
