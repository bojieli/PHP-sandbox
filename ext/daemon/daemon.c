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
#include "ext/standard/base64.h"
#include "ext/standard/url.h"
#include "ext/standard/info.h"
#include "ext/sandbox/php_sandbox.h"
#include "Zend/zend_operators.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "php_daemon.h"

ZEND_DECLARE_MODULE_GLOBALS(daemon)

/* True global resources - no need for thread safety here */
static int le_daemon;

/* {{{ daemon_functions[]
 *
 * Every user visible function must have an entry in daemon_functions[].
 */
const zend_function_entry daemon_functions[] = {
	PHP_FE(install_blog_filesystem, NULL)
	PHP_FE(install_plugin, NULL)
	PHP_FE(remove_plugin, NULL)
	PHP_FE(install_theme, NULL)
	PHP_FE(remove_theme, NULL)
	PHP_FE(sendmail, NULL)
	PHP_FE(set_3rdparty_domain, NULL)
	PHP_FE(install_ssl_key, NULL)
	PHP_FE(http_get, NULL)
	PHP_FE(http_post, NULL)
	PHP_FE(parse_response, NULL)
	PHP_FE_END	/* Must be the last line in daemon_functions[] */
};
/* }}} */

/* {{{ daemon_module_entry
 */
zend_module_entry daemon_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"daemon",
	daemon_functions,
	PHP_MINIT(daemon),
	PHP_MSHUTDOWN(daemon),
	PHP_RINIT(daemon),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(daemon),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(daemon),
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
#ifdef COMPILE_DL_DAEMON
ZEND_GET_MODULE(daemon)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("daemon.hostname", "127.0.0.1", PHP_INI_SYSTEM, OnUpdateString, daemon_hostname, zend_daemon_globals, daemon_globals)
    STD_PHP_INI_ENTRY("daemon.port", "0", PHP_INI_SYSTEM, OnUpdateLong, daemon_port, zend_daemon_globals, daemon_globals)
    STD_PHP_INI_ENTRY("daemon.allowed_hosts", "", PHP_INI_SYSTEM, OnUpdateString, allowed_hosts, zend_daemon_globals, daemon_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_daemon_init_globals
 */
static void php_daemon_init_globals(zend_daemon_globals *daemon_globals)
{
	daemon_globals->daemon_hostname = "";
	daemon_globals->daemon_port = 0;
	daemon_globals->allowed_hosts = "";
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(daemon)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(daemon)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(daemon)
{
	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(daemon)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(daemon)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "daemon support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{ proto bool install_blog_filesystem(string appname) */
PHP_FUNCTION(install_blog_filesystem)
{
	ASSERT_PRIVILEGE

	char* appname = NULL;
	int appname_len;
	const char *method = "sync";
	const char *action = "install-blog-filesystem";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &appname, &appname_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: install_blog_filesystem(string appname)");
		RETURN_FALSE;
	}
	
	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "appname", appname, appname_len, 0);
	RETURN_BOOL(php_request_daemon(return_value, method, strlen(method), action, strlen(action), data));
}
/* }}} */ 

/* {{{ proto bool install_plugin(string plugin_name, string remote_filename) */
PHP_FUNCTION(install_plugin)
{
	char *plugin = NULL, *filename = NULL;
	int plugin_len, filename_len;
	const char *method = "async-callback";
	const char *action = "install-plugin";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &plugin, &plugin_len, &filename, &filename_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: install_plugin(string plugin_name, string remote_filename)");
		RETURN_FALSE;
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "plugin-name", plugin, plugin_len, 0);
	add_assoc_stringl(data, "remote-filename", filename, filename_len, 0);
	RETURN_BOOL(php_request_daemon(return_value, method, strlen(method), action, strlen(action), data));
}
/* }}} */ 

/* {{{ proto bool remove_plugin(string plugin_name) */ 
PHP_FUNCTION(remove_plugin)
{
	char *plugin = NULL;
	int plugin_len;
	const char *method = "async-callback";
	const char *action = "remove-plugin";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &plugin, &plugin_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: remove_plugin(string plugin_name)");
		RETURN_FALSE;
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "plugin-name", plugin, plugin_len, 0);
	RETURN_BOOL(php_request_daemon(return_value, method, strlen(method), action, strlen(action), data));
}
/* }}} */

