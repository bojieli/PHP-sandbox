/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Sterling Hughes <sterling@php.net>                          |
   +----------------------------------------------------------------------+
 */

#ifndef _PHP_XSLT_H
#define _PHP_XSLT_H

#include "php.h"

#ifdef HAVE_XSLT

#define XSLT_OBJ(__func)       (&(__func)->obj)
#define XSLT_FUNC(__func)      ((__func)->func)

struct xslt_function {
	zval *obj;
	zval *func;
};


extern void assign_xslt_handler(struct xslt_function **, zval **);
extern void free_xslt_handler(struct xslt_function *);
extern void call_xslt_function(char *, struct xslt_function *, int, zval **, zval **);

extern void xslt_debug(char *, char *, ...);

#endif

#endif
