/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_01.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Paul Panotzki - Bunyip Information Systems                  |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "php.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <errno.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include "microtime.h"
#include "snprintf.h"

#include <stdio.h>
#ifdef HAVE_GETTIMEOFDAY
#ifdef PHP_WIN32
#include "win32/time.h"
#else
#include <sys/time.h>
#endif
#endif

#define NUL  '\0'
#define MICRO_IN_SEC 1000000.00

/* {{{ proto string microtime(void)
   Returns a string containing the current time in seconds and microseconds */
PHP_FUNCTION(microtime)
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tp;
	long sec = 0L;
	double msec = 0.0;
	char ret[100];
	
	if (gettimeofday((struct timeval *) &tp, (NUL)) == 0) {
		msec = (double) (tp.tv_usec / MICRO_IN_SEC);
		sec = tp.tv_sec;
	}
	if (msec >= 1.0) msec -= (long) msec;
	snprintf(ret, 100, "%.8f %ld", msec, sec);
	RETVAL_STRING(ret,1);
#endif
}
/* }}} */


/* {{{ proto array gettimeofday(void)
   returns the current time as array */
PHP_FUNCTION(gettimeofday)
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tp;
	struct timezone tz;
	
	memset(&tp, 0, sizeof(tp));
	memset(&tz, 0, sizeof(tz));
	if(gettimeofday(&tp, &tz) == 0) {
		array_init(return_value);
		add_assoc_long(return_value, "sec", tp.tv_sec);
		add_assoc_long(return_value, "usec", tp.tv_usec);
		add_assoc_long(return_value, "minuteswest", tz.tz_minuteswest);
		add_assoc_long(return_value, "dsttime", tz.tz_dsttime);
		return;
	} else
#endif
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto array getrusage([ int who ])
   returns an array of usage statistics */
PHP_FUNCTION(getrusage)
{
#if HAVE_GETRUSAGE
	struct rusage usg;
	int ac = ARG_COUNT(ht);
	pval **pwho;
	int who = RUSAGE_SELF;

	if(ac == 1 &&
		zend_get_parameters_ex(ac, &pwho) != FAILURE) {
		convert_to_long_ex(pwho);
		if((*pwho)->value.lval == 1)
			who = RUSAGE_CHILDREN;
	}

	memset(&usg, 0, sizeof(usg));
	if(getrusage(who, &usg) == -1) {
		RETURN_FALSE;
	}

	array_init(return_value);
#define PHP3_RUSAGE_PARA(a) \
		add_assoc_long(return_value, #a, usg.a)
#ifndef _OSD_POSIX /* BS2000 has only a few fields in the rusage struct */
	PHP3_RUSAGE_PARA(ru_oublock);
	PHP3_RUSAGE_PARA(ru_inblock);
	PHP3_RUSAGE_PARA(ru_msgsnd);
	PHP3_RUSAGE_PARA(ru_msgrcv);
	PHP3_RUSAGE_PARA(ru_maxrss);
	PHP3_RUSAGE_PARA(ru_ixrss);
	PHP3_RUSAGE_PARA(ru_idrss);
	PHP3_RUSAGE_PARA(ru_minflt);
	PHP3_RUSAGE_PARA(ru_majflt);
	PHP3_RUSAGE_PARA(ru_nsignals);
	PHP3_RUSAGE_PARA(ru_nvcsw);
	PHP3_RUSAGE_PARA(ru_nivcsw);
#endif /*_OSD_POSIX*/
	PHP3_RUSAGE_PARA(ru_utime.tv_usec);
	PHP3_RUSAGE_PARA(ru_utime.tv_sec);
	PHP3_RUSAGE_PARA(ru_stime.tv_usec);
	PHP3_RUSAGE_PARA(ru_stime.tv_sec);
#undef PHP3_RUSAGE_PARA
#endif /* HAVE_GETRUSAGE */
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
