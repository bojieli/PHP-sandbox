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
   | Authors: Christian Stocker <chregu@php.net>                          |
   |          Rob Richards <rrichards@php.net>                            |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_xsl.h"


/*
* class xsl_xsltprocessor 
*
* URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#
* Since: 
*/

zend_function_entry php_xsl_xsltprocessor_class_functions[] = {
	PHP_FALIAS(import_stylesheet, xsl_xsltprocessor_import_stylesheet, NULL)
	PHP_FALIAS(transform_to_doc, xsl_xsltprocessor_transform_to_doc, NULL)
	PHP_FALIAS(transform_to_uri, xsl_xsltprocessor_transform_to_uri, NULL)
	PHP_FALIAS(transform_to_xml, xsl_xsltprocessor_transform_to_xml, NULL)
	PHP_FALIAS(set_parameter, xsl_xsltprocessor_set_parameter, NULL)
	PHP_FALIAS(get_parameter, xsl_xsltprocessor_get_parameter, NULL)
	PHP_FALIAS(remove_parameter, xsl_xsltprocessor_remove_parameter, NULL)
	{NULL, NULL, NULL}
};

/* {{{ attribute protos, not implemented yet */
/* {{{ php_xsl_xslt_string_to_xpathexpr()
   Translates a string to a XPath Expression */
static char *php_xsl_xslt_string_to_xpathexpr(const char *str TSRMLS_DC)
{
	const xmlChar *string = (const xmlChar *)str;

	xmlChar *value;
	int str_len;
	
	str_len = xmlStrlen(string) + 3;
	
	if (xmlStrchr(string, '"')) {
		if (xmlStrchr(string, '\'')) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot create XPath expression (string contains both quote and double-quotes)");
			return NULL;
		}
		value = (xmlChar*) emalloc (str_len * sizeof(xmlChar) );
		snprintf(value, str_len, "'%s'", string);
	} else {
		value = (xmlChar*) emalloc (str_len * sizeof(xmlChar) );
		snprintf(value, str_len, "\"%s\"", string);
	}
	return (char *) value;
}

/* {{{ php_xsl_xslt_make_params()
   Translates a PHP array to a libxslt parameters array */
static char **php_xsl_xslt_make_params(HashTable *parht, int xpath_params TSRMLS_DC)
{
	
	int parsize;
	zval **value;
	char *xpath_expr, *string_key = NULL;
	ulong num_key;
	char **params = NULL;
	int i = 0;

	parsize = (2 * zend_hash_num_elements(parht) + 1) * sizeof(char *);
	params = (char **)emalloc(parsize);
	memset((char *)params, 0, parsize);

	for (zend_hash_internal_pointer_reset(parht);
		zend_hash_get_current_data(parht, (void **)&value) == SUCCESS;
		zend_hash_move_forward(parht)) {

		if (zend_hash_get_current_key(parht, &string_key, &num_key, 1) != HASH_KEY_IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid argument or parameter array");
			return NULL;
		} else {
			if (Z_TYPE_PP(value) != IS_STRING) {
				SEPARATE_ZVAL(value);
				convert_to_string(*value);
			}

			if (!xpath_params) {
				xpath_expr = php_xsl_xslt_string_to_xpathexpr(Z_STRVAL_PP(value) TSRMLS_CC);
			} else {
				xpath_expr = Z_STRVAL_PP(value);
			}
			if (xpath_expr) {
				params[i++] = string_key;
				params[i++] = xpath_expr;
			}
		}
	}

	params[i++] = NULL;

	return params;
}
/* }}} */
/* {{{ proto xsl_xsltdocucument xsl_xsltprocessor_import_stylesheet(node index);
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#
Since: 
*/
PHP_FUNCTION(xsl_xsltprocessor_import_stylesheet)
{
	zval *id, *docp = NULL;
	xmlDoc *doc;
	xsltStylesheetPtr sheetp, oldsheetp;
	xmlDocPtr newdocp;
	xsl_object *intern;
	dom_object *docobj;
	
	DOM_GET_THIS(id);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &docp) == FAILURE) {
		RETURN_FALSE;
	}
	DOM_GET_OBJ(doc, docp, xmlDocPtr, docobj);
	/* copy the doc, so that it's not accessable from outside
	FIXME: and doubling memory consumption...
	*/
	newdocp = xmlCopyDoc(doc, 1);
	sheetp = xsltParseStylesheetDoc(newdocp);

	if (!sheetp) {
		xmlFreeDoc(newdocp);
		RETURN_FALSE;
	}

	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC); 
	if ((oldsheetp = (xsltStylesheetPtr)intern->ptr)) { 
		/* free wrapper */
		if (((xsltStylesheetPtr) intern->ptr)->_private != NULL) {
			efree(((xsltStylesheetPtr) intern->ptr)->_private);
			((xsltStylesheetPtr) intern->ptr)->_private = NULL;   
		}
		//FIXME: more non-thread safe stuff
		xsltFreeStylesheet((xsltStylesheetPtr) intern->ptr);
		intern->ptr = NULL;
	} 
	php_xsl_set_object(id, sheetp TSRMLS_CC);
}
/* }}} end xsl_xsltprocessor_import_stylesheet */


