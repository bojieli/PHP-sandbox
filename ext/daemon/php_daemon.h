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

#ifndef PHP_DAEMON_H
#define PHP_DAEMON_H

extern zend_module_entry daemon_module_entry;
#define phpext_daemon_ptr &daemon_module_entry

#ifdef PHP_WIN32
#	define PHP_DAEMON_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_DAEMON_API __attribute__ ((visibility("default")))
#else
#	define PHP_DAEMON_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(daemon);
PHP_MSHUTDOWN_FUNCTION(daemon);
PHP_RINIT_FUNCTION(daemon);
PHP_RSHUTDOWN_FUNCTION(daemon);
PHP_MINFO_FUNCTION(daemon);

PHP_FUNCTION(request_daemon);	
PHP_FUNCTION(install_blog_filesystem);
PHP_FUNCTION(install_plugin);
PHP_FUNCTION(remove_plugin);
PHP_FUNCTION(sendmail);
PHP_FUNCTION(http_get);
PHP_FUNCTION(http_post);
PHP_FUNCTION(parse_response);

int php_access_log(long exec_time, long query_num);
int php_request_daemon(zval* return_value, const char* method, int method_len, const char* action, int action_len, zval *data);
char* parse_request(const char* method, int method_len, const char* action, int action_len, zval *req, int* ret_req_len);
long get_appid_from_request_data(zval* req);
char* get_appname_from_request_data(zval* req);
int parse_request_data(char* req_str, zval* req, int req_len);
char* daemon_get_response(char* req_str, int req_len);
int php_parse_response(zval* return_value, char* response);
int parse_post_params(zval* req, char* req_str);

#define true 1
#define false 0

#define DEFINE_ARRAY(data) zval* data; \
	MAKE_STD_ZVAL(data); \
	array_init(data)

#define REQ_MAXLEN 65536
#define REQ_APPEND(str,len) do { \
	if (req_len + (len) >= REQ_MAXLEN) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "request too long"); \
		break; \
	} \
	memcpy(req_str + req_len, (str), (len)); \
	req_len += (len); \
} while(0)
#define REQ_APPEND_CONST(str) REQ_APPEND((str), strlen(str))
#define IS_KEY_CHAR(c) ((c)>='0' && (c)<='9' || (c)>='a' && (c)<='z' || (c)>='A' && (c)<='Z' || (c)=='-' || (c)=='_')

#define RESPONSE_MAXLEN 512000

#define ASSERT_RESPONSE_CHAR(c) do { \
	if (*response != c) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unexpected character in daemon response 0x%x (%c expected)", *response, c); \
		return 0; \
	} \
} while(0)
#define IS_VALUE_CHAR(c) ((c)>=33 && (c)<=126 && (c)!=':')

#define KEY_MAXLEN 256

#ifndef QUERY_MAXLEN
#define QUERY_MAXLEN 512
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(daemon)
	char *daemon_hostname;
	int   daemon_port;
	char *http_prefix;
ZEND_END_MODULE_GLOBALS(daemon)

/* In every utility function you add that needs to use variables 
   in php_daemon_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as DAEMON_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define DAEMON_G(v) TSRMG(daemon_globals_id, zend_daemon_globals *, v)
#else
#define DAEMON_G(v) (daemon_globals.v)
#endif

#endif	/* PHP_DAEMON_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
