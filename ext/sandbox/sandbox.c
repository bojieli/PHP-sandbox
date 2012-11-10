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
#include <sys/time.h>

ZEND_DECLARE_MODULE_GLOBALS(sandbox)

/* True global resources - no need for thread safety here */
static int le_sandbox;

/* {{{ sandbox_functions[]
 *
 * Every user visible function must have an entry in sandbox_functions[].
 */
const zend_function_entry sandbox_functions[] = {
	PHP_FE(confirm_sandbox_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(connect_userdb, NULL)
	PHP_FE(get_appid, NULL)
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
	STD_PHP_INI_ENTRY("sandbox.chroot_basedir", "/tmp", PHP_INI_SYSTEM, OnUpdateString, chroot_basedir, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.chroot_basedir_peruser", "", PHP_INI_SYSTEM, OnUpdateString, chroot_basedir_peruser, zend_sandbox_globals, sandbox_globals)
	STD_PHP_INI_ENTRY("sandbox.hostname_for_subdomain", "localhost", PHP_INI_SYSTEM, OnUpdateString, hostname_for_subdomain, zend_sandbox_globals, sandbox_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_sandbox_init_globals
 */
static void php_sandbox_init_globals(zend_sandbox_globals *sandbox_globals)
{
	sandbox_globals->admindb_mysql = NULL;
	sandbox_globals->admindb_sock = NULL;
	sandbox_globals->appid = 0;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(sandbox)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(sandbox)
{
	UNREGISTER_INI_ENTRIES();
	if (SANDBOX_G(admindb_sock))
		mysql_close(SANDBOX_G(admindb_sock) TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(sandbox)
{
	if (SANDBOX_G(admindb_mysql) == NULL && connect_admindb() == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot connect to admin database");
		return FAILURE;
	}
    gettimeofday(&(SANDBOX_G(start_time)), NULL);
	init_appid(TSRMLS_CC);
	if (SANDBOX_G(appid) <= 0) /* privileged subdomain or out of control */
		return SUCCESS;
	if (php_connect_userdb(SANDBOX_G(appid) TSRMLS_CC) &&
		set_basedir(TSRMLS_CC))
		return SUCCESS;
	else
		return FAILURE;
	return SUCCESS;
}
/* }}} */

/* {{{ connect_admindb */
int connect_admindb()
{
	zend_bool persistent = 1;
	SANDBOX_G(admindb_mysql) = mysql_init(persistent);
	SANDBOX_G(admindb_sock) = mysqlnd_connect(SANDBOX_G(admindb_mysql), 
		SANDBOX_G(admindb_host), SANDBOX_G(admindb_user), SANDBOX_G(admindb_pass),
		strlen(SANDBOX_G(admindb_pass)), NULL, 0, SANDBOX_G(admindb_port), 
		NULL /* donot use file sock */, 0 TSRMLS_CC);
	if (SANDBOX_G(admindb_sock))
		return SUCCESS;
	else
		return FAILURE;
}
/* }}} */

/* {{{ init_appid */
void init_appid(TSRMLS_DC)
{
	char *http_host;
	zval **array, **token;
    if (zend_hash_find(&EG(symbol_table), "_SERVER", sizeof("_SERVER"), (void **) &array) == SUCCESS &&
        Z_TYPE_PP(array) == IS_ARRAY &&
        zend_hash_find(Z_ARRVAL_PP(array), "HTTP_HOST", sizeof("HTTP_HOST"), (void **) &token) == SUCCESS
    ) {
        http_host = Z_STRVAL_PP(token);
    } else {
		goto die;
	}
	char* pos = strstr(http_host, SANDBOX_G(hostname_for_subdomain));
	if (pos == NULL)
		goto die;
	if (pos == http_host) // subdomain is empty
		goto privileged;
	
	if (!(pos = strstr(http_host, ".")))
		goto die;
	char *subdomain = emalloc(pos - http_host + 1);
	memcpy(subdomain, http_host, pos - http_host);
	subdomain[pos - http_host] = '\0';
	if (FAILURE == mysql_result_int(SANDBOX_G(admindb_sock), new_sprintf("SELECT id FROM appinfo WHERE `appname`='%s'", addslashes(subdomain)), &(SANDBOX_G(appid)) TSRMLS_CC))
		goto die;
	return;
	
privileged:
	SANDBOX_G(appid) = 0;
	return;
die:
	SANDBOX_G(appid) = -1; // out of control
	return;
}
/* }}} */

/* {{{ proto void connect_userdb(long appid)
	Privileged apps only  */
PHP_FUNCTION(connect_userdb)
{
	int appid;
	if (SANDBOX_G(appid) != 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "no privilege to call this function");
		RETURN_NULL();
	}
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &appid) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "bad parameters");	
	}
	php_connect_userdb(appid);
	RETURN_NULL();
}
/* }}} */

/* {{{ php_connect_userdb */
int php_connect_userdb(int appid TSRMLS_DC)
{
	MYSQL_ROW row = admindb_fetch_row("dbconf", "id", new_sprintf("%d", SANDBOX_G(appid)) TSRMLS_CC);
	zval *id, *hostname, *username, *password, *dbname;
	MAKE_STD_ZVAL(hostname);
	ZVAL_STRING(hostname, row[1], 0);
	MAKE_STD_ZVAL(username);
	ZVAL_STRING(username, row[2], 0);
	MAKE_STD_ZVAL(password);
	ZVAL_STRING(password, row[3], 0);
	MAKE_STD_ZVAL(dbname);
	ZVAL_STRING(dbname, row[4], 0);

	zval *connfunc;
	MAKE_STD_ZVAL(connfunc);
	ZVAL_STRING(connfunc, "mysql_connect", 1);

	zval **params[3] = {&hostname, &username, &password};
	zval *resource;
	if (call_user_function_ex(CG(function_table), NULL,
	    connfunc, &resource, 3, params, 0, NULL TSRMLS_CC) == FAILURE) {
		return FAILURE;
	}
	if (Z_TYPE_P(resource) != IS_RESOURCE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot connect to user database");
		return FAILURE;
	}
	
	zval *ret;
	zval **params1[2] = {&dbname, &resource};
	ZVAL_STRING(connfunc, "mysql_select_db", 1);
	if (call_user_function_ex(CG(function_table), NULL,
	    connfunc, &ret, 2, params1, 0, NULL TSRMLS_CC) == FAILURE) {
		return FAILURE;
	}
	if (Z_TYPE_P(ret) != IS_RESOURCE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "cannot select user database");
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

int set_basedir(TSRMLS_DC)
{
	zval *func, *entry, *value;
	MAKE_STD_ZVAL(func);
	ZVAL_STRING(func, "ini_set", 1);
	MAKE_STD_ZVAL(entry);
	ZVAL_STRING(entry, "open_basedir", 1);

	char *appid = new_sprintf("%d", SANDBOX_G(appid));
	if (strlen(appid) + strlen(SANDBOX_G(chroot_basedir)) + strlen(SANDBOX_G(chroot_basedir_peruser)) + 3 >= QUERY_MAXLEN) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "basedir too long");
		return FAILURE;
	}
	char *basedir = emalloc(QUERY_MAXLEN);
	bzero(basedir, QUERY_MAXLEN);
	if (strlen(SANDBOX_G(chroot_basedir)) > 0) {
		APPEND_STR(basedir, SANDBOX_G(chroot_basedir));
		APPEND_STR(basedir, ":");
	}
	char *userbase = SANDBOX_G(chroot_basedir_peruser);
	while (*userbase != '\0') {
		char* separator = strstr(userbase, ":");
		if (separator == NULL) { /* the last part */
			APPEND_STR(basedir, userbase);
			APPEND_STR(basedir, new_sprintf("/%d", appid));
			break;
		} else {
			memcpy(basedir + strlen(basedir), userbase, separator - userbase);
			APPEND_STR(basedir, new_sprintf("/%d", appid));
			APPEND_STR(basedir, ":");
			userbase = separator + 1; /* skip separator : */
		}
	}
	MAKE_STD_ZVAL(value);
	ZVAL_STRING(value, basedir, 1);

	zval **params[2] = {&entry, &value};
	zval *ret;
	if (call_user_function_ex(CG(function_table), NULL,
		func, &ret, 2, params, 0, NULL TSRMLS_CC) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "cannot set basedir");
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(sandbox)
{
	struct timeval end;
    gettimeofday(&end, NULL);
    long exec_time = (long)(end.tv_sec - SANDBOX_G(start_time).tv_sec) * 1000 
		* 1000 + end.tv_usec - SANDBOX_G(start_time).tv_usec;

	long query_num = 0;

	php_access_log(exec_time, query_num TSRMLS_CC);
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
	RETURN_LONG(SANDBOX_G(appid));
}
/* }}} */

/* {{{ php_get_appid */
int php_get_appid(TSRMLS_DC)
{
	return SANDBOX_G(appid);
}
/* }}} */

/* --- Supporting Functions --- */

/* {{{ new_sprintf */
char* new_sprintf(char* format, ...) {
    va_list arg;
    va_start(arg, format);
    char *buf = emalloc(QUERY_MAXLEN);
	bzero(buf, QUERY_MAXLEN);
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
	char *query = new_sprintf("UPDATE %s SET `%s`='%s' WHERE `id`='%d'", table, field, addslashes(value), appid);
	MYSQL_RES *res = do_mysql_query(SANDBOX_G(admindb_sock), query TSRMLS_CC);
	return res ? SUCCESS : FAILURE;
}
/* }}} */

/* {{{ admindb_delete_row */
int admindb_delete_row(const char* table, int appid TSRMLS_DC)
{
	char *query = new_sprintf("DELEDE FROM %s WHERE `id`='%d'", table, appid);
	MYSQL_RES *res = do_mysql_query(SANDBOX_G(admindb_sock), query TSRMLS_CC);
	return res ? SUCCESS : FAILURE;
}
/* }}} */

/* {{{ mysql_result */
int mysql_result(MYSQL* sock, char* query, char** result TSRMLS_DC) {
    MYSQL_RES *res = do_mysql_query(sock, query TSRMLS_CC);
    MYSQL_ROW row = mysql_fetch_row(res TSRMLS_CC);
    int length;
    if (row[0] == NULL || !(length = strlen(row[0])))
        return FAILURE;
    *result = emalloc(length + 1);
    memcpy(*result, row[0], length + 1);
    mysql_free_result(res);
    return SUCCESS;
}
/* }}} */

/* {{{ mysql_result_int */
int mysql_result_int(MYSQL* sock, char* query, long* result TSRMLS_DC) {
    char* res;
    if (!mysql_result(sock, query, &res TSRMLS_CC))
        return FAILURE;
    *result = atoi(res);
    efree(res);
    return SUCCESS;
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
