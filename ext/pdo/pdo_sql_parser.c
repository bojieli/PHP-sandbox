/* Generated by re2c 0.5 on Tue May 18 15:32:50 2004 */
#line 1 "/home/rei/php5/ext/pdo/pdo_sql_parser.re"
/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2004 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.0 of the PHP license,       |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_0.txt.                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: George Schlossnagle <george@omniti.com>                      |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php.h"
#include "php_pdo_driver.h"

#define PDO_PARSER_TEXT 1
#define PDO_PARSER_BIND 2
#define PDO_PARSER_EOI 3

#define RET(i) {s->cur = cursor; return i; }

#define YYCTYPE         char
#define YYCURSOR        cursor
#define YYLIMIT         s->lim
#define YYMARKER        s->ptr
#define YYFILL(n)

typedef struct Scanner {
	char 	*lim, *ptr, *cur, *tok;
} Scanner;

static int scan(Scanner *s) 
{
	char *cursor = s->cur;
	std:
		s->tok = cursor;
	#line 51


	{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	  0, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160,   0, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	224, 224, 224, 224, 224, 224, 224, 224, 
	224, 224, 128, 160, 160, 160, 160, 160, 
	160, 224, 224, 224, 224, 224, 224, 224, 
	224, 224, 224, 224, 224, 224, 224, 224, 
	224, 224, 224, 224, 224, 224, 224, 224, 
	224, 224, 224, 160,  32, 160, 160, 160, 
	160, 224, 224, 224, 224, 224, 224, 224, 
	224, 224, 224, 224, 224, 224, 224, 224, 
	224, 224, 224, 224, 224, 224, 224, 224, 
	224, 224, 224, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	160, 160, 160, 160, 160, 160, 160, 160, 
	};
	goto yy0;
yy1:	++YYCURSOR;
yy0:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yybm[0+yych] & 32)	goto yy5;
	if(yych <= '\000')	goto yy8;
	if(yych >= ':')	goto yy4;
yy2:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych >= '\001')	goto yy14;
yy3:
#line 56
	{ RET(PDO_PARSER_TEXT); }
yy4:	yych = *++YYCURSOR;
	if(yybm[0+yych] & 64)	goto yy10;
	goto yy3;
yy5:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy6:	if(yybm[0+yych] & 32)	goto yy5;
yy7:
#line 57
	{ RET(PDO_PARSER_TEXT); }
yy8:	yych = *++YYCURSOR;
yy9:
#line 58
	{ RET(PDO_PARSER_EOI); }
yy10:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy11:	if(yybm[0+yych] & 64)	goto yy10;
yy12:
#line 55
	{ RET(PDO_PARSER_BIND); }
yy13:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy14:	if(yybm[0+yych] & 128)	goto yy13;
	if(yych <= '\000')	goto yy15;
	if(yych <= '[')	goto yy17;
	goto yy16;
yy15:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy3;
	}
yy16:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych == '"')	goto yy13;
	goto yy15;
yy17:	yych = *++YYCURSOR;
yy18:
#line 54
	{ RET(PDO_PARSER_TEXT); }
}
#line 59
	
}

int pdo_parse_params(pdo_stmt_t *stmt, char *inquery, int inquery_len, char **outquery, 
		int *outquery_len TSRMLS_DC)
{
	Scanner s;
	char *ptr;
	int t;
	int bindno = 0;
	int newbuffer_len;
	int padding;
	HashTable *params = stmt->bound_params;
	struct pdo_bound_param_data *param;

	/* allocate buffer for query with expanded binds, ptr is our writing pointer */
	newbuffer_len = inquery_len;

	/* calculate the possible padding factor due to quoting */
	if(stmt->dbh->max_escaped_char_length) {
		padding = stmt->dbh->max_escaped_char_length;
	} else {
		padding = 3;
	}
	if(params) {
		zend_hash_internal_pointer_reset(params);
		while (SUCCESS == zend_hash_get_current_data(params, (void**)&param)) {
			if(param->parameter) {
				convert_to_string(param->parameter);
				/* accomodate a string that needs to be fully quoted
                   bind placeholders are at least 2 characters, so
                   the accomodate their own "'s
                */
				newbuffer_len += padding * Z_STRLEN_P(param->parameter);
			}
			zend_hash_move_forward(params);
		}
	}
	*outquery = (char *) emalloc(newbuffer_len + 1);
	*outquery_len = 0;

	ptr = *outquery;
	s.cur = inquery;
	s.lim = inquery + inquery_len;
	while((t = scan(&s)) != PDO_PARSER_EOI) {
		if(t == PDO_PARSER_TEXT) {
			memcpy(ptr, s.tok, s.cur - s.tok);
			ptr += (s.cur - s.tok);
			*outquery_len += (s.cur - s.tok);
		}
		else if(t == PDO_PARSER_BIND) {
			if(!params) { 
				/* error */
				efree(*outquery);
				return 0;
			}
			/* lookup bind first via hash and then index */
			if((SUCCESS == zend_hash_find(params, s.tok+1, s.cur-s.tok,(void **)&param))  
			    ||
			   (SUCCESS == zend_hash_index_find(params, bindno, (void **)&param))) 
			{
				char *quotedstr;
				int quotedstrlen;
				/* currently everything is a string here */
				
				/* quote the bind value if necessary */
				if(stmt->dbh->methods->quoter(stmt->dbh, Z_STRVAL_P(param->parameter), 
					Z_STRLEN_P(param->parameter), &quotedstr, &quotedstrlen TSRMLS_CC))
				{
					memcpy(ptr, quotedstr, quotedstrlen);
					ptr += quotedstrlen;
					*outquery_len += quotedstrlen;
					efree(quotedstr);
				} else {
					memcpy(ptr, Z_STRVAL_P(param->parameter), Z_STRLEN_P(param->parameter));
					ptr += Z_STRLEN_P(param->parameter);
					*outquery_len += (Z_STRLEN_P(param->parameter));
				}
			}
			else {
				/* error and cleanup */
				efree(*outquery);
				return 0;
			}
			bindno++;
		}	
	}	
	*ptr = '\0';
	return 1;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker ft=c
 * vim<600: noet sw=4 ts=4
 */
