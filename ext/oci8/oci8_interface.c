/*
   +----------------------------------------------------------------------+
   | PHP Version 6                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2009 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Stig S�ther Bakken <ssb@php.net>                            |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |                                                                      |
   | Collection support by Andy Sautins <asautins@veripost.net>           |
   | Temporary LOB support by David Benson <dbenson@mancala.com>          |
   | ZTS per process OCIPLogon by Harald Radi <harald.radi@nme.at>        |
   |                                                                      |
   | Redesigned by: Antony Dovgal <antony@zend.com>                       |
   |                Andi Gutmans <andi@zend.com>                          |
   |                Wez Furlong <wez@omniti.com>                          |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_ini.h"

#if HAVE_OCI8

#include "php_oci8.h"
#include "php_oci8_int.h"

#ifndef OCI_STMT_CALL
#define OCI_STMT_CALL 10
#endif

/* {{{ proto bool oci_define_by_name(resource stmt, string name, mixed &var [, int type]) U
   Define a PHP variable to an Oracle column by name */
/* if you want to define a LOB/CLOB etc make sure you allocate it via OCINewDescriptor BEFORE defining!!! */
PHP_FUNCTION(oci_define_by_name)
{
	zval *stmt, *var;
	zstr name;
	int name_len;
	zend_uchar name_type;
	long type = 0;
	php_oci_statement *statement;
	php_oci_define *define, *tmp_define;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rtz/|l", &stmt, &name, &name_len, &name_type, &var, &type) == FAILURE) {
		return;
	}

	if (!name_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Column name cannot be empty");
		RETURN_FALSE;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(stmt, statement);

	if (statement->defines == NULL) {
		ALLOC_HASHTABLE(statement->defines);
		zend_hash_init(statement->defines, 13, NULL, php_oci_define_hash_dtor, 0);
	}

	define = ecalloc(1,sizeof(php_oci_define));

	if (zend_hash_add(statement->defines, name.s, USTR_BYTES(name_type, name_len+1), define, sizeof(php_oci_define), (void **)&tmp_define) == SUCCESS) {
		efree(define);
		define = tmp_define;
	} else {
		efree(define);
		RETURN_FALSE;
	}

	if (name_type == IS_UNICODE) {
		define->name.u = eustrndup(name.u, name_len);
	} else {
		define->name.s = estrndup(name.s, name_len);
	}

	define->name_len = name_len;
	define->name_type = name_type;
	define->type = type;
	define->zval = var;
	zval_add_ref(&var);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_bind_by_name(resource stmt, string name, mixed &var, [, int maxlength [, int type]]) U
   Bind a PHP variable to an Oracle placeholder by name */
/* if you want to bind a LOB/CLOB etc make sure you allocate it via OCINewDescriptor BEFORE binding!!! */
PHP_FUNCTION(oci_bind_by_name)
{
	ub2	bind_type = SQLT_CHR; /* unterminated string */
	int name_len;
	long maxlen = -1, type = 0;
	zstr name;
	zend_uchar name_type;
	zval *z_statement;
	zval *bind_var = NULL;
	php_oci_statement *statement;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rtz/|ll", &z_statement, &name, &name_len, &name_type, &bind_var, &maxlen, &type) == FAILURE) {
		return;
	}

	if (type) {
		bind_type = (ub2) type;
	}
	
	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (php_oci_bind_by_name(statement, name, name_len, bind_var, maxlen, bind_type, name_type TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_bind_array_by_name(resource stmt, string name, array &var, int max_table_length [, int max_item_length [, int type ]]) U
   Bind a PHP array to an Oracle PL/SQL type by name */
PHP_FUNCTION(oci_bind_array_by_name)
{
	int name_len;
	long max_item_len = -1;
	long max_array_len = 0;
	long type = SQLT_AFC;
	zstr name;
	zend_uchar name_type;
	zval *z_statement;
	zval *bind_var = NULL;
	php_oci_statement *statement;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rtz/l|ll", &z_statement, &name, &name_len, &name_type, &bind_var, &max_array_len, &max_item_len, &type) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (ZEND_NUM_ARGS() == 5 && max_item_len <= 0) {
		max_item_len = -1;
	}
	
	if (max_array_len <= 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Maximum array length must be greater than zero");
		RETURN_FALSE;
	}
	
	if (php_oci_bind_array_by_name(statement, name, name_len, bind_var, max_array_len, max_item_len, type, name_type TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_free_descriptor() U
   Deletes large object description */
PHP_FUNCTION(oci_free_descriptor)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;

	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);

	zend_list_delete(descriptor->id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_lob_save( string data [, int offset ]) U
   Saves a large object */
PHP_FUNCTION(oci_lob_save)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	zstr data;
	int data_len;
	zend_uchar data_type;
	long offset = 0;
	ub4 bytes_written;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|l", &data, &data_len, &data_type, &offset) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ot|l", &z_descriptor, oci_lob_class_entry_ptr, &data, &data_len, &data_type, &offset) == FAILURE) {
			return;
		}
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);

	if (offset < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Offset parameter must be greater than or equal to 0");
		RETURN_FALSE;
	}

	if (php_oci_lob_write(descriptor, offset, data, USTR_BYTES(data_type, data_len), &bytes_written TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_lob_import( string filename ) U
   Loads file into a LOB */
PHP_FUNCTION(oci_lob_import)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	char *filename;
	int filename_len;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &z_descriptor, oci_lob_class_entry_ptr, &filename, &filename_len) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);

	if (php_oci_lob_import(descriptor, filename TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string oci_lob_load() U
   Loads a large object */
PHP_FUNCTION(oci_lob_load)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	zstr buffer = NULL_ZSTR;
	ub4 buffer_len;
	php_oci_lob_type lob_type;

	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);

	if (php_oci_lob_read(descriptor, -1, 0, &buffer, &buffer_len TSRMLS_CC)) {
		RETURN_FALSE;
	}
	
	if (php_oci_lob_get_type(descriptor, &lob_type TSRMLS_CC) > 0) {
		RETURN_FALSE;
	}

	switch (lob_type) {
		case OCI_IS_CLOB:
			if (buffer_len > 0) {
				RETURN_TEXTL(buffer, TEXT_CHARS(buffer_len), 0);
			}
			RETURN_EMPTY_TEXT();
			break;
		case OCI_IS_BLOB:
			if (buffer_len > 0) {
				RETURN_STRINGL(buffer.s, buffer_len, 0);
			}
			RETURN_EMPTY_STRING();
			break;
	}
}
/* }}} */

/* {{{ proto string oci_lob_read( int length ) U
   Reads particular part of a large object */
PHP_FUNCTION(oci_lob_read)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	long length;
	zstr buffer;
	ub4 buffer_len;
	php_oci_lob_type lob_type;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &length) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &z_descriptor, oci_lob_class_entry_ptr, &length) == FAILURE) {
			return;
		}	
	}

	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);

	if (length <= 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter must be greater than 0");
		RETURN_FALSE;
	}
	
	if (php_oci_lob_read(descriptor, length, descriptor->lob_current_position, &buffer, &buffer_len TSRMLS_CC)) {
		RETURN_FALSE;
	}
	
	if (php_oci_lob_get_type(descriptor, &lob_type TSRMLS_CC) > 0) {
		RETURN_FALSE;
	}

	switch (lob_type) {
		case OCI_IS_CLOB:
			if (buffer_len > 0) {
				RETURN_TEXTL(buffer, TEXT_CHARS(buffer_len), 0);
			}
			RETURN_EMPTY_TEXT();
			break;
		case OCI_IS_BLOB:
			if (buffer_len > 0) {
				RETURN_STRINGL(buffer.s, buffer_len, 0);
			}
			RETURN_EMPTY_STRING();
			break;
	}
}
/* }}} */

