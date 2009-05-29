/*
  +----------------------------------------------------------------------+
  | PHP Version 6                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Georg Richter <georg@mysql.com>                             |
  |          Andrey Hristov <andrey@mysql.com>                           |
  |          Ulf Wendel <uwendel@mysql.com>                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef MYSQLND_STATISTICS_H
#define MYSQLND_STATISTICS_H


extern MYSQLND_STATS *mysqlnd_global_stats;

typedef struct st_mysqlnd_string
{
	char	*s;
	size_t	l;
} MYSQLND_STRING;

extern const MYSQLND_STRING mysqlnd_stats_values_names[];

#ifdef ZTS

#define MYSQLND_INC_GLOBAL_STATISTIC(statistic) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		DBG_INF_FMT("Global stat increase [%s]", mysqlnd_stats_values_names[(statistic)]); \
 		tsrm_mutex_lock(mysqlnd_global_stats->LOCK_access); \
		mysqlnd_global_stats->values[(statistic)]++; \
		tsrm_mutex_unlock(mysqlnd_global_stats->LOCK_access); \
	}\
 }

#define MYSQLND_DEC_CONN_STATISTIC(conn_stats, statistic) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		DBG_INF_FMT("Global&conn stat decrease [%s]", mysqlnd_stats_values_names[(statistic)]); \
 		tsrm_mutex_lock(mysqlnd_global_stats->LOCK_access); \
		mysqlnd_global_stats->values[(statistic)]--; \
		tsrm_mutex_unlock(mysqlnd_global_stats->LOCK_access); \
		if ((conn_stats)) { \
			((MYSQLND_STATS *) conn_stats)->values[(statistic)]--; \
		} \
	}\
 }

#define MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(statistic1, value1, statistic2, value2) \
 { \
 	if (MYSQLND_G(collect_statistics)) { \
		uint64_t v1 = (uint64_t) (value1); \
		uint64_t v2 = (uint64_t) (value2); \
		DBG_INF_FMT("Global stat increase [%s] [%s]", mysqlnd_stats_values_names[(statistic1)], mysqlnd_stats_values_names[(statistic2)]); \
										\
 		tsrm_mutex_lock(mysqlnd_global_stats->LOCK_access); \
		if (statistic1 != STAT_LAST) mysqlnd_global_stats->values[(statistic1)]+= v1; \
		if (statistic2 != STAT_LAST) mysqlnd_global_stats->values[(statistic2)]+= v2; \
		tsrm_mutex_unlock(mysqlnd_global_stats->LOCK_access); \
	}\
 }

#define MYSQLND_INC_CONN_STATISTIC(conn_stats, statistic) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		DBG_INF_FMT("Global&Conn stat increase [%s]", mysqlnd_stats_values_names[(statistic)]); \
 		tsrm_mutex_lock(mysqlnd_global_stats->LOCK_access); \
		mysqlnd_global_stats->values[(statistic)]++; \
		tsrm_mutex_unlock(mysqlnd_global_stats->LOCK_access); \
		if ((conn_stats)) { \
			((MYSQLND_STATS *) conn_stats)->values[(statistic)]++; \
		} \
	}\
 }

#define MYSQLND_INC_CONN_STATISTIC_W_VALUE(conn_stats, statistic, value) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		uint64_t v = (uint64_t) (value); \
		DBG_INF_FMT("Global&Conn stat increase w value [%s]", mysqlnd_stats_values_names[(statistic)]); \
		tsrm_mutex_lock(mysqlnd_global_stats->LOCK_access); \
		mysqlnd_global_stats->values[(statistic)] += v; \
		tsrm_mutex_unlock(mysqlnd_global_stats->LOCK_access); \
		if ((conn_stats)) { \
			((MYSQLND_STATS *) conn_stats)->values[(statistic)]+= v; \
		} \
	}\
 }

#define MYSQLND_INC_CONN_STATISTIC_W_VALUE2(conn_stats, statistic1, value1, statistic2, value2) \
 { \
 	if (MYSQLND_G(collect_statistics)) { \
		uint64_t v1 = (uint64_t) (value1); \
		uint64_t v2 = (uint64_t) (value2); \
								 \
		tsrm_mutex_lock(mysqlnd_global_stats->LOCK_access); \
		if (statistic1 != STAT_LAST) mysqlnd_global_stats->values[(statistic1)]+= v1; \
		if (statistic2 != STAT_LAST) mysqlnd_global_stats->values[(statistic2)]+= v2; \
		tsrm_mutex_unlock(mysqlnd_global_stats->LOCK_access); \
		if ((conn_stats)) { \
			if (statistic1 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic1)]+= v1; \
			if (statistic2 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic2)]+= v2; \
		} \
	} \
 }


#define MYSQLND_INC_CONN_STATISTIC_W_VALUE3(conn_stats, statistic1, value1, statistic2, value2, statistic3, value3) \
 { \
 	if (MYSQLND_G(collect_statistics)) { \
		uint64_t v1 = (uint64_t) (value1); \
		uint64_t v2 = (uint64_t) (value2); \
		uint64_t v3 = (uint64_t) (value3); \
								 		\
		tsrm_mutex_lock(mysqlnd_global_stats->LOCK_access); \
		if (statistic1 != STAT_LAST) mysqlnd_global_stats->values[(statistic1)]+= v1; \
		if (statistic2 != STAT_LAST) mysqlnd_global_stats->values[(statistic2)]+= v2; \
		if (statistic3 != STAT_LAST) mysqlnd_global_stats->values[(statistic3)]+= v3; \
		tsrm_mutex_unlock(mysqlnd_global_stats->LOCK_access); \
		if ((conn_stats)) { \
			if (statistic1 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic1)]+= v1; \
			if (statistic2 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic2)]+= v2; \
			if (statistic3 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic3)]+= v3; \
		} \
	} \
 }


#else /* NON-ZTS */

