/*
   +----------------------------------------------------------------------+
   | PHP version 4.0													  |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group					  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.01 of the PHP license,	  |
   | that is bundled with this package in the file LICENSE, and is		  |
   | available at through the world-wide-web at							  |
   | http://www.php.net/license/2_01.txt.								  |
   | If you did not receive a copy of the PHP license and are unable to	  |
   | obtain it through the world-wide-web, please send a note to		  |
   | license@php.net so we can mail you a copy immediately.				  |
   +----------------------------------------------------------------------+
   | Authors: Kristian Koehntopp (kris@koehntopp.de)					  |
   +----------------------------------------------------------------------+
 */
 
/* $Id$ */

/* {{{ includes & prototypes */

#include "php.h"
#include "php_recode.h"

#if HAVE_LIBRECODE
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "zend_list.h"

#ifdef HAVE_BROKEN_RECODE
extern char *program_name;
char *program_name = "php";
#endif

/* }}} */

#define SAFE_STRING(s) ((s)?(s):"")

php_recode_globals recode_globals;
extern int le_fp,le_pp;

/* {{{ module stuff */
static zend_function_entry php_recode_functions[] = {
	PHP_FE(recode_string, NULL)
	PHP_FE(recode_file, NULL)
	PHP_FALIAS(recode, recode_string, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry recode_module_entry = {
	"Recode", 
	php_recode_functions, 
	PHP_MINIT(recode), 
	PHP_MSHUTDOWN(recode), 
	NULL,
	NULL, 
	PHP_MINFO(recode), 
	STANDARD_MODULE_PROPERTIES
};

#if APACHE
extern void timeout(int sig);
#endif

PHP_MINIT_FUNCTION(recode)
{
	ReSLS_FETCH();
	ReSG(outer)	  = recode_new_outer(true);
	if (ReSG(outer) == NULL)
		return FAILURE;
	
	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(recode)
{
	ReSLS_FETCH();

	if (ReSG(outer))
		recode_delete_outer(ReSG(outer));

	return SUCCESS;
}


PHP_MINFO_FUNCTION(recode)
{
	ReSLS_FETCH();

	php_printf("<table border=5 width=\"600\">");
	php_info_print_table_header(1, "Module Revision");
	php_info_print_table_row(1, "$Revision$");
	php_printf("</table>\n");
}

/* {{{ proto string recode_string(string request, string str)
   Recode string str according to request string */

PHP_FUNCTION(recode_string)
{
/*	All of this cores in zend_get_parameters_ex()...

	RECODE_REQUEST request = NULL;
	pval **str;
	pval **req;
	char  *r;
	bool   success;
	
	ReSLS_FETCH();
	if (ARG_COUNT(ht) != 2
	 || zend_get_parameters_ex(ht, 2, &req, &str) == FAILURE) {
	 	WRONG_PARAM_COUNT;
	}
	convert_to_string_ex(str);
	convert_to_string_ex(req);
*/

	RECODE_REQUEST request = NULL;
	char *r = NULL;
	pval *str;
	pval *req;
	bool  success;
	
	ReSLS_FETCH();
	if (ARG_COUNT(ht) != 2
	 || zend_get_parameters(ht, 2, &req, &str) == FAILURE) {
	 	WRONG_PARAM_COUNT;
	}
	convert_to_string(str);
	convert_to_string(req);

	request = recode_new_request(ReSG(outer));
	if (request == NULL) {
		php_error(E_WARNING, "Cannot allocate request structure");
		RETURN_FALSE;
	}
	
	success = recode_scan_request(request, req->value.str.val);
	if (!success) {
		php_error(E_WARNING, "Illegal recode request '%s'", req->value.str.val);
		goto error_exit;
	}
	
	r = recode_string(request, str->value.str.val);
	if (!r) {
		php_error(E_WARNING, "Recoding failed.");
		goto error_exit;
	}
	
	RETVAL_STRING(r, 1);
	free(r);
	/* FALLTHROUGH */

error_exit:
	if (request)
		recode_delete_request(request);

	if (!r)	
		RETURN_FALSE;

	return;
}
/* }}} */

/* {{{ proto bool recode_file(string request, int input, int output)
   Recode file input into file output according to request */
PHP_FUNCTION(recode_file)
{
	php_error(E_WARNING, "This has not been ported, yet");
	RETURN_FALSE;
}
/* }}} */


#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
