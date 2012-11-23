/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Bojie Li <bojieli@gmail.com>                                 |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include <sys/time.h>
#include "php_blog.h"
#include "ext/sandbox/php_sandbox.h"
#include "ext/standard/sha1.h"

/* This extension is highly depending on php_sandbox.h and uses
	mysql functions declared there. */

ZEND_DECLARE_MODULE_GLOBALS(blog)

/* True global resources - no need for thread safety here */
static int le_blog;

/* {{{ blog_functions[]
 *
 * Every user visible function must have an entry in blog_functions[].
 */
const zend_function_entry blog_functions[] = {
	PHP_FE(get_appinfo, NULL)
	PHP_FE(create_app, NULL)
	PHP_FE(app_activate, NULL)
	PHP_FE(app_deactivate, NULL)
	PHP_FE(app_count, NULL)
	PHP_FE(get_appid_by_field, NULL)
	PHP_FE(check_email_count, NULL)
	PHP_FE(random_string, NULL)
	PHP_FE_END	/* Must be the last line in blog_functions[] */
};
/* }}} */

/* {{{ blog_module_entry
 */
zend_module_entry blog_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"blog",
	blog_functions,
	PHP_MINIT(blog),
	PHP_MSHUTDOWN(blog),
	PHP_RINIT(blog),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(blog),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(blog),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

static const zend_module_dep mysql_deps[] = {
    ZEND_MOD_REQUIRED("sandbox")
	ZEND_MOD_END
};