/* {{{ proto bool install_theme(string theme_name, string remote_filename) */
PHP_FUNCTION(install_theme)
{
	char *theme = NULL, *filename = NULL;
	int theme_len, filename_len;
	const char *method = "async-callback";
	const char *action = "install-theme";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &theme, &theme_len, &filename, &filename_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: install_theme(string theme_name, string remote_filename)");
		RETURN_FALSE;
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "theme-name", theme, theme_len, 0);
	add_assoc_stringl(data, "remote-filename", filename, filename_len, 0);
	RETURN_BOOL(php_request_daemon(return_value, method, strlen(method), action, strlen(action), data));
}
/* }}} */ 

/* {{{ proto bool remove_theme(string theme_name) */ 
PHP_FUNCTION(remove_theme)
{
	char *theme = NULL;
	int theme_len;
	const char *method = "async-callback";
	const char *action = "remove-theme";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &theme, &theme_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: remove_theme(string theme_name)");
		RETURN_FALSE;
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "theme-name", theme, theme_len, 0);
	RETURN_BOOL(php_request_daemon(return_value, method, strlen(method), action, strlen(action), data));
}
/* }}} */

/* {{{ proto bool sendmail(string target, string subject, string content) */
PHP_FUNCTION(sendmail)
{
	char *target = NULL, *subject = NULL, *content = NULL;
	int target_len, subject_len, content_len;
	const char *method = "async-callback";
	const char *action = "sendmail";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &target, &target_len, &subject, &subject_len, &content, &content_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: sendmail(string target, string subject, string content)");
		RETURN_FALSE;
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "target", target, target_len, 0);
	add_assoc_stringl(data, "subject", subject, subject_len, 0);
	add_assoc_stringl(data, "content", content, content_len, 0);
	RETURN_BOOL(php_request_daemon(return_value, method, strlen(method), action, strlen(action), data));
}
/* }}} */

/* {{{ proto long set_3rdparty_domain(string domain, bool is_ssl) */
PHP_FUNCTION(set_3rdparty_domain)
{
	char *domain = NULL;
	size_t domain_len;
	zend_bool is_ssl;
	const char *method = "sync";
	const char *action = "set-3rdparty-domain";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sb", &domain, &domain_len, &is_ssl) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: set_3rdparty_domain(string domain, bool is_ssl)");
		RETURN_FALSE;
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "domain", domain, domain_len, 0);
	add_assoc_long(data, "is_ssl", (is_ssl ? 1 : 0));
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
}
/* }}} */

/* {{{ proto long install_ssl_key(string domain) */
PHP_FUNCTION(install_ssl_key)
{
	char *domain = NULL;
	size_t domain_len;
	const char *method = "sync";
	const char *action = "install-ssl-key";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &domain, &domain_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: install_ssl_key(string domain)");
		RETURN_FALSE;
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "domain", domain, domain_len, 0);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
}
/* }}} */

/* {{{ proto bool access_log(int exec-time, int query-num) */
PHP_FUNCTION(access_log)
{
	int exec_time, query_num;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &exec_time, &query_num) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: access_log(int exec_time, int query_num)");
		RETURN_FALSE;
	}
	RETURN_BOOL(php_access_log(exec_time, query_num));
}
/* }}} */

/* {{{ proto bool is_url_allowed(string url) */
PHP_FUNCTION(is_url_allowed)
{
	char* url = NULL;
	int url_length;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &url_length) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: is_url_allowed(string url)");
		RETURN_FALSE;
	}
	RETURN_BOOL(is_url_allowed(url));
}
/* }}} */

/* {{{ is_url_allowed */
#define ISDIGIT(c) ((c)>='0' && (c)<='9')
#define ISLETTER(c) ((c)>='a' && (c)<='z' || (c)>='A' && (c)<='Z')
#define ISHOSTCHAR(c) (ISDIGIT(c) || ISLETTER(c) || (c)=='-')

int is_url_allowed(char* url)
{
	char *allowed_hosts = estrdup(DAEMON_G(allowed_hosts));
	char *comma_pos, *match;
	do {
		comma_pos = strchrnul(allowed_hosts, ',');
		*comma_pos = '\0'; /* for strstr */
		if (match = strstr(url, allowed_hosts)) {
			if (match == url)
				return true;
			--match;
			if (*match == '/')
				goto scheme;
			if (*match != '.')
				goto next;
			while (--match >= url) {
				if (*match == '/')
					goto scheme;
				if (! ISHOSTCHAR(*match) && *match!='.')
					goto next;
			}
			return true;
scheme:
			if (*(--match) != '/')
				goto next;
			if (*(--match) != ':')
				goto next;
			while (--match >= url)
				if (! ISLETTER(*match))
					goto next;
			return true;
		}
next:
		allowed_hosts = comma_pos + 1;
	} while(true);
	return false;
}
/* }}} */

