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

#ifndef PHP_SANDBOX_H
#define PHP_SANDBOX_H

#include "ext/mysqlnd/mysqlnd.h"
#include "ext/mysqlnd/mysqlnd_libmysql_compat.h"

extern zend_module_entry sandbox_module_entry;
#define phpext_sandbox_ptr &sandbox_module_entry

#ifdef PHP_WIN32
#	define PHP_SANDBOX_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SANDBOX_API __attribute__ ((visibility("default")))
#else
#	define PHP_SANDBOX_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(sandbox);
PHP_MSHUTDOWN_FUNCTION(sandbox);
PHP_RINIT_FUNCTION(sandbox);
PHP_RSHUTDOWN_FUNCTION(sandbox);
PHP_MINFO_FUNCTION(sandbox);

PHP_FUNCTION(confirm_sandbox_compiled);	/* For testing, remove later. */
PHP_FUNCTION(get_appid);
PHP_FUNCTION(connect_userdb);

int connect_admindb();
MYSQL_ROW admindb_fetch_row(const char* table, const char* field, char* value TSRMLS_DC);
int admindb_update_row(const char* table, int appid, const char* field, char* value TSRMLS_DC);
int admindb_delete_row(const char* table, int appid TSRMLS_DC);

void init_appid(TSRMLS_DC);
int php_get_appid(TSRMLS_DC);
int php_connect_userdb(int appid TSRMLS_DC);
int set_basedir(TSRMLS_DC);

char* new_sprintf(char* format, ...);

MYSQL_RES* do_mysql_query(MYSQL* sock, char* query TSRMLS_DC);
int mysql_result(MYSQLND* sock, char* query, char** result TSRMLS_DC);
int mysql_result_int(MYSQL* sock, char* query, long* result TSRMLS_DC);
MYSQL_RES* do_buffered_get_next_row(MYSQL* new_sock, char* new_query, MYSQL_ROW* row TSRMLS_DC);
void init_buffered_get_next_row(MYSQL* sock, char* query TSRMLS_DC);
MYSQL_RES* buffered_get_next_row(MYSQL_ROW* row TSRMLS_DC);

#ifndef QUERY_MAXLEN
#define QUERY_MAXLEN 512
#endif

#define BUFFER_SIZE 65536
#define addslashes(str) php_addslashes((str), strlen(str), NULL, 1 TSRMLS_CC)
#define APPEND_STR(target,source) memcpy(target + strlen(target), source, strlen(source))

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(sandbox)
	// INI settings
	char *admindb_host;
	long  admindb_port;
	char *admindb_name;
	char *admindb_user;
	char *admindb_pass;
	char *userdb_prefix;
	char *chroot_basedir;
	char *chroot_basedir_peruser;
	char *hostname_for_subdomain;
	// module scope
	MYSQL *admindb_mysql;
	MYSQL *admindb_sock;
	// request scope
	long  appid; /* -1 for out of control, 0 for privileged, >0 for apps */
	struct timeval start_time;
ZEND_END_MODULE_GLOBALS(sandbox)

/* In every utility function you add that needs to use variables 
   in php_sandbox_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SANDBOX_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SANDBOX_G(v) TSRMG(sandbox_globals_id, zend_sandbox_globals *, v)
#else
#define SANDBOX_G(v) (sandbox_globals.v)
#endif

#endif	/* PHP_SANDBOX_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