#ifdef COMPILE_DL_BLOG
ZEND_GET_MODULE(blog)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("blog.userdb_prefix", "ub_", PHP_INI_SYSTEM, OnUpdateString, userdb_prefix, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.max_blogs_per_email", "5", PHP_INI_SYSTEM, OnUpdateLong, max_blogs_per_email, zend_blog_globals, blog_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_blog_init_globals
 */
static void php_blog_init_globals(zend_blog_globals *blog_globals)
{
	blog_globals->userdb_prefix = "";
	blog_globals->max_blogs_per_email = 0;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(blog)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(blog)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(blog)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(blog)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(blog)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "blog support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ proto int get_appinfo(int appid) */
PHP_FUNCTION(get_appinfo)
{
	ASSERT_PRIVILEGE
	GET_APPID_PARAM

	char **row = admindb_fetch_row("appinfo", "id", ltostr(appid));
	if (row == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "appid %d not found", appid);
		RETURN_NULL();
	}

	array_init(return_value);
	add_assoc_string(return_value, "id", row[0], 1);
	add_assoc_string(return_value, "appname", row[1], 1);
	add_assoc_string(return_value, "username", row[2], 1);
	add_assoc_string(return_value, "email", row[3], 1);
	add_assoc_string(return_value, "password", row[4], 1);
	add_assoc_string(return_value, "salt", row[5], 1);
	add_assoc_string(return_value, "token", row[6], 1);
	add_assoc_string(return_value, "isactive", row[7], 1);
	add_assoc_string(return_value, "register_time", row[8], 1);
}
/* }}} */

/* {{{ proto int create_app(appname, username, email, password) 
	RETURN appid */
PHP_FUNCTION(create_app)
{
	ASSERT_PRIVILEGE

	char *appname = NULL, *username = NULL, *email = NULL, *password = NULL;
	int appname_len, username_len, email_len, password_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssss", &appname, &appname_len, &username, &username_len, &email, &email_len, &password, &password_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "bad parameters");
		RETURN_NULL();
	}

	if (0 < admindb_row_count("appinfo", "appname", appname TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "This appname has been taken");
		RETURN_NULL();
	}
	if (BLOG_G(max_blogs_per_email) <= admindb_row_count("appinfo", "email", email TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Each email can register at most %d blogs", BLOG_G(max_blogs_per_email));
		RETURN_NULL();
	}

#define SHA1_LENGTH 40
	char* salt = random_str_gen(SHA1_LENGTH);
	char* salted_pass = emalloc(SHA1_LENGTH+1);
	bzero(salted_pass, SHA1_LENGTH+1);
	make_sha1_digest(salted_pass, new_sprintf("%s\n%s", password, salt));

	char* fields[] = {"appname", "username", "email", "password", "salt", "token", "isactive", "register_time"};
	char* values[] = {appname, username, email, salted_pass, salt, random_str_gen(40), "0", ltostr(time(NULL))};
	if (FAILURE == admindb_insert_row("appinfo", sizeof(fields)/sizeof(fields[0]), fields, values TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to save appinfo");
		RETURN_NULL();
	}
	long appid = admindb_insert_id(TSRMLS_CC);

	char* dbname = new_sprintf("%s%d", BLOG_G(userdb_prefix), appid);
	char* dbpass = random_str_gen(SHA1_LENGTH);
	char* fields1[] = {"id", "hostname", "username", "password", "dbname"};
	char* values1[] = {ltostr(appid), "localhost", dbname, dbpass, dbname};
	if (FAILURE == admindb_insert_row("dbconf", sizeof(fields1)/sizeof(fields1[0]), fields1, values1 TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to save dbconf");
		RETURN_NULL();
	}

	if (FAILURE == create_database(dbname TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create database");
		RETURN_NULL();
	}
	if (FAILURE == grant_db_privilege(dbname, "localhost", dbname, dbpass TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to grant db privilege");
		RETURN_NULL();
	}
	if (FAILURE == php_connect_userdb(appid TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to connect to user database");
		RETURN_NULL();
	}
	ZVAL_LONG(return_value, appid);
}
/* }}} */

/* {{{ proto int app_activate(int appid) */
PHP_FUNCTION(app_activate)
{
	ASSERT_PRIVILEGE
	GET_APPID_PARAM
	RETURN_BOOL(admindb_update_row("appinfo", appid, "isactive", "1"));
}
/* }}} */

/* {{{ proto int app_deactivate(int appid) */
PHP_FUNCTION(app_deactivate)
{
	ASSERT_PRIVILEGE
	GET_APPID_PARAM
	RETURN_BOOL(admindb_update_row("appinfo", appid, "isactive", "0"));
}
/* }}} */

/* {{{ proto int app_count(string field, string value) */
PHP_FUNCTION(app_count)
{
	ASSERT_PRIVILEGE

	char *field = NULL, *value = NULL;
	int field_len, value_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ss", &field, &field_len, &value, &value_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "bad parameters");
		RETURN_NULL();
	}

	long count;
	if (field == NULL || value == NULL)
		count = admindb_num_rows();
	else
		count = admindb_row_count("appinfo", field, value);
	ZVAL_LONG(return_value, count);
}
/* }}} */

/* {{{ proto int get_appid_by_field(string field, string value) */
PHP_FUNCTION(get_appid_by_field)
{
	ASSERT_PRIVILEGE

	char *field = NULL, *value = NULL;
	int field_len, value_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &field, &field_len, &value, &value_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "bad parameters");
		RETURN_NULL();
	}

	char *appid = admindb_fetch_field("appinfo", "id", field, value);
	if (appid == NULL)
		RETURN_FALSE;

	ZVAL_LONG(return_value, atol(appid));
}
/* }}} */

/* {{{ proto bool check_email_count(string email) */
PHP_FUNCTION(check_email_count)
{
	ASSERT_PRIVILEGE

	char *email = NULL;
	int email_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &email, &email_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "bad parameters");
		RETURN_FALSE;
	}

	RETURN_BOOL(BLOG_G(max_blogs_per_email) > admindb_row_count("appinfo", "email", email));
}
/* }}} */

/* {{{ proto string random_string(long length) */
PHP_FUNCTION(random_string)
{
	long length;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &length) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "bad parameters");
		RETURN_NULL();
	}
	ZVAL_STRING(return_value, random_str_gen(length), 1);
}
/* }}} */

/* {{{ random_str_gen */
char* random_str_gen(int length)
{
	char* pass = emalloc(length+1);
	pass[length] = '\0';
	int i;
	for (i=0;i<length;i++) {
		int t = rand() % 62;
		if (t<10)
			pass[i] = '0' + t;
		else if (t<36)
			pass[i] = 'A' + t - 10;
		else
			pass[i] = 'a' + t - 36;
	}
	return pass;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