/* {{{ proto array http_get(string url)
	return: {status: int, body: string} */
#define ASSERT_ALLOW_URL \
	char *root = php_app_root_url(); \
	if ((root == NULL || url != strstr(url, root)) && !is_url_allowed(url)) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "URL %s is not allowed to be accessed", url); \
		RETURN_NULL(); \
	}

PHP_FUNCTION(http_get)
{
	char *url = NULL;
	int url_len;
	const char *method = "sync";
	const char *action = "http-get";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &url_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: http_get(string url)");
		RETURN_NULL();
	}
	ASSERT_ALLOW_URL

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "url", url, url_len, 1);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
}
/* }}} */

/* {{{ proto array http_post(string url, [array post_data])
	return: {status: int, body: string} */
PHP_FUNCTION(http_post)
{
	char *url = NULL;
	int url_len;
	zval *post_data = NULL;
	const char *method = "sync";
	const char *action = "http-post";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &url, &url_len, &post_data) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: http_post(string url, string body)");
		RETURN_NULL();
	}
	ASSERT_ALLOW_URL
	
	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "url", url, url_len, 1);

	if (post_data) {
		char *body = emalloc(REQ_MAXLEN);
		int body_len = parse_post_params(post_data, body);
		if (body_len > 0)
			add_assoc_stringl(data, "body", body, body_len, 1);
		efree(body);
	}
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
}
/* }}} */

/* {{{ proto array request_daemon(string method, string action, array data)
   Return an array of data retrieved from daemon. */
PHP_FUNCTION(request_daemon)
{
	char *method = NULL, *action = NULL;
	int method_len, action_len;
	zval *data = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|a", &method, &method_len, &action, &action_len, &data) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: request_daemon(string method, string action, [array data])");
		RETURN_NULL();
	}

	php_request_daemon(return_value, method, method_len, action, action_len, data);
}
/* }}} */

/* {{{ proto array parse_response(string response) */
PHP_FUNCTION(parse_response)
{
	char *response = NULL;
	int response_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &response, &response_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: parse_response(string response)");
		RETURN_NULL();
	}

	php_parse_response(return_value, response);
}
/* }}} */

/* --- INTERNAL FUNCTIONS BELOW --- */

/* {{{ proto INTERNAL int php_access_log(long exec_time, long query_num) */
int php_access_log(long exec_time, long query_num)
{
	const char *method = "async";
	const char *action = "access-log";
	DEFINE_ARRAY(data);
	add_assoc_long(data, "exec-time", exec_time);
	add_assoc_long(data, "query-num", query_num);
	return php_request_daemon(NULL, method, strlen(method), action, strlen(action), data);
}
/* }}} */

/* {{{ proto INTERNAL int parse_post_params(data, req_str) 
	return req_str_len */
/* key=value&key=value ... */
int parse_post_params(zval* req, char* req_str)
{
	int req_len = 0;
	HashTable *req_hash = Z_ARRVAL_P(req);
	HashPosition req_pointer;
	zval **value;
	int first = true;

    for (zend_hash_internal_pointer_reset_ex(req_hash, &req_pointer);
		zend_hash_get_current_data_ex(req_hash, (void**) &value, &req_pointer) == SUCCESS; 
		zend_hash_move_forward_ex(req_hash, &req_pointer)) {

		if (!first) {
			REQ_APPEND_CONST("&");
		} else {
			first = false;
		}

		char *key;
		int key_len;
		long index;
		if (zend_hash_get_current_key_ex(req_hash, &key, &key_len, &index, 0, &req_pointer) == HASH_KEY_IS_STRING) {
			int i;
			key_len--; // original length includes \0
			for (i=0; i<key_len; i++) {
				if (! IS_KEY_CHAR(key[i])) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key of data should be [a-zA-Z0-9_-]+ (%s given)", key);
					goto die;
				}
			}
			REQ_APPEND(key, key_len);
        }
		else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key of data should be string (NOT number)");
			goto die;
		}

		REQ_APPEND_CONST("=");

		convert_to_string(*value);
		if (Z_TYPE_PP(value) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "array value of data should be able to convert to string");
			goto die;
		}
		int value_str_len;
		char *value_str = php_url_encode(Z_STRVAL_PP(value), Z_STRLEN_PP(value), &value_str_len);
		REQ_APPEND(value_str, value_str_len);
    }
	return req_len;
