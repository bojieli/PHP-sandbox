/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: 								  |
   |  Initial version     by  Alex Barkov <bar@izhcom.ru>                 |
   |                      and Ramil Kalimullin <ram@izhcom.ru>            |
   |  Further development by  Sergey Kartashoff <gluke@biosys.net>        |
   +----------------------------------------------------------------------+
 */
 
/* $Id: php_mnogo.c,v 0.3 2001/01/27 15:30:00 */

#include "php.h"
#include "php_mnogo.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/info.h"
#include "php_globals.h"

#ifdef HAVE_MNOGOSEARCH

#define UDM_FIELD_URLID		1
#define UDM_FIELD_URL		2
#define UDM_FIELD_CONTENT	4	
#define UDM_FIELD_TITLE		8
#define UDM_FIELD_KEYWORDS	16
#define UDM_FIELD_DESC		32
#define UDM_FIELD_TEXT		64
#define UDM_FIELD_SIZE		128
#define UDM_FIELD_SCORE		256
#define UDM_FIELD_MODIFIED	512

#define UDM_PARAM_PAGE_SIZE	1
#define UDM_PARAM_PAGE_NUM	2
#define UDM_PARAM_SEARCH_MODE	4
#define UDM_PARAM_CHARSET	8
#define UDM_PARAM_NUM_ROWS	16
#define UDM_PARAM_FOUND		32

/* True globals, no need for thread safety */
static int le_link,le_res;

#include <udmsearch.h>

function_entry mnogosearch_functions[] = {
	PHP_FE(udm_alloc_agent,		NULL)
	PHP_FE(udm_set_agent_param,	NULL)
	PHP_FE(udm_free_agent,		NULL)

	PHP_FE(udm_errno,		NULL)
	PHP_FE(udm_error,		NULL)

	PHP_FE(udm_find,		NULL)
	PHP_FE(udm_free_res,		NULL)
	PHP_FE(udm_get_res_field,	NULL)
	PHP_FE(udm_get_res_param,	NULL)

	{NULL, NULL, NULL}
};


