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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_blog.h"
#include "ext/standard/base64.h"
#include "ext/standard/url.h"
#include "Zend/zend_operators.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

/* If you declare any globals in php_blog.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(blog)
*/

/* True global resources - no need for thread safety here */
static int le_blog;

/* {{{ blog_functions[]
 *
 * Every user visible function must have an entry in blog_functions[].
 */
const zend_function_entry blog_functions[] = {
	PHP_FE(request_daemon, NULL)
	PHP_FE(install_blog_filesystem, NULL)
	PHP_FE(install_plugin, NULL)
	PHP_FE(remove_plugin, NULL)
	PHP_FE(sendmail, NULL)
	PHP_FE(http_get, NULL)
	PHP_FE(http_post, NULL)
	PHP_FE(parse_response, NULL)
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

#ifdef COMPILE_DL_BLOG
ZEND_GET_MODULE(blog)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("blog.daemon_hostname", "127.0.0.1", PHP_INI_SYSTEM, OnUpdateString, daemon_hostname, zend_blog_globals, blog_globals)
    STD_PHP_INI_ENTRY("blog.daemon_port", "0", PHP_INI_SYSTEM, OnUpdateLong, daemon_port, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.admindb_host", "localhost", PHP_INI_SYSTEM, OnUpdateString, admindb_host, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.admindb_name", "", PHP_INI_SYSTEM, OnUpdateString, admindb_name, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.admindb_user", "root", PHP_INI_SYSTEM, OnUpdateString, admindb_user, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.admindb_pass", "", PHP_INI_SYSTEM, OnUpdateString, admindb_pass, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.userdb_prefix", "ub_", PHP_INI_SYSTEM, OnUpdateString, userdb_prefix, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.chroot_basedir", "/tmp", PHP_INI_SYSTEM, OnUpdateString, chroot_basedir, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.chroot_basedir_peruser", "", PHP_INI_SYSTEM, OnUpdateString, chroot_basedir_peruser, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.hostname_for_subdomain", "localhost", PHP_INI_SYSTEM, OnUpdateString, hostname_for_subdomain, zend_blog_globals, blog_globals)
	STD_PHP_INI_ENTRY("blog.chroot_except_subdomain", "", PHP_INI_SYSTEM, OnUpdateString, chroot_except_subdomain, zend_blog_globals, blog_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_blog_init_globals
 */
static void php_blog_init_globals(zend_blog_globals *blog_globals)
{
	blog_globals->daemon_hostname = "127.0.0.1";
	blog_globals->daemon_port = 0;
	blog_globals->admindb_host = "localhost";
	blog_globals->admindb_name = "";
	blog_globals->admindb_user = "";
	blog_globals->admindb_pass = "";
	blog_globals->userdb_prefix = "ub_";
	blog_globals->chroot_basedir = "/tmp";
	blog_globals->chroot_basedir_peruser = "";
	blog_globals->hostname_for_subdomain = "";
	blog_globals->chroot_except_subdomain = "";
	blog_globals->admindb_conn = NULL;
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

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(blog)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(blog)
{
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


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_blog_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_blog_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "blog", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto bool install_blog_filesystem(int appid) */
PHP_FUNCTION(install_blog_filesystem)
{
	int appid;
	const char *method = "async-callback";
	const char *action = "install-blog-filesystem";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &appid) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: install_blog_filesystem(int appid)");
		RETURN_BOOL(false);
	}
	
	DEFINE_ARRAY(data);
	add_assoc_long(data, "appid", appid);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
	RETURN_BOOL(true);
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
		RETURN_BOOL(false);
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "plugin-name", plugin, plugin_len, 0);
	add_assoc_stringl(data, "remote-filename", filename, filename_len, 0);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
	RETURN_BOOL(true);
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
		RETURN_BOOL(false);
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "plugin-name", plugin, plugin_len, 0);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
	RETURN_BOOL(true);
}
/* }}} */

/* {{{ proto bool sendmail(string target, string subject, str ing content) */
PHP_FUNCTION(sendmail)
{
	char *target = NULL, *subject = NULL, *content = NULL;
	int target_len, subject_len, content_len;
	const char *method = "async-callback";
	const char *action = "install-blog-filesystem";
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &target, &target_len, &subject, &subject_len, &content, &content_len) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: sendmail(string target, string subject, string content)");
		RETURN_BOOL(false);
	}

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "target", target, target_len, 0);
	add_assoc_stringl(data, "subject", subject, subject_len, 0);
	add_assoc_stringl(data, "content", content, content_len, 0);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
	RETURN_BOOL(true);
}
/* }}} */

