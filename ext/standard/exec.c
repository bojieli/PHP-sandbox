/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Rasmus Lerdorf                                               |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#include <stdio.h>
#include "php.h"
#include <ctype.h>
#include "php_string.h"
#include "safe_mode.h"
#include "ext/standard/head.h"
#include "exec.h"
#include "php_globals.h"
#include "SAPI.h"

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

/*
 * If type==0, only last line of output is returned (exec)
 * If type==1, all lines will be printed and last lined returned (system)
 * If type==2, all lines will be saved to given array (exec with &$array)
 * If type==3, output will be printed binary, no lines will be saved or returned (passthru)
 *
 */
static int _Exec(int type, char *cmd, pval *array, pval *return_value)
{
	FILE *fp;
	char *buf, *tmp=NULL;
	int buflen = 0;
	int t, l, ret, output=1;
	int overflow_limit, lcmd, ldir;
	char *b, *c, *d=NULL;
	PLS_FETCH();

	buf = (char*) emalloc(EXEC_INPUT_BUF);
    if (!buf) {
		php_error(E_WARNING, "Unable to emalloc %d bytes for exec buffer", EXEC_INPUT_BUF);
		return -1;
    }
	buflen = EXEC_INPUT_BUF;

	if (PG(safe_mode)) {
		lcmd = strlen(cmd);
		ldir = strlen(PG(safe_mode_exec_dir));
		l = lcmd + ldir + 2;
		overflow_limit = l;
		c = strchr(cmd, ' ');
		if (c) *c = '\0';
		if (strstr(cmd, "..")) {
			php_error(E_WARNING, "No '..' components allowed in path");
			efree(buf);
			return -1;
		}
		d = emalloc(l);
		strcpy(d, PG(safe_mode_exec_dir));
		overflow_limit -= ldir;
		b = strrchr(cmd, '/');
		if (b) {
			strcat(d, b);
			overflow_limit -= strlen(b);
		} else {
			strcat(d, "/");
			strcat(d, cmd);
			overflow_limit-=(strlen(cmd)+1);
		}
		if (c) {
			*c = ' ';
			strncat(d, c, overflow_limit);
		}
		tmp = php_escape_shell_cmd(d);
		efree(d);
		d = tmp;
#if WIN32|WINNT
		fp = popen(d, "rb");
#else
		fp = popen(d, "r");
#endif
		if (!fp) {
			php_error(E_WARNING, "Unable to fork [%s]", d);
			efree(d);
			efree(buf);
			return -1;
		}
	} else { /* not safe_mode */
#if WIN32|WINNT
		fp = popen(cmd, "rb");
#else
		fp = popen(cmd, "r");
#endif
		if (!fp) {
			php_error(E_WARNING, "Unable to fork [%s]", cmd);
			efree(buf);
			return -1;
		}
	}
	buf[0] = '\0';
	if (type==2) {
		if (array->type != IS_ARRAY) {
			pval_destructor(array);
			array_init(array);
		}
	}
	if (type != 3) {
		l=0;
		while ( !feof(fp) || l != 0 ) {
			l = 0;
			/* Read a line or fill the buffer, whichever comes first */
			do {
				if ( buflen <= (l+1) ) {
					buf = erealloc(buf, buflen + EXEC_INPUT_BUF);
					if ( buf == NULL ) {
						php_error(E_WARNING, "Unable to erealloc %d bytes for exec buffer", 
								buflen + EXEC_INPUT_BUF);
						return -1;
					}
					buflen += EXEC_INPUT_BUF;
				}

				if ( fgets(&(buf[l]), buflen - l, fp) == NULL ) {
					/* eof */
					break;
				}
				l += strlen(&(buf[l]));
			} while ( (l > 0) && (buf[l-1] != '\n') );

			if ( feof(fp) && (l == 0) ) {
				break;
			}

		
			if (type == 1) {
				if (output) PUTS(buf);
				sapi_flush();
			}
			else if (type == 2) {
				/* strip trailing whitespaces */	
				l = strlen(buf);
				t = l;
				while (l-- && isspace((int)buf[l]));
				if (l < t) {
					buf[l + 1] = '\0';
				}
				add_next_index_string(array, buf, 1);
			}
		}

		/* strip trailing spaces */
		l = strlen(buf);
		t = l;
		while (l && isspace((int)buf[--l]));
		if (l < t) buf[l + 1] = '\0';

	} else {
		int b, i;

		while ((b = fread(buf, 1, sizeof(buf), fp)) > 0) {
			for (i = 0; i < b; i++)
				if (output) (void)PUTC(buf[i]);
		}
	}

	/* Return last line from the shell command */
	if (PG(magic_quotes_runtime) && type!=3) {
		int len;
		
		tmp = php_addslashes(buf, 0, &len, 0);
		RETVAL_STRINGL(tmp,len,0);
	} else {
		RETVAL_STRING(buf,1);
	}
	
	ret = pclose(fp);
#if HAVE_SYS_WAIT_H
	if (WIFEXITED(ret)) {
		ret = WEXITSTATUS(ret);
	}
#endif

	if (d) efree(d);
	efree(buf);
	return ret;
}

/* {{{ proto int exec(string command [, array output [, int return_value]])
   Execute an external program */
