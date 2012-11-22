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

#ifndef PHP_BLOG_H
#define PHP_BLOG_H

extern zend_module_entry blog_module_entry;
#define phpext_blog_ptr &blog_module_entry

#ifdef PHP_WIN32
#	define PHP_BLOG_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_BLOG_API __attribute__ ((visibility("default")))
#else
#	define PHP_BLOG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(blog);
PHP_MSHUTDOWN_FUNCTION(blog);
PHP_RINIT_FUNCTION(blog);
PHP_RSHUTDOWN_FUNCTION(blog);
PHP_MINFO_FUNCTION(blog);

PHP_FUNCTION(get_appinfo);
PHP_FUNCTION(create_app);
PHP_FUNCTION(app_activate);
PHP_FUNCTION(app_deactivate);
PHP_FUNCTION(app_count);
PHP_FUNCTION(check_email_count);

char* random_str_gen(int length);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(blog)
	char *userdb_prefix;
	long  max_blogs_per_email;
ZEND_END_MODULE_GLOBALS(blog)

/* In every utility function you add that needs to use variables 
   in php_blog_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as BLOG_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define BLOG_G(v) TSRMG(blog_globals_id, zend_blog_globals *, v)
#else
#define BLOG_G(v) (blog_globals.v)
#endif

#endif	/* PHP_BLOG_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