#define MYSQLND_INC_GLOBAL_STATISTIC(statistic) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		DBG_INF_FMT("Global stat increase [%s]", mysqlnd_stats_values_names[(statistic)]); \
		mysqlnd_global_stats->values[(statistic)]++; \
	} \
 }


#define MYSQLND_DEC_CONN_STATISTIC(conn_stats, statistic) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		DBG_INF_FMT("Global&Conn stat decrease [%s]", mysqlnd_stats_values_names[(statistic)]); \
		mysqlnd_global_stats->values[(statistic)]--; \
		if ((conn_stats)) { \
			((MYSQLND_STATS *) conn_stats)->values[(statistic)]--; \
		} \
	} \
 }

#define MYSQLND_INC_GLOBAL_STATISTIC_W_VALUE2(statistic1, value1, statistic2, value2) \
 { \
 	if (MYSQLND_G(collect_statistics)) { \
		uint64_t v1 = (uint64_t) (value1); \
		uint64_t v2 = (uint64_t) (value2); \
		DBG_INF_FMT("Global stat increase [%s] [%s]", mysqlnd_stats_values_names[(statistic1)], mysqlnd_stats_values_names[(statistic2)]); \
			\
		if (statistic1 != STAT_LAST) mysqlnd_global_stats->values[(statistic1)]+= v1; \
		if (statistic2 != STAT_LAST) mysqlnd_global_stats->values[(statistic2)]+= v2; \
	}\
 }

#define MYSQLND_INC_CONN_STATISTIC(conn_stats, statistic) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		DBG_INF_FMT("Global&Conn stat increase [%s]", mysqlnd_stats_values_names[(statistic)]); \
		mysqlnd_global_stats->values[(statistic)]++; \
		if ((conn_stats)) { \
			((MYSQLND_STATS *) conn_stats)->values[(statistic)]++; \
		} \
	} \
 }

#define MYSQLND_INC_CONN_STATISTIC_W_VALUE(conn_stats, statistic, value) \
 { \
 	if (MYSQLND_G(collect_statistics) && (statistic) != STAT_LAST) { \
		uint64_t v = (uint64_t) (value); \
		DBG_INF_FMT("Global&Conn stats increase w value [%s]", mysqlnd_stats_values_names[(statistic)]); \
		mysqlnd_global_stats->values[(statistic)] += v; \
		if ((conn_stats)) { \
			((MYSQLND_STATS *) conn_stats)->values[(statistic)] += v; \
		} \
	} \
 } 

#define MYSQLND_INC_CONN_STATISTIC_W_VALUE2(conn_stats, statistic1, value1, statistic2, value2) \
 { \
 	if (MYSQLND_G(collect_statistics)) { \
		uint64_t v1 = (uint64_t) (value1); \
		uint64_t v2 = (uint64_t) (value2); \
								 \
		if (statistic1 != STAT_LAST) mysqlnd_global_stats->values[(statistic1)]+= v1; \
		if (statistic2 != STAT_LAST) mysqlnd_global_stats->values[(statistic2)]+= v2; \
		if ((conn_stats)) { \
			if (statistic1 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic1)]+= v1; \
			if (statistic2 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic2)]+= v2; \
		} \
	} \
 }

#define MYSQLND_INC_CONN_STATISTIC_W_VALUE3(conn_stats, statistic1, value1, statistic2, value2, statistic3, value3) \
 { \
 	if (MYSQLND_G(collect_statistics)) { \
		uint64_t v1 = (uint64_t) (value1); \
		uint64_t v2 = (uint64_t) (value2); \
		uint64_t v3 = (uint64_t) (value3); \
								\
		if (statistic1 != STAT_LAST) mysqlnd_global_stats->values[(statistic1)]+= v1; \
		if (statistic2 != STAT_LAST) mysqlnd_global_stats->values[(statistic2)]+= v2; \
		if (statistic3 != STAT_LAST) mysqlnd_global_stats->values[(statistic3)]+= v3; \
		if ((conn_stats)) { \
			if (statistic1 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic1)]+= v1; \
			if (statistic2 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic2)]+= v2; \
			if (statistic3 != STAT_LAST) ((MYSQLND_STATS *) conn_stats)->values[(statistic3)]+= v3; \
		} \
	} \
 }

#endif

void mysqlnd_fill_stats_hash(const MYSQLND_STATS * const stats, zval *return_value
							 TSRMLS_DC ZEND_FILE_LINE_DC);


void mysqlnd_stats_init(MYSQLND_STATS ** stats);
void mysqlnd_stats_end(MYSQLND_STATS * stats);

#endif	/* MYSQLND_STATISTICS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