PHP_FUNCTION(exec)
{
	pval **arg1, **arg2, **arg3;
	int arg_count = ARG_COUNT(ht);
	int ret;

	if (arg_count > 3 || zend_get_parameters_ex(arg_count, &arg1,&arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	switch (arg_count) {
		case 1:
			ret = _Exec(0, (*arg1)->value.str.val, NULL,return_value);
			break;
		case 2:
			if (!ParameterPassedByReference(ht,2)) {
				php_error(E_WARNING,"Array argument to exec() not passed by reference");
			}
			ret = _Exec(2, (*arg1)->value.str.val,*arg2,return_value);
			break;
		case 3:
			if (!ParameterPassedByReference(ht,2)) {
				php_error(E_WARNING,"Array argument to exec() not passed by reference");
			}
			if (!ParameterPassedByReference(ht,3)) {
				php_error(E_WARNING,"return_status argument to exec() not passed by reference");
			}
			ret = _Exec(2,(*arg1)->value.str.val,*arg2,return_value);
			(*arg3)->type = IS_LONG;
			(*arg3)->value.lval=ret;
			break;
	}
}
/* }}} */

/* {{{ proto int system(string command [, int return_value])
   Execute an external program and display output */
PHP_FUNCTION(system)
{
	pval **arg1, **arg2;
	int arg_count = ARG_COUNT(ht);
	int ret;

	if (arg_count > 2 || zend_get_parameters_ex(arg_count, &arg1,&arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	switch (arg_count) {
		case 1:
			ret = _Exec(1, (*arg1)->value.str.val, NULL,return_value);
			break;
		case 2:
			if (!ParameterPassedByReference(ht,2)) {
				php_error(E_WARNING,"return_status argument to system() not passed by reference");
			}
			ret = _Exec(1, (*arg1)->value.str.val, NULL,return_value);
			(*arg2)->type = IS_LONG;
			(*arg2)->value.lval=ret;
			break;
	}
}
/* }}} */

/* {{{ proto int passthru(string command [, int return_value])
   Execute an external program and display raw output */
PHP_FUNCTION(passthru)
{
	pval **arg1, **arg2;
	int arg_count = ARG_COUNT(ht);
	int ret;

	if (arg_count > 2 || zend_get_parameters_ex(arg_count, &arg1,&arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	switch (arg_count) {
		case 1:
			ret = _Exec(3, (*arg1)->value.str.val, NULL,return_value);
			break;
		case 2:
			if (!ParameterPassedByReference(ht,2)) {
				php_error(E_WARNING,"return_status argument to system() not passed by reference");
			}
			ret = _Exec(3, (*arg1)->value.str.val, NULL,return_value);
			(*arg2)->type = IS_LONG;
			(*arg2)->value.lval=ret;
			break;
	}
}
/* }}} */

static int php_get_index(char *s, char c)
{
	register int x;

	for (x = 0; s[x]; x++)
		if (s[x] == c)
			return x;

	return -1;
}

/* Escape all chars that could possibly be used to
   break out of a shell command

   This function emalloc's a string and returns the pointer.
   Remember to efree it when done with it.

   *NOT* safe for binary strings
*/
char * php_escape_shell_cmd(char *str) {
	register int x, y, l;
	char *cmd;

	l = strlen(str);
	cmd = emalloc(2 * l + 1);
	strcpy(cmd, str);
	for (x = 0; cmd[x]; x++) {
		if (php_get_index("&;`'\"|*?~<>^()[]{}$\\\x0A\xFF", cmd[x]) != -1) {
			for (y = l + 1; y > x; y--)
				cmd[y] = cmd[y - 1];
			l++;				/* length has been increased */
			cmd[x] = '\\';
			x++;				/* skip the character */
		}
	}
	return cmd;
}

/* {{{ proto escapeshellcmd(string command)
   escape shell metacharacters */
PHP_FUNCTION(escapeshellcmd)
{
	pval **arg1;
	char *cmd = NULL;

	if (zend_get_parameters_ex(1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_string_ex(arg1);
	if ((*arg1)->value.str.len) {
		cmd = php_escape_shell_cmd((*arg1)->value.str.val);
		RETVAL_STRING(cmd, 1);
		efree(cmd);
	}
}
/* }}} */


PHP_FUNCTION(shell_exec)
{
	FILE *in;
	int readbytes,total_readbytes=0,allocated_space;
	pval **cmd;
	PLS_FETCH();

	if (ARG_COUNT(ht)!=1 || zend_get_parameters_ex(1,&cmd)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	if (PG(safe_mode)) {
		php_error(E_WARNING,"Cannot execute using backquotes in safe mode");
		RETURN_FALSE;
	}

	convert_to_string_ex(cmd);
#if WIN32|WINNT
	if ((in=popen((*cmd)->value.str.val,"rt"))==NULL) {
#else
	if ((in=popen((*cmd)->value.str.val,"r"))==NULL) {
#endif
		php_error(E_WARNING,"Unable to execute '%s'",(*cmd)->value.str.val);
	}
	allocated_space = EXEC_INPUT_BUF;
	return_value->value.str.val = (char *) emalloc(allocated_space);
	while (1) {
		readbytes = fread(return_value->value.str.val+total_readbytes,1,EXEC_INPUT_BUF,in);
		if (readbytes<=0) {
			break;
		}
		total_readbytes += readbytes;
		allocated_space = total_readbytes+EXEC_INPUT_BUF;
		return_value->value.str.val = (char *) erealloc(return_value->value.str.val,allocated_space);
	}
	pclose(in);
		
	return_value->value.str.val = erealloc(return_value->value.str.val,total_readbytes+1);
	return_value->value.str.val[total_readbytes]=0;
	return_value->value.str.len = total_readbytes;
	return_value->type = IS_STRING;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