/* {{{ proto bool oci_lob_eof() U
   Checks if EOF is reached */
PHP_FUNCTION(oci_lob_eof)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	ub4 lob_length;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (!php_oci_lob_get_length(descriptor, &lob_length TSRMLS_CC) && lob_length >= 0) {
		if (lob_length == descriptor->lob_current_position) {
			RETURN_TRUE;
		}
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int oci_lob_tell() U
   Tells LOB pointer position */
PHP_FUNCTION(oci_lob_tell)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	RETURN_LONG(descriptor->lob_current_position);	
}
/* }}} */

/* {{{ proto bool oci_lob_rewind() U
   Rewind pointer of a LOB */
PHP_FUNCTION(oci_lob_rewind)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	descriptor->lob_current_position = 0;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_lob_seek( int offset [, int whence ]) U
   Moves the pointer of a LOB */
PHP_FUNCTION(oci_lob_seek)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	long offset, whence = PHP_OCI_SEEK_SET;
	ub4 lob_length;
	
	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &offset, &whence) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol|l", &z_descriptor, oci_lob_class_entry_ptr, &offset, &whence) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (php_oci_lob_get_length(descriptor, &lob_length TSRMLS_CC)) {
		RETURN_FALSE;
	}

	switch(whence) {
		case PHP_OCI_SEEK_CUR:
			descriptor->lob_current_position += offset;
			break;
		case PHP_OCI_SEEK_END:
			if ((descriptor->lob_size + offset) >= 0) {
				descriptor->lob_current_position = descriptor->lob_size + offset;
			}
			else {
				descriptor->lob_current_position = 0;
			}
			break;
		case PHP_OCI_SEEK_SET:
		default:
			descriptor->lob_current_position = (offset > 0) ? offset : 0;
			break;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int oci_lob_size() U
   Returns size of a large object */
PHP_FUNCTION(oci_lob_size)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	ub4 lob_length;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (php_oci_lob_get_length(descriptor, &lob_length TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_LONG(lob_length);
}
/* }}} */

/* {{{ proto int oci_lob_write( string string [, int length ]) U
   Writes data to current position of a LOB */
PHP_FUNCTION(oci_lob_write)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	int data_len;
	long write_len = 0;
	ub4 bytes_written;
	zstr data;
	zend_uchar data_type;
	
	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|l", &data, &data_len, &data_type, &write_len) == FAILURE) {
			return;
		}
		
		if (ZEND_NUM_ARGS() == 2) {
			data_len = MIN(data_len, write_len);
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ot|l", &z_descriptor, oci_lob_class_entry_ptr, &data, &data_len, &data_type, &write_len) == FAILURE) {
			return;
		}

		if (ZEND_NUM_ARGS() == 3) {
			data_len = MIN(data_len, write_len);
		}
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (data_len <= 0) {
		RETURN_LONG(0);
	}

	if (php_oci_lob_write(descriptor, descriptor->lob_current_position, data, USTR_BYTES(data_type, data_len), &bytes_written TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_LONG(bytes_written);
}
/* }}} */

/* {{{ proto bool oci_lob_append( object lob ) U
   Appends data from a LOB to another LOB */
PHP_FUNCTION(oci_lob_append)
{
	zval **tmp_dest, **tmp_from, *z_descriptor_dest = getThis(), *z_descriptor_from;
	php_oci_descriptor *descriptor_dest, *descriptor_from;
	
	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor_from, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &z_descriptor_dest, oci_lob_class_entry_ptr, &z_descriptor_from, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor_dest), "descriptor", sizeof("descriptor"), (void **)&tmp_dest) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property. The first argument should be valid descriptor object");
		RETURN_FALSE;
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor_from), "descriptor", sizeof("descriptor"), (void **)&tmp_from) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property. The second argument should be valid descriptor object");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp_dest, descriptor_dest);
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp_from, descriptor_from);
	
	if (php_oci_lob_append(descriptor_dest, descriptor_from TSRMLS_CC)) {
		RETURN_FALSE;
	}
	/* XXX should we increase lob_size here ? */
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_lob_truncate( [ int length ]) U
   Truncates a LOB */
