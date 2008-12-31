/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
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
   | Author: Stig S�ther Bakken <ssb@php.net>                             |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"

#ifdef HAVE_SYSLOG_H
#include "php_ini.h"
#include "zend_globals.h"

#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <errno.h>

#include <stdio.h>
#include "basic_functions.h"
#include "php_ext_syslog.h"

static void start_syslog(TSRMLS_D);

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(syslog)
{
	/* error levels */
	REGISTER_LONG_CONSTANT("LOG_EMERG", LOG_EMERG, CONST_CS | CONST_PERSISTENT); /* system unusable */
	REGISTER_LONG_CONSTANT("LOG_ALERT", LOG_ALERT, CONST_CS | CONST_PERSISTENT); /* immediate action required */
	REGISTER_LONG_CONSTANT("LOG_CRIT", LOG_CRIT, CONST_CS | CONST_PERSISTENT); /* critical conditions */
	REGISTER_LONG_CONSTANT("LOG_ERR", LOG_ERR, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LOG_WARNING", LOG_WARNING, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_NOTICE", LOG_NOTICE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_INFO", LOG_INFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_DEBUG", LOG_DEBUG, CONST_CS | CONST_PERSISTENT);
	/* facility: type of program logging the message */
	REGISTER_LONG_CONSTANT("LOG_KERN", LOG_KERN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_USER", LOG_USER, CONST_CS | CONST_PERSISTENT); /* generic user level */
	REGISTER_LONG_CONSTANT("LOG_MAIL", LOG_MAIL, CONST_CS | CONST_PERSISTENT); /* log to email */
	REGISTER_LONG_CONSTANT("LOG_DAEMON", LOG_DAEMON, CONST_CS | CONST_PERSISTENT); /* other system daemons */
	REGISTER_LONG_CONSTANT("LOG_AUTH", LOG_AUTH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_SYSLOG", LOG_SYSLOG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LPR", LOG_LPR, CONST_CS | CONST_PERSISTENT);
#ifdef LOG_NEWS
	/* No LOG_NEWS on HP-UX */
	REGISTER_LONG_CONSTANT("LOG_NEWS", LOG_NEWS, CONST_CS | CONST_PERSISTENT); /* usenet new */
#endif
#ifdef LOG_UUCP
	/* No LOG_UUCP on HP-UX */
	REGISTER_LONG_CONSTANT("LOG_UUCP", LOG_UUCP, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef LOG_CRON
	/* apparently some systems don't have this one */
	REGISTER_LONG_CONSTANT("LOG_CRON", LOG_CRON, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef LOG_AUTHPRIV
	/* AIX doesn't have LOG_AUTHPRIV */
	REGISTER_LONG_CONSTANT("LOG_AUTHPRIV", LOG_AUTHPRIV, CONST_CS | CONST_PERSISTENT);
#endif
#ifndef PHP_WIN32
	REGISTER_LONG_CONSTANT("LOG_LOCAL0", LOG_LOCAL0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LOCAL1", LOG_LOCAL1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LOCAL2", LOG_LOCAL2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LOCAL3", LOG_LOCAL3, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LOCAL4", LOG_LOCAL4, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LOCAL5", LOG_LOCAL5, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LOCAL6", LOG_LOCAL6, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_LOCAL7", LOG_LOCAL7, CONST_CS | CONST_PERSISTENT);
#endif
	/* options */
	REGISTER_LONG_CONSTANT("LOG_PID", LOG_PID, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_CONS", LOG_CONS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_ODELAY", LOG_ODELAY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LOG_NDELAY", LOG_NDELAY, CONST_CS | CONST_PERSISTENT);
#ifdef LOG_NOWAIT
	REGISTER_LONG_CONSTANT("LOG_NOWAIT", LOG_NOWAIT, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef LOG_PERROR
	/* AIX doesn't have LOG_PERROR */
	REGISTER_LONG_CONSTANT("LOG_PERROR", LOG_PERROR, CONST_CS | CONST_PERSISTENT); /*log to stderr*/
#endif
	BG(syslog_device)=NULL;

	return SUCCESS;
}
/* }}} */

PHP_RINIT_FUNCTION(syslog) /* {{{ */
{
	BG(syslog_device) = NULL;
	return SUCCESS;
}
/* }}} */

#ifdef PHP_WIN32
PHP_RSHUTDOWN_FUNCTION(syslog) /* {{{ */
{
	closelog();
	return SUCCESS;
}
/* }}} */
#endif

PHP_MSHUTDOWN_FUNCTION(syslog) /* {{{ */
{
	if (BG(syslog_device)) {
		free(BG(syslog_device));
		BG(syslog_device) = NULL;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ proto bool openlog(string ident, int option, int facility) U
   Open connection to system logger */
/*
   ** OpenLog("nettopp", $LOG_PID, $LOG_LOCAL1);
   ** Syslog($LOG_EMERG, "help me!")
   ** CloseLog();
 */
PHP_FUNCTION(openlog)
{
	char *ident;
	long option, facility;
	int ident_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &ident,
							  &ident_len, &option, &facility) == FAILURE) {
		return;
	}
	if (BG(syslog_device)) {
		free(BG(syslog_device));
	}
	BG(syslog_device) = zend_strndup(ident, ident_len);
	openlog(BG(syslog_device), option, facility);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool closelog(void) U
   Close connection to system logger */
PHP_FUNCTION(closelog)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	closelog();
	if (BG(syslog_device)) {
		free(BG(syslog_device));
		BG(syslog_device)=NULL;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool syslog(int priority, string message) U
   Generate a system log message */
PHP_FUNCTION(syslog)
{
	long priority;
	char *message;
	int message_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &priority,
							  &message, &message_len) == FAILURE) {
		return;
	}

	php_syslog(priority, "%s", message);
	RETURN_TRUE;
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
