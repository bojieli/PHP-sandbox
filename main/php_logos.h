/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2003 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */


#ifndef _PHP_LOGOS_H
#define _PHP_LOGOS_H

PHPAPI int php_register_info_logo(char *logo_string, char *mimetype, unsigned char *data, int size);
PHPAPI int php_unregister_info_logo(char *logo_string);
int php_init_info_logos(void);
int php_shutdown_info_logos(void);
int php_info_logos(const char *logo_string TSRMLS_DC);

#endif /* _PHP_LOGOS_H */