zend_module_entry mnogosearch_module_entry = {
	"mnogosearch", 
	mnogosearch_functions, 
	PHP_MINIT(mnogosearch), 
	PHP_MSHUTDOWN(mnogosearch), 
	PHP_RINIT(mnogosearch), 
	NULL,
	PHP_MINFO(mnogosearch), 
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_MNOGOSEARCH
ZEND_GET_MODULE(mnogosearch)
#endif

static void _free_udm_agent(zend_rsrc_list_entry *rsrc){
	UDM_AGENT * Agent = (UDM_AGENT *)rsrc->ptr;
	UdmFreeAgent(Agent);
}

static void _free_udm_res(zend_rsrc_list_entry *rsrc){
	UDM_RESULT * Res = (UDM_RESULT *)rsrc->ptr;
	UdmFreeResult(Res);	
}

DLEXPORT PHP_MINIT_FUNCTION(mnogosearch)
{
	UdmInit();
	le_link = zend_register_list_destructors_ex(_free_udm_agent,NULL,"mnogosearch agent",module_number);
	le_res = zend_register_list_destructors_ex(_free_udm_res,NULL,"mnogosearch result",module_number);

	REGISTER_LONG_CONSTANT("UDM_FIELD_URLID",	UDM_FIELD_URLID,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_URL",	UDM_FIELD_URL,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_CONTENT",UDM_FIELD_CONTENT,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_TITLE",	UDM_FIELD_TITLE,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_KEYWORDS",UDM_FIELD_KEYWORDS,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_DESC",	UDM_FIELD_DESC,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_TEXT",	UDM_FIELD_TEXT,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_SIZE",	UDM_FIELD_SIZE,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_SCORE",	UDM_FIELD_SCORE,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_FIELD_MODIFIED",UDM_FIELD_MODIFIED,CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("UDM_PARAM_PAGE_SIZE",UDM_PARAM_PAGE_SIZE,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_PARAM_PAGE_NUM",UDM_PARAM_PAGE_NUM,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_PARAM_SEARCH_MODE",UDM_PARAM_SEARCH_MODE,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_PARAM_CHARSET",UDM_PARAM_CHARSET,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_PARAM_FOUND",UDM_PARAM_FOUND,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_PARAM_NUM_ROWS",UDM_PARAM_NUM_ROWS,CONST_CS | CONST_PERSISTENT);


	REGISTER_LONG_CONSTANT("UDM_MODE_ALL",UDM_MODE_ALL,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_MODE_ANY",UDM_MODE_ANY,CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("UDM_MODE_BOOL",UDM_MODE_BOOL,CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}


DLEXPORT PHP_MSHUTDOWN_FUNCTION(mnogosearch)
{
	return SUCCESS;
}


DLEXPORT PHP_RINIT_FUNCTION(mnogosearch)
{
	return SUCCESS;
}


DLEXPORT PHP_MINFO_FUNCTION(mnogosearch)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "mnoGoSearch Support", "enabled" );
	php_info_print_table_end();
}


/* {{{ proto int mnogosearch_alloc_agent(string dbaddr [, string dbmode])
   Allocate mnoGoSearch session */
DLEXPORT PHP_FUNCTION(udm_alloc_agent)
{
	switch(ZEND_NUM_ARGS()){

		case 1: {
				pval **yydbaddr;
				char *dbaddr;
				UDM_ENV   * Env;
				UDM_AGENT * Agent;
				
				if(zend_get_parameters_ex(1,&yydbaddr)==FAILURE){
					RETURN_FALSE;
				}
				convert_to_string_ex(yydbaddr);
				dbaddr = (*yydbaddr)->value.str.val;
				
				Env=UdmAllocEnv();
				UdmEnvSetDBAddr(Env,dbaddr);
				Agent=UdmAllocAgent(Env,0,UDM_OPEN_MODE_READ);
				
				ZEND_REGISTER_RESOURCE(return_value,Agent,le_link);
			}
			break;
			
		case 2: {
				pval **yydbaddr;
				pval **yydbmode;
				char *dbaddr;
				char *dbmode;
				UDM_ENV   * Env;
				UDM_AGENT * Agent;
				
				if(zend_get_parameters_ex(2,&yydbaddr,&yydbmode)==FAILURE){
					RETURN_FALSE;
				}
				convert_to_string_ex(yydbaddr);
				convert_to_string_ex(yydbmode);
				dbaddr = (*yydbaddr)->value.str.val;
				dbmode = (*yydbmode)->value.str.val;
				
				Env=UdmAllocEnv();				
				UdmEnvSetDBAddr(Env,dbaddr);
				UdmEnvSetDBMode(Env,dbmode);				
				Agent=UdmAllocAgent(Env,0,UDM_OPEN_MODE_READ);				
				
				ZEND_REGISTER_RESOURCE(return_value,Agent,le_link);
			}
			break;
			
		default:
			WRONG_PARAM_COUNT;
			break;
	}
}
/* }}} */


/* {{{ proto int udm_set_agent_param(agent,var,val)
   Set mnoGoSearch agent session parameters */
DLEXPORT PHP_FUNCTION(udm_set_agent_param)
{
	pval **yyagent, **yyvar, **yyval;
	char *val;
	int var;
	UDM_AGENT * Agent;

	switch(ZEND_NUM_ARGS()){
	
		case 3: {
				if(zend_get_parameters_ex(3,&yyagent,&yyvar,&yyval)==FAILURE){
					RETURN_FALSE;
				}
				convert_to_long_ex(yyvar);
				convert_to_string_ex(yyval);
				ZEND_FETCH_RESOURCE(Agent, UDM_AGENT *, yyagent, -1, "mnoGoSearch-agent", le_link);
				var = (*yyvar)->value.lval;
				val = (*yyval)->value.str.val;
			}
			break;
			
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	switch(var){
		case UDM_PARAM_PAGE_SIZE: {
				Agent->page_size=atoi(val);
				if(Agent->page_size<1)Agent->page_size=20;
			}
			break;
		case UDM_PARAM_PAGE_NUM: {
				Agent->page_number=atoi(val);
				if(Agent->page_number<0)Agent->page_number=0;
			}
			break;
		case UDM_PARAM_SEARCH_MODE: {
				switch (atoi(val)){
					case UDM_MODE_ALL:
						Agent->search_mode=UDM_MODE_ALL;
						break;
					case UDM_MODE_ANY:
						Agent->search_mode=UDM_MODE_ANY;
						break;
					case UDM_MODE_BOOL: 
						Agent->search_mode=UDM_MODE_BOOL;
						break;
					default:
						RETURN_STRING("<Udm_Set_Agent_Param: Unknown search mode>",1);
						break;
				}
			}
			break;
		default:
			RETURN_STRING("<Udm_Set_Agent_Param: Unknown agent parameter>",1);
			break;
	}
}
/* }}} */



/* {{{ proto int udm_free_agent(int agent_identifier)
   Free mnoGoSearch session */
DLEXPORT PHP_FUNCTION(udm_free_agent)
{
	pval ** yyagent;
	UDM_RESULT * Agent;
	switch(ZEND_NUM_ARGS()){
		case 1: {
				if (zend_get_parameters_ex(1, &yyagent)==FAILURE) {
					RETURN_FALSE;
				}
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE(Agent, UDM_RESULT *, yyagent, -1, "mnoGoSearch-agent", le_link);
	zend_list_delete((*yyagent)->value.lval);
}
/* }}} */


/* {{{ proto int udm_find(int agent_identifier,string query)
   perform search */
DLEXPORT PHP_FUNCTION(udm_find)
{
	pval ** yyquery, ** yyagent;
	UDM_RESULT * Res;
	UDM_AGENT * Agent;
	int id=-1;

	switch(ZEND_NUM_ARGS()){
		case 2: {
				if (zend_get_parameters_ex(2, &yyagent,&yyquery)==FAILURE) {
					RETURN_FALSE;
				}
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE(Agent, UDM_AGENT *, yyagent, id, "mnoGoSearch-Agent", le_link);
	convert_to_string_ex(yyquery);
	Res=UdmFind(Agent,(*yyquery)->value.str.val);
	ZEND_REGISTER_RESOURCE(return_value,Res,le_res);
}
/* }}} */


/* {{{ proto int udm_get_res_field(int res_identifier,int row_num,const int field_name)
   Fetch mnoGoSearch result field */
DLEXPORT PHP_FUNCTION(udm_get_res_field){
	pval **yyres, **yyrow_num, **yyfield_name;

	UDM_RESULT * Res;
	int row,field;
	
	switch(ZEND_NUM_ARGS()){
		case 3: {
				if (zend_get_parameters_ex(3, &yyres,&yyrow_num,&yyfield_name)==FAILURE){
					RETURN_FALSE;
				}
				convert_to_string_ex(yyrow_num);
				convert_to_string_ex(yyfield_name);
				field=atoi((*yyfield_name)->value.str.val);
				row=atoi((*yyrow_num)->value.str.val);
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE(Res, UDM_RESULT *, yyres, -1, "mnoGoSearch-Result", le_res);
	if(row<Res->num_rows){
		switch(field){
			case UDM_FIELD_URL: 		RETURN_STRING((Res->Doc[row].url),1);break;
			case UDM_FIELD_CONTENT: 	RETURN_STRING((Res->Doc[row].content_type),1);break;
			case UDM_FIELD_TITLE:		RETURN_STRING((Res->Doc[row].title),1);break;
			case UDM_FIELD_KEYWORDS:	RETURN_STRING((Res->Doc[row].keywords),1);break;
			case UDM_FIELD_DESC:		RETURN_STRING((Res->Doc[row].description),1);break;
			case UDM_FIELD_TEXT:		RETURN_STRING((Res->Doc[row].text),1);break;
			case UDM_FIELD_SIZE:		RETURN_LONG((Res->Doc[row].size));break;
			case UDM_FIELD_URLID:		RETURN_LONG((Res->Doc[row].url_id));break;
			case UDM_FIELD_SCORE:		RETURN_LONG((Res->Doc[row].rating));break;
			case UDM_FIELD_MODIFIED:	RETURN_LONG((Res->Doc[row].last_mod_time));break;
			default: 
				RETURN_STRING("<Udm_Get_Res_Field: Unknown mnoGoSearch field name>",1);break;
		}
	}else{
		RETURN_STRING("<Udm_Get_Res_Field: row number too large>",1);
	}
}
/* }}} */


/* {{{ proto int udm_free_res(int res_identifier)
    mnoGoSearch free result */
DLEXPORT PHP_FUNCTION(udm_free_res)
{
	pval ** yyres;
	UDM_RESULT * Res;
	switch(ZEND_NUM_ARGS()){
		case 1: {
				if (zend_get_parameters_ex(1, &yyres)==FAILURE) {
					RETURN_FALSE;
				}
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE(Res, UDM_RESULT *, yyres, -1, "mnoGoSearch-Result", le_res);
	zend_list_delete((*yyres)->value.lval);

}
/* }}} */


/* {{{ proto int udm_error(int agent_identifier)
    mnoGoSearch error message */
DLEXPORT PHP_FUNCTION(udm_error)
{
	pval ** yyagent;
	UDM_AGENT * Agent;
	
	switch(ZEND_NUM_ARGS()){
		case 1: {
				if (zend_get_parameters_ex(1, &yyagent)==FAILURE) {
					RETURN_FALSE;
				}
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE(Agent, UDM_AGENT *, yyagent, -1, "mnoGoSearch-Agent", le_link);
	RETURN_STRING(UdmDBErrorMsg(Agent->db),1);
}
/* }}} */

/* {{{ proto int udm_errno(int agent_identifier)
    mnoGoSearch error number */
DLEXPORT PHP_FUNCTION(udm_errno)
{
	pval ** yyagent;
	UDM_AGENT * Agent;
	switch(ZEND_NUM_ARGS()){
		case 1: {
				if (zend_get_parameters_ex(1, &yyagent)==FAILURE) {
					RETURN_FALSE;
				}
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE(Agent, UDM_AGENT *, yyagent, -1, "mnoGoSearch-Agent", le_link);
	RETURN_LONG(UdmDBErrorCode(Agent->db));
}
/* }}} */


/* {{{ proto int udm_get_res_param(int res_identifier, const int param_id)
    mnoGoSearch result parameters */
DLEXPORT PHP_FUNCTION(udm_get_res_param)
{
	pval ** yyres, ** yyparam;
	int param;
	UDM_RESULT * Res;
	switch(ZEND_NUM_ARGS()){
		case 2: {
				if (zend_get_parameters_ex(2, &yyres, &yyparam)==FAILURE) {
					RETURN_FALSE;
				}
				convert_to_long_ex(yyparam);
				param=((*yyparam)->value.lval);
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	ZEND_FETCH_RESOURCE(Res, UDM_RESULT *, yyres, -1, "mnoGoSearch-Result", le_res);
	switch(param){
		case UDM_PARAM_NUM_ROWS: RETURN_LONG(Res->num_rows);break;
		case UDM_PARAM_FOUND:	 RETURN_LONG(Res->total_found);break;
		default:
			/* FIXME: unknown parameter */
			RETURN_STRING("<Udm_Get_Res_Param: Unknown mnoGoSearch param name>",1);
			break;
	}
}
/* }}} */

#endif


/*
 * Local variables:
 * End:
 */

