/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2002 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Jim Winstead <jimw@php.net>                                  |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#ifndef FOPEN_WRAPPERS_H
#define FOPEN_WRAPPERS_H

#include "php_globals.h"

#define IGNORE_PATH		0
#define USE_PATH		1
#define IGNORE_URL		2
/* There's no USE_URL. */
#ifdef PHP_WIN32
# define IGNORE_URL_WIN IGNORE_URL
#else
# define IGNORE_URL_WIN 0
#endif
#define ENFORCE_SAFE_MODE 4

#define REPORT_ERRORS	8

#ifdef PHP_WIN32
# define SOCK_ERR INVALID_SOCKET
# define SOCK_CONN_ERR SOCKET_ERROR
# define SOCK_RECV_ERR SOCKET_ERROR
#else
# define SOCK_ERR -1
# define SOCK_CONN_ERR -1
# define SOCK_RECV_ERR -1
#endif

/* values for issock */
#define IS_NOT_SOCKET	0
#define IS_SOCKET		1
#define BAD_URL			2

typedef FILE *(*php_fopen_url_wrapper_t)(const char *, char *, int, int *, int *, char ** TSRMLS_DC);

PHPAPI FILE *php_fopen_wrapper(char *filename, char *mode, int options, int *issock, int *socketd, char **opened_path TSRMLS_DC);

PHPAPI int php_fopen_primary_script(zend_file_handle *file_handle TSRMLS_DC);
PHPAPI char *expand_filepath(const char *filepath, char *real_path TSRMLS_DC);

PHPAPI int php_check_open_basedir(char *path TSRMLS_DC);
PHPAPI int php_check_specific_open_basedir(char *basedir, char *path TSRMLS_DC);

PHPAPI int php_check_safe_mode_include_dir(char *path TSRMLS_DC);

PHPAPI FILE *php_fopen_with_path(char *filename, char *mode, char *path, char **opened_path TSRMLS_DC);

PHPAPI int php_is_url(char *path);
PHPAPI char *php_strip_url_passwd(char *path);


int php_init_fopen_wrappers(TSRMLS_D);
int php_shutdown_fopen_wrappers(TSRMLS_D);
PHPAPI int php_register_url_wrapper(const char *protocol, php_fopen_url_wrapper_t wrapper TSRMLS_DC);
PHPAPI int php_unregister_url_wrapper(char *protocol TSRMLS_DC);

#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