PHP_FUNCTION(oci_lob_truncate)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	long trim_length = 0;
	ub4 ub_trim_length;
	
	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &trim_length) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|l", &z_descriptor, oci_lob_class_entry_ptr, &trim_length) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	if (trim_length < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length must be greater than or equal to zero");
		RETURN_FALSE;
	}

	ub_trim_length = (ub4) trim_length;
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (php_oci_lob_truncate(descriptor, ub_trim_length TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int oci_lob_erase( [ int offset [, int length ] ] ) U
   Erases a specified portion of the internal LOB, starting at a specified offset */
PHP_FUNCTION(oci_lob_erase)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	ub4 bytes_erased;
	long offset = -1, length = -1;
	
	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ll", &offset, &length) == FAILURE) {
			return;
		}
	
		if (ZEND_NUM_ARGS() > 0 && offset < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Offset must be greater than or equal to 0");
			RETURN_FALSE;
		}
		
		if (ZEND_NUM_ARGS() > 1 && length < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length must be greater than or equal to 0");
			RETURN_FALSE;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|ll", &z_descriptor, oci_lob_class_entry_ptr, &offset, &length) == FAILURE) {
			return;
		}
			
		if (ZEND_NUM_ARGS() > 1 && offset < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Offset must be greater than or equal to 0");
			RETURN_FALSE;
		}
		
		if (ZEND_NUM_ARGS() > 2 && length < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length must be greater than or equal to 0");
			RETURN_FALSE;
		}
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}

	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (php_oci_lob_erase(descriptor, offset, length, &bytes_erased TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_LONG(bytes_erased);
}
/* }}} */

/* {{{ proto bool oci_lob_flush( [ int flag ] ) U
   Flushes the LOB buffer */
PHP_FUNCTION(oci_lob_flush)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	long flush_flag = 0;
	
	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flush_flag) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|l", &z_descriptor, oci_lob_class_entry_ptr, &flush_flag) == FAILURE) {
			return;
		}
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (descriptor->buffering == PHP_OCI_LOB_BUFFER_DISABLED) {
		/* buffering wasn't enabled, there is nothing to flush */
		RETURN_FALSE;
	}

	if (php_oci_lob_flush(descriptor, flush_flag TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ocisetbufferinglob( boolean flag ) U
   Enables/disables buffering for a LOB */
PHP_FUNCTION(ocisetbufferinglob)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	zend_bool flag;
	
	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &flag) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ob", &z_descriptor, oci_lob_class_entry_ptr, &flag) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (php_oci_lob_set_buffering(descriptor, flag TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ocigetbufferinglob() U
   Returns current state of buffering for a LOB */
PHP_FUNCTION(ocigetbufferinglob)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}

	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (descriptor->buffering != PHP_OCI_LOB_BUFFER_DISABLED) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool oci_lob_copy( object lob_to, object lob_from [, int length ] ) U
   Copies data from a LOB to another LOB */
PHP_FUNCTION(oci_lob_copy)
{
	zval **tmp_dest, **tmp_from, *z_descriptor_dest, *z_descriptor_from;
	php_oci_descriptor *descriptor_dest, *descriptor_from;
	long length = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO|l", &z_descriptor_dest, oci_lob_class_entry_ptr, &z_descriptor_from, oci_lob_class_entry_ptr, &length) == FAILURE) {
		return;
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor_dest), "descriptor", sizeof("descriptor"), (void **)&tmp_dest) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property. The first argument should be valid descriptor object");
		RETURN_FALSE;
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor_from), "descriptor", sizeof("descriptor"), (void **)&tmp_from) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property. The second argument should be valid descriptor object");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp_dest, descriptor_dest);
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp_from, descriptor_from);
	
	if (ZEND_NUM_ARGS() == 3 && length < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter must be greater than 0");
		RETURN_FALSE;
	}
	
	if (ZEND_NUM_ARGS() == 2) {
		/* indicate that we want to copy from the current position to the end of the LOB */
		length = -1;
	}

	if (php_oci_lob_copy(descriptor_dest, descriptor_from, length TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_lob_is_equal( object lob1, object lob2 ) U
   Tests to see if two LOB/FILE locators are equal */
PHP_FUNCTION(oci_lob_is_equal)
{
	zval **tmp_first, **tmp_second, *z_descriptor_first, *z_descriptor_second;
	php_oci_descriptor *descriptor_first, *descriptor_second;
	boolean is_equal;
		
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &z_descriptor_first, oci_lob_class_entry_ptr, &z_descriptor_second, oci_lob_class_entry_ptr) == FAILURE) {
		return;
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor_first), "descriptor", sizeof("descriptor"), (void **)&tmp_first) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property. The first argument should be valid descriptor object");
		RETURN_FALSE;
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor_second), "descriptor", sizeof("descriptor"), (void **)&tmp_second) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property. The second argument should be valid descriptor object");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp_first, descriptor_first);
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp_second, descriptor_second);

	if (php_oci_lob_is_equal(descriptor_first, descriptor_second, &is_equal TSRMLS_CC)) {
		RETURN_FALSE;
	}
	
	if (is_equal == TRUE) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool oci_lob_export([string filename [, int start [, int length]]]) U
   Writes a large object into a file */
PHP_FUNCTION(oci_lob_export)
{	
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	char *filename;
	zstr buffer;
	int filename_len;
	long start = -1, length = -1, block_length;
	php_stream *stream;
	ub4 lob_length;
	php_oci_lob_type lob_type;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &filename, &filename_len, &start, &length) == FAILURE) {
			return;
		}
	
		if (ZEND_NUM_ARGS() > 1 && start < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Start parameter must be greater than or equal to 0");
			RETURN_FALSE;
		}
		if (ZEND_NUM_ARGS() > 2 && length < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter must be greater than or equal to 0");
			RETURN_FALSE;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os|ll", &z_descriptor, oci_lob_class_entry_ptr, &filename, &filename_len, &start, &length) == FAILURE) {
			return;
		}
			
		if (ZEND_NUM_ARGS() > 2 && start < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Start parameter must be greater than or equal to 0");
			RETURN_FALSE;
		}
		if (ZEND_NUM_ARGS() > 3 && length < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length parameter must be greater than or equal to 0");
			RETURN_FALSE;
		}
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);
	
	if (php_oci_lob_get_length(descriptor, &lob_length TSRMLS_CC)) {
		RETURN_FALSE;
	}		
	
	if (start == -1) {
		start = 0;
	}

	if (length == -1) {
		length = lob_length - descriptor->lob_current_position;
	}
	
	if (length == 0) {
		/* nothing to write, fail silently */
		RETURN_FALSE;
	}
	
	if (php_check_open_basedir(filename TSRMLS_CC)) {
		RETURN_FALSE;
	}

	stream = php_stream_open_wrapper_ex(filename, "w", REPORT_ERRORS, NULL, NULL);

	block_length = PHP_OCI_LOB_BUFFER_SIZE;
	if (block_length > length) {
		block_length = length;
	}

	if (php_oci_lob_get_type(descriptor, &lob_type TSRMLS_CC)) {
		RETURN_FALSE;
	}

	while(length > 0) {
		ub4 tmp_bytes_read = 0;
		if (php_oci_lob_read(descriptor, block_length, start, &buffer, &tmp_bytes_read TSRMLS_CC)) {
			php_stream_close(stream);
			RETURN_FALSE;
		}
		if (tmp_bytes_read && !php_stream_u_write(stream, (lob_type == OCI_IS_CLOB ? IS_UNICODE : IS_STRING), buffer, tmp_bytes_read)) {
			php_stream_close(stream);
			efree(buffer.v);
			RETURN_FALSE;
		}
		if (buffer.v) {
			efree(buffer.v);
		}
		
		length -= tmp_bytes_read;
		descriptor->lob_current_position += tmp_bytes_read;
		start += tmp_bytes_read;

		if (block_length > length) {
			block_length = length;
		}
	}

	php_stream_close(stream);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_lob_write_temporary(string var [, int lob_type]) U
   Writes temporary blob */
