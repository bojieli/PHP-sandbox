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
#include "php_blog.h"
#include "ext/sandbox/php_sandbox.h"

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
	PHP_FE(confirm_blog_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(get_appinfo, NULL)
	PHP_FE(create_app, NULL)
	PHP_FE(email_activate, NULL)
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
	STD_PHP_INI_ENTRY("blog.userdb_prefix", "ub_", PHP_INI_SYSTEM, OnUpdateString, userdb_prefix, zend_sandbox_globals, sandbox_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_blog_init_globals
 */
static void php_blog_init_globals(zend_blog_globals *blog_globals)
{
	blog_globals->userdb_prefix = "";
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

/* {{{ proto int get_appinfo */
PHP_FUNCTION(get_appinfo)
{
}
/* }}} */

/* {{{ proto int create_app(appname, username, email, password) 
	RETURN appid */
PHP_FUNCTION(create_app)
{
}
/* }}} */

/* {{{ proto int email_activate(appid, token) */
PHP_FUNCTION(email_activate)
{
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