/* {{{ proto xsl_document xsl_xsltprocessor_transform_to_doc(node doc);
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#
Since: 
*/
PHP_FUNCTION(xsl_xsltprocessor_transform_to_doc)
{
	zval *id, *rv = NULL, *docp = NULL;
	xmlDoc *doc;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	int ret;
	char **params = NULL;
	xsl_object *intern;
	dom_object *docobj;
	
	id = getThis();
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &docp) == FAILURE) {
		RETURN_FALSE;
	}
	DOM_GET_OBJ(doc, docp, xmlDocPtr, docobj);

	if (intern->parameter) {
		params = php_xsl_xslt_make_params(intern->parameter, 0 TSRMLS_CC);
	}
	newdocp = xsltApplyStylesheet(sheetp, doc, (const char**) params);

	if (params) {
		efree(params);
	}

	if (newdocp) {
		DOM_RET_OBJ(rv, (xmlNodePtr) newdocp, &ret, NULL);
	} else {
		RETURN_FALSE;
	}
	
}
/* }}} end xsl_xsltprocessor_transform_to_doc */


/* {{{ proto xsl_ xsl_xsltprocessor_transform_to_uri(node doc, string uri);
*/
PHP_FUNCTION(xsl_xsltprocessor_transform_to_uri)
{
 DOM_NOT_IMPLEMENTED();
}
/* }}} end xsl_xsltprocessor_transform_to_uri */


/* {{{ proto xsl_string xsl_xsltprocessor_transform_to_xml(node doc);
*/
PHP_FUNCTION(xsl_xsltprocessor_transform_to_xml)
{
	zval *id, *docp = NULL;
	xmlDoc *doc;
	xmlDoc *newdocp;
	xsltStylesheetPtr sheetp;
	int ret;
	xmlChar *doc_txt_ptr;
	int doc_txt_len;
	char **params = NULL;
	xsl_object *intern;
	dom_object *docobj;
	
	id = getThis();
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	sheetp = (xsltStylesheetPtr) intern->ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &docp) == FAILURE) {
		RETURN_FALSE;
	}
	DOM_GET_OBJ(doc, docp, xmlDocPtr, docobj);

	if (intern->parameter) {
		params = php_xsl_xslt_make_params(intern->parameter, 0 TSRMLS_CC);
	}
	newdocp = xsltApplyStylesheet(sheetp, doc, (const char**)params);
	
	if (params) {
		efree(params);
	}
	ret = xsltSaveResultToString(&doc_txt_ptr, &doc_txt_len, newdocp, sheetp);

	if (ret < 0) {
		RETURN_FALSE;
	}

	RETVAL_STRINGL(doc_txt_ptr, doc_txt_len, 1);
	xmlFree(doc_txt_ptr);
	xmlFreeDoc(newdocp);
}
/* }}} end xsl_xsltprocessor_transform_to_xml */


/* {{{ proto xsl_ xsl_xsltprocessor_set_parameter(string namespace, string name, string value);
*/
PHP_FUNCTION(xsl_xsltprocessor_set_parameter)
{
 
	zval *id;
	int name_len = 0, namespace_len = 0, value_len = 0;
	char *name, *namespace, *value;
	xsl_object *intern;
	zval *new_string;

	DOM_GET_THIS(id);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &namespace, &namespace_len, &name, &name_len, &value, &value_len) == FAILURE) {
		RETURN_FALSE;
	}
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
   

	MAKE_STD_ZVAL(new_string);
	ZVAL_STRING(new_string, value, 1);
	zend_hash_update(intern->parameter, name, name_len + 1, &new_string, sizeof(zval*), NULL);
}
/* }}} end xsl_xsltprocessor_set_parameter */

/* {{{ proto xsl_ xsl_xsltprocessor_get_parameter(string namespace, string name);
*/
PHP_FUNCTION(xsl_xsltprocessor_get_parameter)
{
	zval *id;
	int name_len = 0, namespace_len = 0;
	char *name, *namespace;
	zval **value;
	xsl_object *intern;

	DOM_GET_THIS(id);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &namespace, &namespace_len, &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	if ( zend_hash_find(intern->parameter, name, name_len + 1,  (void**) &value) == SUCCESS) {
		convert_to_string_ex(value);
		RETVAL_STRING(Z_STRVAL_PP(value),1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} end xsl_xsltprocessor_get_parameter */

/* {{{ proto xsl_ xsl_xsltprocessor_remove_parameter(string namespace, string name);
*/
PHP_FUNCTION(xsl_xsltprocessor_remove_parameter)
{
	zval *id;
	int name_len = 0, namespace_len = 0;
	char *name, *namespace;
	xsl_object *intern;

	DOM_GET_THIS(id);
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &namespace, &namespace_len, &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}
	intern = (xsl_object *)zend_object_store_get_object(id TSRMLS_CC);
	if ( zend_hash_del(intern->parameter, name, name_len + 1) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} end xsl_xsltprocessor_remove_parameter */
