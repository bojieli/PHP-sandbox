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
   | Author : Rui Hirokawa        <hirokawa@php.net>                      |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifndef ZEND_DBX_PGSQL_H
#define ZEND_DBX_PGSQL_H

#ifndef INIT_FUNC_ARGS
#include "zend_modules.h"
#endif

#include "php.h"

int dbx_pgsql_connect(zval **rv, zval **host, zval **db, zval **username, zval **password, INTERNAL_FUNCTION_PARAMETERS);
	/* returns connection handle as resource on success or 0 as long on failure */
int dbx_pgsql_pconnect(zval **rv, zval **host, zval **db, zval **username, zval **password, INTERNAL_FUNCTION_PARAMETERS);
	/* returns persistent connection handle as resource on success or 0 as long on failure */
int dbx_pgsql_close(zval **rv, zval **dbx_handle, INTERNAL_FUNCTION_PARAMETERS);
	/* returns 1 as long on success or 0 as long on failure */
int dbx_pgsql_query(zval **rv, zval **dbx_handle, zval **db_name, zval **sql_statement, INTERNAL_FUNCTION_PARAMETERS);
	/* returns 1 as long or a result identifier as resource on success  or 0 as long on failure */
int dbx_pgsql_getcolumncount(zval **rv, zval **result_handle, INTERNAL_FUNCTION_PARAMETERS);
	/* returns column-count as long on success or 0 as long on failure */
int dbx_pgsql_getcolumnname(zval **rv, zval **result_handle, long column_index, INTERNAL_FUNCTION_PARAMETERS);
	/* returns column-name as string on success or 0 as long on failure */
int dbx_pgsql_getcolumntype(zval **rv, zval **result_handle, long column_index, INTERNAL_FUNCTION_PARAMETERS);
	/* returns column-type as string on success or 0 as long on failure */
int dbx_pgsql_getrow(zval **rv, zval **result_handle, long row_number, INTERNAL_FUNCTION_PARAMETERS);
	/* returns array[0..columncount-1] as strings on success or 0 as long on failure */
int dbx_pgsql_error(zval **rv, zval **dbx_handle, INTERNAL_FUNCTION_PARAMETERS);
	/* returns string */

#endif /* ZEND_DBX_PGSQL_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