PHP_FUNCTION(oci_lob_write_temporary)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	zstr data;
	int data_len;
	zend_uchar data_type;
	long type = OCI_TEMP_CLOB;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t|l", &data, &data_len, &data_type, &type) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ot|l", &z_descriptor, oci_lob_class_entry_ptr, &data, &data_len, &data_type, &type) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);

	if (php_oci_lob_write_tmp(descriptor, type, data, USTR_BYTES(data_type, data_len) TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_lob_close() U
   Closes lob descriptor */
PHP_FUNCTION(oci_lob_close)
{
	zval **tmp, *z_descriptor = getThis();
	php_oci_descriptor *descriptor;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_descriptor, oci_lob_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_descriptor), "descriptor", sizeof("descriptor"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find descriptor property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_DESCRIPTOR(*tmp, descriptor);

	if (php_oci_lob_close(descriptor TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto object oci_new_descriptor(resource connection [, int type]) U
   Initialize a new empty descriptor LOB/FILE (LOB is default) */
PHP_FUNCTION(oci_new_descriptor)
{
	zval *z_connection;
	php_oci_connection *connection;
	php_oci_descriptor *descriptor;
	long type = OCI_DTYPE_LOB;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &z_connection, &type) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	/* php_oci_lob_create() checks type */
	descriptor = php_oci_lob_create(connection, type TSRMLS_CC);	
	
	if (!descriptor) {
		RETURN_NULL();
	}

	object_init_ex(return_value, oci_lob_class_entry_ptr);
	add_property_resource(return_value, "descriptor", descriptor->id);
}
/* }}} */

/* {{{ proto bool oci_rollback(resource connection) U
   Rollback the current context */
PHP_FUNCTION(oci_rollback)
{
	zval *z_connection;
	php_oci_connection *connection;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_connection) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if (connection->descriptors) {
		zend_hash_destroy(connection->descriptors);
		efree(connection->descriptors);
		connection->descriptors = NULL;
	}

	if (php_oci_connection_rollback(connection TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_commit(resource connection) U
   Commit the current context */
PHP_FUNCTION(oci_commit)
{
	zval *z_connection;
	php_oci_connection *connection;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_connection) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if (connection->descriptors) {
		zend_hash_destroy(connection->descriptors);
		efree(connection->descriptors);
		connection->descriptors = NULL;
	}
	
	if (php_oci_connection_commit(connection TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string oci_field_name(resource stmt, int col) U
   Tell the name of a column */
PHP_FUNCTION(oci_field_name)
{
	php_oci_out_column *column;

	if ( ( column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		RETURN_TEXTL(column->name, column->name_len, 1);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int oci_field_size(resource stmt, int col) U
   Tell the maximum data size of a column */
PHP_FUNCTION(oci_field_size)
{
	php_oci_out_column *column;

	if ( ( column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		/* Handle data type of LONG */
		if (column->data_type == SQLT_LNG){
			RETURN_LONG(column->storage_size4);
		}
		RETURN_LONG(column->data_size);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int oci_field_scale(resource stmt, int col) U
   Tell the scale of a column */
PHP_FUNCTION(oci_field_scale)
{
	php_oci_out_column *column;

	if ( ( column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		RETURN_LONG(column->scale);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int oci_field_precision(resource stmt, int col) U
   Tell the precision of a column */
PHP_FUNCTION(oci_field_precision)
{
	php_oci_out_column *column;

	if ( ( column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		RETURN_LONG(column->precision);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto mixed oci_field_type(resource stmt, int col) U
   Tell the data type of a column */
PHP_FUNCTION(oci_field_type)
{
	php_oci_out_column *column;

	column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);

	if (!column) {
		RETURN_FALSE;
	}
	
	switch (column->data_type) {
#ifdef SQLT_TIMESTAMP
		case SQLT_TIMESTAMP:
			RETVAL_ASCII_STRING("TIMESTAMP", ZSTR_DUPLICATE);
			break;
#endif
#ifdef SQLT_TIMESTAMP_TZ
		case SQLT_TIMESTAMP_TZ:
			RETVAL_ASCII_STRING("TIMESTAMP WITH TIMEZONE", ZSTR_DUPLICATE);
			break;
#endif
#ifdef SQLT_TIMESTAMP_LTZ
		case SQLT_TIMESTAMP_LTZ:
			RETVAL_ASCII_STRING("TIMESTAMP WITH LOCAL TIMEZONE", ZSTR_DUPLICATE);
			break;
#endif
#ifdef SQLT_INTERVAL_YM
		case SQLT_INTERVAL_YM:
			RETVAL_ASCII_STRING("INTERVAL YEAR TO MONTH", ZSTR_DUPLICATE);
			break;
#endif
#ifdef SQLT_INTERVAL_DS
		case SQLT_INTERVAL_DS:
			RETVAL_ASCII_STRING("INTERVAL DAY TO SECOND", ZSTR_DUPLICATE);
			break;
#endif
		case SQLT_DAT:
			RETVAL_ASCII_STRING("DATE", ZSTR_DUPLICATE);
			break;
		case SQLT_NUM:
			RETVAL_ASCII_STRING("NUMBER", ZSTR_DUPLICATE);
			break;
		case SQLT_LNG:
			RETVAL_ASCII_STRING("LONG", ZSTR_DUPLICATE);
			break;
		case SQLT_BIN:
			RETVAL_ASCII_STRING("RAW", ZSTR_DUPLICATE);
			break;
		case SQLT_LBI:
			RETVAL_ASCII_STRING("LONG RAW", ZSTR_DUPLICATE);
			break;
		case SQLT_CHR:
			RETVAL_ASCII_STRING("VARCHAR2", ZSTR_DUPLICATE);
			break;
		case SQLT_RSET:
			RETVAL_ASCII_STRING("REFCURSOR", ZSTR_DUPLICATE);
			break;
		case SQLT_AFC:
			RETVAL_ASCII_STRING("CHAR", ZSTR_DUPLICATE);
			break;
		case SQLT_BLOB:
			RETVAL_ASCII_STRING("BLOB", ZSTR_DUPLICATE);
			break;
		case SQLT_CLOB:
			RETVAL_ASCII_STRING("CLOB", ZSTR_DUPLICATE);
			break;
		case SQLT_BFILE:
			RETVAL_ASCII_STRING("BFILE", ZSTR_DUPLICATE);
			break;
		case SQLT_RDD:
			RETVAL_ASCII_STRING("ROWID", ZSTR_DUPLICATE);
			break;
		default:
			RETVAL_LONG(column->data_type);
	}
}
/* }}} */

/* {{{ proto int oci_field_type_raw(resource stmt, int col) U
   Tell the raw oracle data type of a column */
PHP_FUNCTION(oci_field_type_raw)
{
	php_oci_out_column *column;

	column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
	if (column) {
		RETURN_LONG(column->data_type);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool oci_field_is_null(resource stmt, int col) U
   Tell whether a column is NULL */
PHP_FUNCTION(oci_field_is_null)
{
	php_oci_out_column *column;

	if ( ( column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0) ) ) {
		if (column->indicator == -1) {
			RETURN_TRUE;
		}
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto void oci_internal_debug(int onoff) U
   Toggle internal debugging output for the OCI extension */
PHP_FUNCTION(oci_internal_debug)
{
	zend_bool on_off;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &on_off) == FAILURE) {
		return;
	}
	OCI_G(debug_mode) = on_off;
}
/* }}} */

/* {{{ proto bool oci_execute(resource stmt [, int mode]) U
   Execute a parsed statement */
PHP_FUNCTION(oci_execute)
{
	zval *z_statement;
	php_oci_statement *statement;
	long mode = OCI_COMMIT_ON_SUCCESS;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &z_statement, &mode) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (php_oci_statement_execute(statement, mode TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_cancel(resource stmt) U
   Cancel reading from a cursor */
PHP_FUNCTION(oci_cancel)
{
	zval *z_statement;
	php_oci_statement *statement;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_statement) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (php_oci_statement_cancel(statement TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_fetch(resource stmt) U
   Prepare a new row of data for reading */
PHP_FUNCTION(oci_fetch)
{
	zval *z_statement;
	php_oci_statement *statement;
	ub4 nrows = 1; /* only one row at a time is supported for now */

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_statement) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (php_oci_statement_fetch(statement, nrows TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ocifetchinto(resource stmt, array &output [, int mode]) U
   Fetch a row of result data into an array */
PHP_FUNCTION(ocifetchinto)
{
	php_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_OCI_NUM, 3);
}
/* }}} */

/* {{{ proto int oci_fetch_all(resource stmt, array &output[, int skip[, int maxrows[, int flags]]]) U
   Fetch all rows of result data into an array */
PHP_FUNCTION(oci_fetch_all)
{
	zval *z_statement, *array, *element, *tmp;
	php_oci_statement *statement;
	php_oci_out_column **columns;
	zval ***outarrs;
	ub4 nrows = 1;
	int i;
	long rows = 0, flags = 0, skip = 0, maxrows = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz/|lll", &z_statement, &array, &skip, &maxrows, &flags) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	zval_dtor(array);
	array_init(array);

	while (skip--) {
		if (php_oci_statement_fetch(statement, nrows TSRMLS_CC)) {
			RETURN_LONG(0);
		}
	}

	if (flags & PHP_OCI_FETCHSTATEMENT_BY_ROW) {
		columns = safe_emalloc(statement->ncolumns, sizeof(php_oci_out_column *), 0);

		for (i = 0; i < statement->ncolumns; i++) {
			columns[ i ] = php_oci_statement_get_column(statement, i + 1, NULL_ZSTR, 0 TSRMLS_CC);
		}

		while (!php_oci_statement_fetch(statement, nrows TSRMLS_CC)) {
			zval *row;
			
			MAKE_STD_ZVAL(row);
			array_init(row);

			for (i = 0; i < statement->ncolumns; i++) {
				MAKE_STD_ZVAL(element);
				php_oci_column_to_zval(columns[ i ], element, PHP_OCI_RETURN_LOBS TSRMLS_CC);

				if (flags & PHP_OCI_NUM) {
					zend_hash_next_index_insert(Z_ARRVAL_P(row), &element, sizeof(zval*), NULL);
				} else { /* default to ASSOC */
					zend_u_symtable_update(Z_ARRVAL_P(row), (UG(unicode) ? IS_UNICODE : IS_STRING), columns[ i ]->name, columns[ i ]->name_len+1, &element, sizeof(zval*), NULL);
				}
			}

			zend_hash_next_index_insert(Z_ARRVAL_P(array), &row, sizeof(zval*), NULL);
			rows++;

			if (maxrows != -1 && rows == maxrows) {
				php_oci_statement_cancel(statement TSRMLS_CC);
				break;
			}
		}
		efree(columns);

	} else { /* default to BY_COLUMN */
		columns = safe_emalloc(statement->ncolumns, sizeof(php_oci_out_column *), 0);
		outarrs = safe_emalloc(statement->ncolumns, sizeof(zval*), 0);
		
		if (flags & PHP_OCI_NUM) {
			for (i = 0; i < statement->ncolumns; i++) {
				columns[ i ] = php_oci_statement_get_column(statement, i + 1, NULL_ZSTR, 0 TSRMLS_CC);
				
				MAKE_STD_ZVAL(tmp);
				array_init(tmp);
				zend_hash_next_index_insert(Z_ARRVAL_P(array), &tmp, sizeof(zval*), (void **) &(outarrs[ i ]));
			}
		} else { /* default to ASSOC */
			for (i = 0; i < statement->ncolumns; i++) {
				columns[ i ] = php_oci_statement_get_column(statement, i + 1, NULL_ZSTR, 0 TSRMLS_CC);
				
				MAKE_STD_ZVAL(tmp);
				array_init(tmp);
				zend_u_symtable_update(Z_ARRVAL_P(array), (UG(unicode) ? IS_UNICODE : IS_STRING), columns[ i ]->name, columns[ i ]->name_len+1, (void *) &tmp, sizeof(zval*), (void **) &(outarrs[ i ]));
			}
		}

		while (!php_oci_statement_fetch(statement, nrows TSRMLS_CC)) {
			for (i = 0; i < statement->ncolumns; i++) {
				MAKE_STD_ZVAL(element);
				php_oci_column_to_zval(columns[ i ], element, PHP_OCI_RETURN_LOBS TSRMLS_CC);
				zend_hash_index_update((*(outarrs[ i ]))->value.ht, rows, (void *)&element, sizeof(zval*), NULL);
			}

			rows++;

			if (maxrows != -1 && rows == maxrows) {
				php_oci_statement_cancel(statement TSRMLS_CC);
				break;
			}
		}
		
		efree(columns);
		efree(outarrs);
	}

	RETURN_LONG(rows);
}
/* }}} */

/* {{{ proto object oci_fetch_object( resource stmt ) U
   Fetch a result row as an object */
PHP_FUNCTION(oci_fetch_object)
{
	php_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_OCI_ASSOC | PHP_OCI_RETURN_NULLS, 2);

	if (Z_TYPE_P(return_value) == IS_ARRAY) {
		object_and_properties_init(return_value, ZEND_STANDARD_CLASS_DEF_PTR, Z_ARRVAL_P(return_value));
	}
}
/* }}} */

/* {{{ proto array oci_fetch_row( resource stmt ) U
   Fetch a result row as an enumerated array */
PHP_FUNCTION(oci_fetch_row)
{
	php_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_OCI_NUM | PHP_OCI_RETURN_NULLS, 1);
}
/* }}} */

/* {{{ proto array oci_fetch_assoc( resource stmt ) U
   Fetch a result row as an associative array */
PHP_FUNCTION(oci_fetch_assoc)
{
	php_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_OCI_ASSOC | PHP_OCI_RETURN_NULLS, 1);
}
/* }}} */

/* {{{ proto array oci_fetch_array( resource stmt [, int mode ]) U
   Fetch a result row as an array */
PHP_FUNCTION(oci_fetch_array)
{
	php_oci_fetch_row(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP_OCI_BOTH | PHP_OCI_RETURN_NULLS, 2);
}
/* }}} */

/* {{{ proto bool oci_free_statement(resource stmt) U
   Free all resources associated with a statement */
PHP_FUNCTION(oci_free_statement)
{
	zval *z_statement;
	php_oci_statement *statement;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_statement) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	zend_list_delete(statement->id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_close(resource connection) U
   Disconnect from database */
PHP_FUNCTION(oci_close)
{
	/* oci_close for pconnect (if old_oci_close_semantics not set) would
	 * release the connection back to the client-side session pool (and to the
	 * server-side pool if Database Resident Connection Pool is being used).
	 * Subsequent pconnects in the same script are not guaranteed to get the
	 * same database session.
	 */

	zval *z_connection;
	php_oci_connection *connection;

	if (OCI_G(old_oci_close_semantics)) {
		/* do nothing to keep BC */
		return;
	}
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_connection) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);
	zend_list_delete(connection->rsrc_id);

	ZVAL_NULL(z_connection);
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource oci_new_connect(string user, string pass [, string db]) U
   Connect to an Oracle database and log on. Returns a new session. */
PHP_FUNCTION(oci_new_connect)
{
	php_oci_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 1);
}
/* }}} */

/* {{{ proto resource oci_connect(string user, string pass [, string db [, string charset [, int session_mode ]]) U
   Connect to an Oracle database and log on. Returns a new session. */
PHP_FUNCTION(oci_connect)
{
	php_oci_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 0);
}
/* }}} */

/* {{{ proto resource oci_pconnect(string user, string pass [, string db [, string charset ]]) U
   Connect to an Oracle database using a persistent connection and log on. Returns a new session. */
PHP_FUNCTION(oci_pconnect)
{
	php_oci_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 0);
}
/* }}} */

/* {{{ proto array oci_error([resource stmt|connection|global]) U
   Return the last error of stmt|connection|global. If no error happened returns false. */
PHP_FUNCTION(oci_error)
{
	zval *arg = NULL;
	php_oci_statement *statement;
	php_oci_connection *connection;
	text *errbuf;
	sb4 errcode = 0;
	sword error = OCI_SUCCESS;
	dvoid *errh = NULL;
	ub2 error_offset = 0;
	zstr sqltext = NULL_ZSTR;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &arg) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() > 0) {
		statement = (php_oci_statement *) zend_fetch_resource(&arg TSRMLS_CC, -1, NULL, NULL, 1, le_statement);
	
		if (statement) {
			errh = statement->err;
			error = statement->errcode;

			if (php_oci_fetch_sqltext_offset(statement, &sqltext, &error_offset TSRMLS_CC)) {
				RETURN_FALSE;
			}
			goto go_out;
		}

		connection = (php_oci_connection *) zend_fetch_resource(&arg TSRMLS_CC, -1, NULL, NULL, 1, le_connection);
		if (connection) {
			errh = connection->err;
			error = connection->errcode;
			goto go_out;
		}

		connection = (php_oci_connection *) zend_fetch_resource(&arg TSRMLS_CC, -1, NULL, NULL, 1, le_pconnection);
		if (connection) {
			errh = connection->err;
			error = connection->errcode;
			goto go_out;
		}
	} else {
		errh = OCI_G(err);
		error = OCI_G(errcode);
	}

go_out:
	if (error == OCI_SUCCESS) { /* no error set in the handle */
		RETURN_FALSE;
	}

	if (!errh) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "OCIError: unable to find error handle");
		RETURN_FALSE;
	}

	errcode = php_oci_fetch_errmsg(errh, &errbuf TSRMLS_CC);

	if (errcode) {
		array_init(return_value);
		add_ascii_assoc_long(return_value, "code", errcode);
		add_ascii_assoc_text(return_value, "message", ZSTR((char *)errbuf), 0);
		add_ascii_assoc_long(return_value, "offset", error_offset);
		if (sqltext.v) {
			add_ascii_assoc_text(return_value, "sqltext", sqltext, 1);
		} else {
			add_ascii_assoc_ascii_string(return_value, "sqltext", "", 1);
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int oci_num_fields(resource stmt) U
   Return the number of result columns in a statement */
PHP_FUNCTION(oci_num_fields)
{
	zval *z_statement;
	php_oci_statement *statement;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_statement) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	RETURN_LONG(statement->ncolumns);
}
/* }}} */

/* {{{ proto resource oci_parse(resource connection, string query) U
   Parse a query and return a statement */
PHP_FUNCTION(oci_parse)
{
	zval *z_connection;
	php_oci_connection *connection;
	php_oci_statement *statement;
	zstr query;
	zend_uchar query_type;
	int query_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rt", &z_connection, &query, &query_len, &query_type) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	statement = php_oci_statement_create(connection, query, query_len, query_type TSRMLS_CC);

	if (statement) {
		RETURN_RESOURCE(statement->id);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool oci_set_prefetch(resource stmt, int prefetch_rows) U
  Sets the number of rows to be prefetched on execute to prefetch_rows for stmt */
PHP_FUNCTION(oci_set_prefetch)
{
	zval *z_statement;
	php_oci_statement *statement;
	long size;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &z_statement, &size) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (php_oci_statement_set_prefetch(statement, size TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_password_change(resource connection, string username, string old_password, string new_password) U
  Changes the password of an account */
PHP_FUNCTION(oci_password_change)
{
	zval *z_connection;
	zstr user, pass_old, pass_new, dbname;
	zend_uchar user_type, pass_old_type, pass_new_type, dbname_type;
	int user_len, pass_old_len, pass_new_len, dbname_len;
	php_oci_connection *connection;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "rTTT", &z_connection, &user, &user_len, &user_type, &pass_old, &pass_old_len, &pass_old_type, &pass_new, &pass_new_len, &pass_new_type) == SUCCESS) {
		PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

		if (!user_len) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "username cannot be empty");
			RETURN_FALSE;
		}
		if (!pass_old_len) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "old password cannot be empty");
			RETURN_FALSE;
		}
		if (!pass_new_len) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "new password cannot be empty");
			RETURN_FALSE;
		}

		if (php_oci_password_change(connection, user, user_len, pass_old, pass_old_len, pass_new, pass_new_len, user_type TSRMLS_CC)) {
			RETURN_FALSE;
		}
		RETURN_TRUE;
	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "TTTT", &dbname, &dbname_len, &dbname_type, &user, &user_len, &user_type, &pass_old, &pass_old_len, &pass_old_type, &pass_new, &pass_new_len, &pass_new_type) == SUCCESS) {

		if (!user_len) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "username cannot be empty");
			RETURN_FALSE;
		}
		if (!pass_old_len) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "old password cannot be empty");
			RETURN_FALSE;
		}
		if (!pass_new_len) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "new password cannot be empty");
			RETURN_FALSE;
		}

		connection = php_oci_do_connect_ex(user, user_len, pass_old, pass_old_len, pass_new, pass_new_len, dbname, dbname_len, NULL_ZSTR, OCI_DEFAULT, 0, 0, user_type TSRMLS_CC);
		if (!connection) {
			RETURN_FALSE;
		}
		RETURN_RESOURCE(connection->rsrc_id);
	}
	WRONG_PARAM_COUNT;
}
/* }}} */

/* {{{ proto resource oci_new_cursor(resource connection) U
   Return a new cursor (Statement-Handle) - use this to bind ref-cursors! */
PHP_FUNCTION(oci_new_cursor)
{
	zval *z_connection;
	php_oci_connection *connection;
	php_oci_statement *statement;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_connection) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	statement = php_oci_statement_create(connection, NULL_ZSTR, 0, 0 TSRMLS_CC);
	
	if (statement) {
		RETURN_RESOURCE(statement->id);
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto string oci_result(resource stmt, mixed column) U
   Return a single column of result data */
PHP_FUNCTION(oci_result)
{
	php_oci_out_column *column;
	
	column = php_oci_statement_get_column_helper(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
	if(column) {
		php_oci_column_to_zval(column, return_value, 0 TSRMLS_CC);
	}
	else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string oci_server_version(resource connection) U
   Return a string containing server version information */
PHP_FUNCTION(oci_server_version)
{
	zval *z_connection;
	php_oci_connection *connection;
	zstr version = NULL_ZSTR;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_connection) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);

	if (php_oci_server_get_version(connection, &version TSRMLS_CC)) {
		RETURN_FALSE;
	}
	
	RETURN_TEXT(version, 0);
}
/* }}} */

/* {{{ proto string oci_statement_type(resource stmt) U
   Return the query type of an OCI statement */
PHP_FUNCTION(oci_statement_type)
{
	zval *z_statement;
	php_oci_statement *statement;
	ub2 type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_statement) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (php_oci_statement_get_type(statement, &type TSRMLS_CC)) {
		RETURN_FALSE;
	}

	switch (type) {
		case OCI_STMT_SELECT:
			RETVAL_ASCII_STRING("SELECT", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_UPDATE:
			RETVAL_ASCII_STRING("UPDATE", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_DELETE:
			RETVAL_ASCII_STRING("DELETE", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_INSERT:
			RETVAL_ASCII_STRING("INSERT", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_CREATE:
			RETVAL_ASCII_STRING("CREATE", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_DROP:
			RETVAL_ASCII_STRING("DROP", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_ALTER:
			RETVAL_ASCII_STRING("ALTER", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_BEGIN:
			RETVAL_ASCII_STRING("BEGIN", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_DECLARE:
			RETVAL_ASCII_STRING("DECLARE", ZSTR_DUPLICATE);
			break;
		case OCI_STMT_CALL:
			RETVAL_ASCII_STRING("CALL", ZSTR_DUPLICATE);
			break;
		default:
			RETVAL_ASCII_STRING("UNKNOWN", ZSTR_DUPLICATE);
	}
}
/* }}} */

/* {{{ proto int oci_num_rows(resource stmt) U
   Return the row count of an OCI statement */
PHP_FUNCTION(oci_num_rows)
{
	zval *z_statement;
	php_oci_statement *statement;
	ub4 rowcount;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_statement) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_STATEMENT(z_statement, statement);

	if (php_oci_statement_get_numrows(statement, &rowcount TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_LONG(rowcount);
}
/* }}} */

/* {{{ proto bool oci_free_collection() U
   Deletes collection object*/
PHP_FUNCTION(oci_free_collection)
{
	zval **tmp, *z_collection = getThis();
	php_oci_collection *collection;

	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_collection, oci_coll_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection), "collection", sizeof("collection"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp, collection);

	zend_list_delete(collection->id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_collection_append(string value) U
   Append an object to the collection */
PHP_FUNCTION(oci_collection_append)
{
	zval **tmp, *z_collection = getThis();
	php_oci_collection *collection;
	zstr value;
	int value_len;
	zend_uchar value_type;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "t", &value, &value_len, &value_type) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ot", &z_collection, oci_coll_class_entry_ptr, &value, &value_len, &value_type) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection), "collection", sizeof("collection"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp, collection);

	if (php_oci_collection_append(collection, value, value_len TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string oci_collection_element_get(int ndx) U
   Retrieve the value at collection index ndx */
PHP_FUNCTION(oci_collection_element_get)
{
	zval **tmp, *z_collection = getThis();
	php_oci_collection *collection;
	long element_index;
	zval *value;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &element_index) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &z_collection, oci_coll_class_entry_ptr, &element_index) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection), "collection", sizeof("collection"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp, collection);

	if (php_oci_collection_element_get(collection, element_index, &value TSRMLS_CC)) {
		RETURN_FALSE;
	}
	
	*return_value = *value;
	zval_copy_ctor(return_value);
	zval_ptr_dtor(&value);
}
/* }}} */

/* {{{ proto bool oci_collection_assign(object from) U
   Assign a collection from another existing collection */
PHP_FUNCTION(oci_collection_assign)
{
	zval **tmp_dest, **tmp_from, *z_collection_dest = getThis(), *z_collection_from;
	php_oci_collection *collection_dest, *collection_from;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_collection_from, oci_coll_class_entry_ptr) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &z_collection_dest, oci_coll_class_entry_ptr, &z_collection_from, oci_coll_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection_dest), "collection", sizeof("collection"), (void **)&tmp_dest) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property. The first argument should be valid collection object");
		RETURN_FALSE;
	}

	if (zend_hash_find(Z_OBJPROP_P(z_collection_from), "collection", sizeof("collection"), (void **)&tmp_from) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property. The second argument should be valid collection object");
		RETURN_FALSE;
	}

	PHP_OCI_ZVAL_TO_COLLECTION(*tmp_dest, collection_dest);
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp_from, collection_from);

	if (php_oci_collection_assign(collection_dest, collection_from TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool oci_collection_element_assign(int index, string val) U
   Assign element val to collection at index ndx */
PHP_FUNCTION(oci_collection_element_assign)
{
	zval **tmp, *z_collection = getThis();
	php_oci_collection *collection;
	int value_len;
	long element_index;
	zstr value;
	zend_uchar value_type;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lt", &element_index, &value, &value_len, &value_type) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Olt", &z_collection, oci_coll_class_entry_ptr, &element_index, &value, &value_len, &value_type) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection), "collection", sizeof("collection"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp, collection);

	if (php_oci_collection_element_set(collection, element_index, value, value_len TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int oci_collection_size() U
   Return the size of a collection */
PHP_FUNCTION(oci_collection_size)
{
	zval **tmp, *z_collection = getThis();
	php_oci_collection *collection;
	sb4 size = 0;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_collection, oci_coll_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection), "collection", sizeof("collection"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp, collection);

	if (php_oci_collection_size(collection, &size TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_LONG(size);
}
/* }}} */

/* {{{ proto int oci_collection_max() U
   Return the max value of a collection. For a varray this is the maximum length of the array */
PHP_FUNCTION(oci_collection_max)
{
	zval **tmp, *z_collection = getThis();
	php_oci_collection *collection;
	long max;
	
	if (!getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &z_collection, oci_coll_class_entry_ptr) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection), "collection", sizeof("collection"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp, collection);

	if (php_oci_collection_max(collection, &max TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_LONG(max);
}
/* }}} */

/* {{{ proto bool oci_collection_trim(int num) U
   Trim num elements from the end of a collection */
PHP_FUNCTION(oci_collection_trim)
{
	zval **tmp, *z_collection = getThis();
	php_oci_collection *collection;
	long trim_size;

	if (getThis()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &trim_size) == FAILURE) {
			return;
		}
	}
	else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &z_collection, oci_coll_class_entry_ptr, &trim_size) == FAILURE) {
			return;
		}	
	}
	
	if (zend_hash_find(Z_OBJPROP_P(z_collection), "collection", sizeof("collection"), (void **)&tmp) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find collection property");
		RETURN_FALSE;
	}
	
	PHP_OCI_ZVAL_TO_COLLECTION(*tmp, collection);

	if (php_oci_collection_trim(collection, trim_size TSRMLS_CC)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;	
}
/* }}} */

/* {{{ proto object oci_new_collection(resource connection, string tdo [, string schema]) U
   Initialize a new collection */
PHP_FUNCTION(oci_new_collection)
{
	zval *z_connection;
	php_oci_connection *connection;
	php_oci_collection *collection;
	zstr tdo, schema = NULL_ZSTR;
	int tdo_len, schema_len = 0;
	zend_uchar tdo_type, schema_type = '\0';
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rT|T", &z_connection, &tdo, &tdo_len, &tdo_type, &schema, &schema_len, &schema_type) == FAILURE) {
		return;
	}

	PHP_OCI_ZVAL_TO_CONNECTION(z_connection, connection);
	
	if ( (collection = php_oci_collection_create(connection, tdo, tdo_len, schema, schema_len TSRMLS_CC)) ) {
		object_init_ex(return_value, oci_coll_class_entry_ptr);
		add_property_resource(return_value, "collection", collection->id);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

#endif /* HAVE_OCI8 */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