die:
	return -1;
}
/* }}} */

/* {{{ proto INTERNAL int php_request_daemon(return_value, method, method_len, action, action_len, data) */
int php_request_daemon(zval* return_value, const char* method, int method_len, const char* action, int action_len, zval *data)
{
	int req_len;
	char *req_str = parse_request(method, method_len, action, action_len, data, &req_len);
	if (req_str == NULL)
		goto die;

	char *response = daemon_get_response(req_str, req_len);

	if (response == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Daemon returned empty response");
		goto die;
	}
	if (*response == '\0') // when it comes to async requests
		return true;
	if (*response == '*') {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Daemon returned error: %s", response);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Original query string: %s", req_str);
		goto die;
	}

	if (return_value == NULL) // when it comes to access-log
		return true;

	if (! php_parse_response(return_value, response)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Daemon response parse failed");
		goto die;
	}
	if (return_value == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Internal Error: NULL return value");
		goto die;
	}
	response ? efree(response) : 0;
	req_str ? efree(req_str) : 0;
	return true;

die:
	response ? efree(response) : 0;
	req_str ? efree(req_str) : 0;
	if (return_value == NULL)
		return false;
	RETURN_NULL();
	return false;
}
/* }}} */

/* {{{ proto INTERNAL char* parse_request()
 */
char* parse_request(const char* method, int method_len, const char* action, int action_len, zval *req, int* ret_req_len) 
{
	char *req_str = (char*)emalloc(REQ_MAXLEN);
	int req_len = 0;
	
	bzero(req_str, REQ_MAXLEN);
	REQ_APPEND_CONST("method:p:");
	REQ_APPEND(method, method_len);
	REQ_APPEND_CONST("\n");
	REQ_APPEND_CONST("action:p:");
	REQ_APPEND(action, action_len);
	REQ_APPEND_CONST("\n");

    REQ_APPEND_CONST("appid:p:");
    long appid = php_get_appid();
    if (appid == 0 && req) {
        appid = get_appid_from_request_data(req);
    }
    if (appid < 0)
        appid = 0;
    char *appid_str = ltostr(appid);
    REQ_APPEND(appid_str, strlen(appid_str));
	REQ_APPEND_CONST("\n");

	REQ_APPEND_CONST("appname:b:");
	char *appname = php_get_appname(TSRMLS_CC);
	if (appid == 0 && req) {
		appname = get_appname_from_request_data(req);
	}
	if (appname == NULL || strlen(appname) == 0)
		appname = "unknown";
	int appname_len;
	appname = php_base64_encode(appname, strlen(appname), &appname_len);
	REQ_APPEND(appname, appname_len);
	REQ_APPEND_CONST("\n\n");

	if (req) {
		req_len = parse_request_data(req_str, req, req_len);
		if (req_len < 0)
			goto die;
	}

	*ret_req_len = req_len;
	return req_str;
die:
	if (req_str)
		efree(req_str);
	return NULL;
}
/* }}} */

/* {{{ INTERNAL get_appid_from_request_data */
long get_appid_from_request_data(zval* req)
{
	zval **appid = NULL;
	if (FAILURE == zend_hash_find(Z_ARRVAL_P(req), "appid", sizeof("appid"), (void *)&appid))
		return 0;
	if (Z_TYPE_PP(appid) != IS_LONG)
		return 0;
	return Z_LVAL_PP(appid);	
}
/* }}} */

/* {{{ INTERNAL get_appname_from_request_data */
char* get_appname_from_request_data(zval* req)
{
	zval **appname = NULL;
	if (FAILURE == zend_hash_find(Z_ARRVAL_P(req), "appname", sizeof("appname"), (void *)&appname))
		return NULL;
	if (Z_TYPE_PP(appname) != IS_STRING)
		return NULL;
	return Z_STRVAL_PP(appname);
}
/* }}} */

/* {{{ proto INTERNAL int parse_request_data(req_str, req, req_len)
	return: new req_len */
