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
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("blog.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_blog_globals, blog_globals)
    STD_PHP_INI_ENTRY("blog.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_blog_globals, blog_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_blog_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_blog_init_globals(zend_blog_globals *blog_globals)
{
	blog_globals->global_value = 0;
	blog_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(blog)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(blog)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
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

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
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
		goto die;
	}

	if (! parse_response(return_value, response)) {
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
char* parse_request(char* method, int method_len, char* action, int action_len, zval *req, int* ret_req_len) {

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

	HashTable *req_hash = Z_ARRVAL_P(req);
	HashPosition req_pointer;
	zval **value;
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

		if (Z_TYPE_PP(value) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "array value of data should be string");
			goto die;
		}
		int value_str_len;
		char *value_str = php_base64_encode(Z_STRVAL_PP(value), Z_STRLEN_PP(value), &value_str_len);
		REQ_APPEND(value_str, value_str_len);
		REQ_APPEND_CONST("\n");
    }
	*ret_req_len = req_len;
	return req_str;
die:
	if (req_str)
		efree(req_str);
	return NULL;
}
/* }}} */

/* {{{ proto INTERNAL char* daemon_get_response(req_str, req_len)
 */
char* daemon_get_response(char* req_str, int req_len) {
	struct sockaddr_in pin;
	struct hostent *nlp_host;
	int sd; 
	char *host_name = "127.0.0.1";
	int port = 12696;
	 
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

#define RESPONSE_MAXLEN 512000
	char* buf = emalloc(RESPONSE_MAXLEN+1);
	bzero(buf, RESPONSE_MAXLEN+1);
	int length = recv(sd, buf, RESPONSE_MAXLEN, 0);
	close(sd);

	return buf;
}
/* }}} */

/* {{{ proto INTERNAL int parse_response(return_value, response)
 */
int parse_response(zval* return_value, char* response) {
#define ASSERT_RESPONSE_CHAR(c) do { \
	if (*response != c) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unexpected character in daemon response 0x%x (%c expected)", *response, c); \
		return 0; \
	} \
} while(0)
#define IS_VALUE_CHAR(c) ((c)>=33 && (c)<=126 && (c)!=':')

#define KEY_MAXLEN 256
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
