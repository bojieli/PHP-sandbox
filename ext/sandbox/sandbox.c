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
#include "php_sandbox.h"
#include "Zend/zend_list.h"
#include "ext/daemon/php_daemon.h"
#include "ext/mysql/php_mysql_structs.h"

ZEND_DECLARE_MODULE_GLOBALS(sandbox)

/* True global resources - no need for thread safety here */
static int le_sandbox;

/* {{{ sandbox_functions[]
 *
 * Every user visible function must have an entry in sandbox_functions[].
 */
const zend_function_entry sandbox_functions[] = {
	PHP_FE(confirm_sandbox_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(get_appid, NULL)
	PHP_FE(get_appinfo, NULL)
	PHP_FE_END	/* Must be the last line in sandbox_functions[] */
};
/* }}} */

/* {{{ sandbox_module_entry
 */
zend_module_entry sandbox_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"sandbox",
	sandbox_functions,
	PHP_MINIT(sandbox),
	PHP_MSHUTDOWN(sandbox),
	PHP_RINIT(sandbox),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(sandbox),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(sandbox),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

static const zend_module_dep mysql_deps[] = {
    ZEND_MOD_REQUIRED("mysqlnd")
	ZEND_MOD_REQUIRED("mysql")
	ZEND_MOD_REQUIRED("daemon")
	ZEND_MOD_END
};

#ifdef COMPILE_DL_SANDBOX
ZEND_GET_MODULE(sandbox)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("sandbox.admindb_host", "localhost", PHP_INI_SYSTEM, OnUpdateString, admindb_host, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.admindb_port", "3306", PHP_INI_SYSTEM, OnUpdateLong, admindb_port, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.admindb_name", "", PHP_INI_SYSTEM, OnUpdateString, admindb_name, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.admindb_user", "root", PHP_INI_SYSTEM, OnUpdateString, admindb_user, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.admindb_pass", "", PHP_INI_SYSTEM, OnUpdateString, admindb_pass, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.userdb_prefix", "ub_", PHP_INI_SYSTEM, OnUpdateString, userdb_prefix, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.chroot_basedir", "/tmp", PHP_INI_SYSTEM, OnUpdateString, chroot_basedir, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.chroot_basedir_peruser", "", PHP_INI_SYSTEM, OnUpdateString, chroot_basedir_peruser, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.hostname_for_subdomain", "localhost", PHP_INI_SYSTEM, OnUpdateString, hostname_for_subdomain, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.chroot_except_subdomain", "", PHP_INI_SYSTEM, OnUpdateString, chroot_except_subdomain, zend_sandbox_globals, sandbox_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_sandbox_init_globals
 */
static void php_sandbox_init_globals(zend_sandbox_globals *sandbox_globals)
{
	sandbox_globals->admindb_mysql = NULL;
	sandbox_globals->admindb_sock = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(sandbox)
{
	REGISTER_INI_ENTRIES();
	if (connect_admindb() == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot connect to admin database");
		return FAILURE;
	}
	return SUCCESS;
}

int connect_admindb()
{
	SANDBOX_G(admindb_mysql) = mysql_init(1); /* persistent */
	SANDBOX_G(admindb_sock) = mysqlnd_connect(SANDBOX_G(admindb_mysql), 
		SANDBOX_G(admindb_host), SANDBOX_G(admindb_user), SANDBOX_G(admindb_pass),
		strlen(SANDBOX_G(admindb_pass)), NULL, 0, SANDBOX_G(admindb_port), 
		NULL /* donot use sock */, 0 TSRMLS_CC);
	if (SANDBOX_G(admindb_sock))
		return SUCCESS;
	else
		return FAILURE;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(sandbox)
{
	UNREGISTER_INI_ENTRIES();
	mysql_close(SANDBOX_G(admindb_sock) TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(sandbox)
{
	int appid = php_get_appid(TSRMLS_DC);
	if (appid == 0) /* not in the range of control */
		return SUCCESS;
	if (connect_userdb(appid TSRMLS_DC) &&
		set_basedir(appid TSRMLS_DC))
		return SUCCESS;
	else
		return FAILURE;
	return SUCCESS;
}

int connect_userdb(int appid TSRMLS_DC)
{
	char *query = new_sprintf("SELECT * FROM appinfo WHERE id='%s'", php_get_appid());
	MYSQL_RES *row = do_mysql_query(SANDBOX_G(admindb_sock), query TSRMLS_CC);

	// call user func to make a resource
}

int set_basedir(int appid TSRMLS_DC)
{
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(sandbox)
{
	//php_access_log(exec_time, query_num);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(sandbox)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "sandbox support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_sandbox_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_sandbox_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "sandbox", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto int get_appid */
PHP_FUNCTION(get_appid)
{
	RETURN_LONG(php_get_appid(TSRMLS_DC));
}
/* }}} */

/* {{{ php_get_appid */
int php_get_appid(TSRMLS_DC)
{
}
/* }}} */

/* --- Supporting Functions --- */

/* {{{ new_sprintf */
char* new_sprintf(char* format, ...) {
    va_list arg;
    va_start(arg, format);
    char *buf = emalloc(QUERY_MAXLEN);
    vsprintf(buf, format, arg);
    va_end(arg);
    return buf;
}
/* }}} */

/* {{{ do_mysql_query */
MYSQL_RES* do_mysql_query(MYSQL* sock, char* query TSRMLS_DC) {
    MYSQL_RES *res;
    if (mysql_real_query(sock, query, strlen(query) TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Query failed (%s)", mysql_error(sock TSRMLS_CC));
        return NULL;
    }
    if (!(res = mysql_store_result(sock TSRMLS_CC))) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Cannot get result (%s)", mysql_error(sock TSRMLS_CC));
        return NULL;
    }
    return res;
}
// Remember to do:
// mysql_free_result(MYSQL_RES *result);
/* }}} */

/* {{{ admindb_fetch_row */
MYSQL_ROW admindb_fetch_row(const char* table, const char* field, char* value TSRMLS_DC)
{
	char *query = new_sprintf("SELECT * FROM %s WHERE `%s`='%s'", table, field, addslashes(value));
    MYSQL_RES *res = do_mysql_query(SANDBOX_G(admindb_sock), query TSRMLS_CC);
	if (res)
    	return mysql_fetch_row(res TSRMLS_CC);
	return NULL;
}
/* }}} */

/* {{{ admindb_update_row */
int admindb_update_row(const char* table, int appid, const char* field, char* value TSRMLS_DC)
{
	char *query = new_sprintf("UPDATE %s SET `%s`='%s' WHERE `id`='%s'", table, field, addslashes(value), appid);
	MYSQL_RES *res = do_mysql_query(SANDBOX_G(admindb_sock), query TSRMLS_CC);
	return res ? 1 : 0;
}
/* }}} */

/* {{{ admindb_delete_row */
int admindb_delete_row(const char* table, int appid TSRMLS_DC)
{
	char *query = new_sprintf("DELEDE FROM %s WHERE `id`='%s'", table, appid);
	MYSQL_RES *res = do_mysql_query(SANDBOX_G(admindb_sock), query TSRMLS_CC);
	return res ? 1 : 0;
}
/* }}} */

/* {{{ mysql_result */
int mysql_result(MYSQL* sock, char* query, char** result TSRMLS_DC) {
    MYSQL_RES *res = do_mysql_query(sock, query TSRMLS_CC);
    MYSQL_ROW row = mysql_fetch_row(res TSRMLS_CC);
    int length;
    if (row[0] == NULL || !(length = strlen(row[0])))
        return 0;
    *result = emalloc(length + 1);
    memcpy(*result, row[0], length + 1);
    mysql_free_result(res);
    return 1;
}
/* }}} */

/* {{{ mysql_result_int */
int mysql_result_int(MYSQL* sock, char* query TSRMLS_DC) {
    int res_int;
    char* res;
    if (!mysql_result(sock, query, &res TSRMLS_CC))
        return 0;
    res_int = atoi(res);
    efree(res);
    return res_int;
}
/* }}} */

/* {{{ buffered_get_next_row */
MYSQL_RES* do_buffered_get_next_row(MYSQL* new_sock, char* new_query, MYSQL_ROW* row) {
    static unsigned buffered = 0;
    static unsigned start = 0;
    static char* query = NULL;
    static MYSQL* sock;
    static MYSQL_RES *res = NULL;

    if (new_query) {
        buffered = start = 0;
        sock = new_sock;
        if (query)
            efree(query);
        query = emalloc(strlen(new_query) + 1);
        strcpy(query, new_query);
        return NULL;
    }

    if (buffered == 0) {
        if (res)
            mysql_free_result(res TSRMLS_CC);
        char *full_query = new_sprintf("%s LIMIT %d,%d", query, start, BUFFER_SIZE);
        res = do_mysql_query(sock, full_query TSRMLS_CC);
        efree(full_query);
        if (!res)
            return NULL;
        if (!(buffered = mysql_num_rows(res TSRMLS_CC)))
            return NULL;
        start += buffered;
    }
    buffered--;
    *row = mysql_fetch_row(res TSRMLS_CC);
    return res;
}

inline void init_buffered_get_next_row(MYSQL* sock, char* query TSRMLS_DC) {
    do_buffered_get_next_row(sock, query, NULL TSRMLS_CC);
}

inline MYSQL_RES* buffered_get_next_row(MYSQL_ROW* row TSRMLS_DC) {
    return do_buffered_get_next_row(NULL, NULL, row TSRMLS_CC);
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