/* {{{ proto bool access_log(int exec-time, int query-num) */
PHP_FUNCTION(access_log)
{
	int exec_time, query_num;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &exec_time, &query_num) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "wrong parameters passed\n  Usage: access_log(int exec_time, int query_num)");
		RETURN_BOOL(false);
	}
	php_access_log(exec_time, query_num);
	RETURN_BOOL(true);
}
/* }}} */

/* {{{ proto array http_get(string url)
	return: {status: int, body: string} */
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
	const char *prefix = "http://api.wordpress.org/";
	char *full_url = emalloc(strlen(prefix) + url_len);
	memcpy(full_url, prefix, strlen(prefix));
	memcpy(full_url + strlen(prefix), url, url_len);

	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "url", full_url, strlen(prefix) + url_len, 0);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
}
/* }}} */

/* {{{ proto array http_post(string url, string body)
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
	const char *prefix = "http://api.wordpress.org/";
	char *full_url = emalloc(strlen(prefix) + url_len);
	memcpy(full_url, prefix, strlen(prefix));
	memcpy(full_url + strlen(prefix), url, url_len);
	
	DEFINE_ARRAY(data);
	add_assoc_stringl(data, "url", full_url, strlen(prefix) + url_len, 0);

	if (post_data) {
		char *body = emalloc(REQ_MAXLEN);
		int body_len = parse_post_params(post_data, body);
		if (body_len > 0)
			add_assoc_stringl(data, "body", body, body_len, 0);
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

/* {{{ proto INTERNAL void php_access_log(int exec_time, int query_num) */
void php_access_log(int exec_time, int query_num)
{
	const char *method = "async";
	const char *action = "access-log";
	zval *return_value = NULL;
	DEFINE_ARRAY(data);
	add_assoc_long(data, "exec-time", exec_time);
	add_assoc_long(data, "query-num", query_num);
	php_request_daemon(return_value, method, strlen(method), action, strlen(action), data);
}
/* }}} */

/* {{{ proto INTERNAL int parse_post_params(data, req_str) 
	return req_str_len */
/* key=value+key=value ... */
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
			REQ_APPEND_CONST("+");
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

/* {{{ proto INTERNAL void php_request_daemon(return_value, method, method_len, action, action_len, data) */
void php_request_daemon(zval* return_value, const char* method, int method_len, const char* action, int action_len, zval *data)
{
	int req_len;
	char *req_str = parse_request(method, method_len, action, action_len, data, &req_len);
	if (req_str == NULL)
		goto die;

	char *response = daemon_get_response(req_str, req_len);

	if (response == NULL || *response == '\0') {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Daemon returned empty response");
		goto die;
	}
	if (*response == '*') {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Daemon returned error: %s", response);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Original query string: %s", req_str);
		goto die;
	}

	if (! php_parse_response(return_value, response)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Daemon response parse failed");
		goto die;
	}
	if (response)
		efree(response);
	if (return_value == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Internal Error: NULL return value");
		goto die;
	}
	return;

die:
	if (req_str)
		efree(req_str);
	RETURN_NULL();
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
	REQ_APPEND_CONST("0"); // for testing
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
	char *host_name = BLOG_G(daemon_hostname);
	int port = BLOG_G(daemon_port);
	TSRMLS_FETCH();
	 
	if ((nlp_host = gethostbyname(host_name)) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot resolve host to daemon");
		return NULL;
	}
	 
	bzero(&pin, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = htonl(INADDR_ANY);  
	pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host->h_addr))->s_addr;
	pin.sin_port = htons(port);
	 
	sd = socket(AF_INET, SOCK_STREAM, 0);
	 
	if (connect(sd, (struct sockaddr*)&pin, sizeof(pin)) == -1){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot connect to daemon");
		return NULL;
	}
	
	send(sd, req_str, req_len, 0);

	char* buf = emalloc(RESPONSE_MAXLEN+1);
	bzero(buf, RESPONSE_MAXLEN+1);
	int length = recv(sd, buf, RESPONSE_MAXLEN, 0);
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
	ASSERT_RESPONSE_CHAR('\n');
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