int parse_request_data(char* req_str, zval* req, int req_len) 
{
	HashTable *req_hash = Z_ARRVAL_P(req);
	HashPosition req_pointer;
	zval **value;

    for (zend_hash_internal_pointer_reset_ex(req_hash, &req_pointer);
		zend_hash_get_current_data_ex(req_hash, (void**) &value, &req_pointer) == SUCCESS; 
		zend_hash_move_forward_ex(req_hash, &req_pointer)) {

		/* key:b:base64(value) */
		char *key;
		int key_len;
		long index;

		if (zend_hash_get_current_key_ex(req_hash, &key, &key_len, &index, 0, &req_pointer) == HASH_KEY_IS_STRING) {
			int i;
			key_len--; // original length includes \0
			for (i=0; i<key_len; i++) {
				if (! IS_KEY_CHAR(key[i])) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key of data should be [a-zA-Z0-9_-]+ (%s given)", key);
					goto die;
				}
			}
			REQ_APPEND(key, key_len);
        }
		else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key of data should be string (NOT number)");
			goto die;
		}

		REQ_APPEND_CONST(":b:");

		convert_to_string(*value);
		if (Z_TYPE_PP(value) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "array value of data should be able to convert to string");
			goto die;
		}
		int value_str_len;
		char *value_str = php_base64_encode(Z_STRVAL_PP(value), Z_STRLEN_PP(value), &value_str_len);
		REQ_APPEND(value_str, value_str_len);
		REQ_APPEND_CONST("\n");
    }
	return req_len;
die:
	return -1;
}
/* }}} */

/* {{{ proto INTERNAL char* daemon_get_response(req_str, req_len) */
char* daemon_get_response(char* req_str, int req_len) 
{
	struct sockaddr_in pin;
	struct hostent *nlp_host;
	int sd; 
	char *host_name = DAEMON_G(daemon_hostname);
	int port = DAEMON_G(daemon_port);
	TSRMLS_FETCH();
	 
	if ((nlp_host = gethostbyname(host_name)) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot resolve host to daemon");
		return NULL;
	}
	 
	bzero(&pin, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = htonl(INADDR_ANY);  
	pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host->h_addr))->s_addr;
	pin.sin_port = htons(port);
	 
	sd = socket(AF_INET, SOCK_STREAM, 0);
	 
	if (connect(sd, (struct sockaddr*)&pin, sizeof(pin)) == -1){
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot connect to daemon");
		return NULL;
	}
	
	send(sd, req_str, req_len, 0);

	char* buf = emalloc(RESPONSE_MAXLEN+1);
	bzero(buf, RESPONSE_MAXLEN+1);
	char* buf_end = buf;
	int length;
	while (0 < (length = recv(sd, buf_end, RESPONSE_MAXLEN-(buf_end-buf), 0)))
		buf_end += length;

	if (buf_end >= buf + RESPONSE_MAXLEN)
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Daemon response is too long");

	close(sd);
	return buf;
}
/* }}} */

/* {{{ proto INTERNAL int php_parse_response(return_value, response)
 */
int php_parse_response(zval* return_value, char* response) 
{
	char type;
	char *orig_response;
	char *value;
	char *key;
	int value_len;

	array_init(return_value);

state_key:
	orig_response = response;
	while (IS_KEY_CHAR(*response)) {
		response++;
	}
	ASSERT_RESPONSE_CHAR(':');
	key = emalloc(response - orig_response + 1);
	memcpy(key, orig_response, response - orig_response);
	key[response - orig_response] = '\0';

state_type:
	response++;
	switch (*response) {
		case 'b': type = 'b';break;
		case 'u': type = 'u';break;
		case 'p': type = 'p';break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unimplemented response encoding type");
			return 0;
	}
	response++;
	ASSERT_RESPONSE_CHAR(':');

state_value:
	response++;
	orig_response = response;
	while (IS_VALUE_CHAR(*response))
		response++;
	switch (type) {
		case 'b':
			value = php_base64_decode(orig_response, response - orig_response, &value_len);
			break;
		case 'u':
			value_len = php_url_decode(orig_response, response - orig_response);
			value = orig_response;
			break;
		case 'p':
			value_len = response - orig_response;
			value = orig_response;
			break;
	}
	add_assoc_stringl(return_value, key, value, value_len, 1);

state_crlf:
	while (*response == '\n')
		response++;
	if (*response != '\0')
		goto state_key;

	return 1;
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
