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
  | Author: Ard Biesheuvel <abies@php.net>                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "pdo/php_pdo.h"
#include "pdo/php_pdo_driver.h"
#include "php_pdo_firebird.h"
#include "php_pdo_firebird_int.h"

/* map driver specific error message to PDO error */
void _firebird_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, char const *file, long line TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = stmt ? ((pdo_firebird_stmt *)stmt->driver_data)->H 
		: (pdo_firebird_db_handle *)dbh->driver_data;
	long *error_code = stmt ? &stmt->error_code : &dbh->error_code;
	
	switch (isc_sqlcode(H->isc_status)) {

		case 0:
			*error_code = PDO_ERR_NONE;
			break;
		default:
			*error_code = PDO_ERR_CANT_MAP;
			break;
		case -104:
			*error_code = PDO_ERR_SYNTAX;
			break;
		case -530:
		case -803:
			*error_code = PDO_ERR_CONSTRAINT;
			break;
		case -204:
		case -205:
		case -206:
		case -829:
			*error_code = PDO_ERR_NOT_FOUND;
			break;
		case -607: 		
			*error_code = PDO_ERR_ALREADY_EXISTS;
			break;
		
			*error_code = PDO_ERR_NOT_IMPLEMENTED;
			break;
		case -313:
		case -804:
			*error_code = PDO_ERR_MISMATCH;
			break;
		case -303:
		case -314:	
		case -413:
			*error_code = PDO_ERR_TRUNCATED;
			break;
			
			*error_code = PDO_ERR_DISCONNECTED;
			break;
	}
}
/* }}} */

#define RECORD_ERROR(dbh) _firebird_error(dbh, NULL, __FILE__, __LINE__ TSRMLS_CC)

/* called by PDO to close a db handle */
static int firebird_handle_closer(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;
	
	if (dbh->in_txn) {
		if (dbh->auto_commit) {
			if (isc_commit_transaction(H->isc_status, &H->tr)) {
				RECORD_ERROR(dbh);
			}
		} else {
			if (isc_rollback_transaction(H->isc_status, &H->tr)) {
				RECORD_ERROR(dbh);
			}
		}
	}
	
	if (isc_detach_database(H->isc_status, &H->db)) {
		RECORD_ERROR(dbh);
	}

	pefree(H, dbh->is_persistent);

	return 0;
}
/* }}} */

/* called by PDO to prepare an SQL query */
static int firebird_handle_preparer(pdo_dbh_t *dbh, const char *sql, long sql_len, /* {{{ */
	pdo_stmt_t *stmt, long options, zval *driver_options TSRMLS_DC)
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;
	pdo_firebird_stmt *S = NULL;

	do {
		isc_stmt_handle s = NULL;
		XSQLDA num_sqlda;
		static char info[] = {isc_info_sql_stmt_type};
		char result[8];

		num_sqlda.version = PDO_FB_SQLDA_VERSION;
		num_sqlda.sqln = 1;

		/* allocate a statement handle */
		if (isc_dsql_allocate_statement(H->isc_status, &H->db, &s)) {
			break;
		}

		/* prepare the SQL statement */
		if (isc_dsql_prepare(H->isc_status, &H->tr, &s, (short)sql_len, const_cast(sql),
				PDO_FB_DIALECT, &num_sqlda)) {
			break;
		}
		
		/* allocate a statement handle struct of the right size (struct out_sqlda is inlined) */
		S = ecalloc(1, sizeof(*S)-sizeof(XSQLDA) + XSQLDA_LENGTH(num_sqlda.sqld));
		S->H = H;
		S->stmt = s;
		S->fetch_buf = ecalloc(1,sizeof(char*) * num_sqlda.sqld);
		S->out_sqlda.version = PDO_FB_SQLDA_VERSION;
		S->out_sqlda.sqln = stmt->column_count = num_sqlda.sqld;

		/* determine the statement type */
		if (isc_dsql_sql_info(H->isc_status, &s, sizeof(info), info, sizeof(result), result)) {
			break;
		}
		S->statement_type = result[3];	
		
		/* fill the output sqlda with information about the prepared query */
		if (isc_dsql_describe(H->isc_status, &s, PDO_FB_SQLDA_VERSION, &S->out_sqlda)) {
			RECORD_ERROR(dbh);
			break;
		}
		
		/* allocate the input descriptors */
		if (isc_dsql_describe_bind(H->isc_status, &s, PDO_FB_SQLDA_VERSION, &num_sqlda)) {
			break;
		}
		
		if (num_sqlda.sqld) {
			S->in_sqlda = ecalloc(1,XSQLDA_LENGTH(num_sqlda.sqld));
			S->in_sqlda->version = PDO_FB_SQLDA_VERSION;
			S->in_sqlda->sqln = num_sqlda.sqld;
		
			if (isc_dsql_describe_bind(H->isc_status, &s, PDO_FB_SQLDA_VERSION, S->in_sqlda)) {
				break;
			}
		}
	
		stmt->driver_data = S;
		stmt->methods = &firebird_stmt_methods;
	
		return 1;

	} while (0);

	RECORD_ERROR(dbh);
	
	if (S) {
		if (S->in_sqlda) {
			efree(S->in_sqlda);
		}
		efree(S);
	}
	
	return 0;
}
/* }}} */

/* called by PDO to execute a statement that doesn't produce a result */
static long firebird_handle_doer(pdo_dbh_t *dbh, const char *sql, long sql_len TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;
	isc_stmt_handle stmt = NULL;
	static char info_count[] = { isc_info_sql_records };
	char result[64];
	int ret = 0;
	XSQLDA in_sqlda, out_sqlda;
		
	/* TODO no placeholders in exec() for now */
	in_sqlda.version = out_sqlda.version = PDO_FB_SQLDA_VERSION;
	in_sqlda.sqld = out_sqlda.sqld = 0;
	
	/* start a new transaction implicitly if auto_commit is enabled and no transaction is open */
	if (dbh->auto_commit && !dbh->in_txn) {
		if (isc_start_transaction(H->isc_status, &H->tr, 1, &H->db, 0, NULL)) {
			RECORD_ERROR(dbh);
			return -1;
		}
		dbh->in_txn = 1;
	}
	
	/* allocate the statement */
	if (isc_dsql_allocate_statement(H->isc_status, &H->db, &stmt)) {
		RECORD_ERROR(dbh);
		return -1;
	}
	
	/* Firebird allows SQL statements up to 64k, so bail if it doesn't fit */
	if (sql_len > SHORT_MAX) {
		dbh->error_code = PDO_ERR_TRUNCATED;
		return -1;
	}
	
	/* prepare the statement */
	if (isc_dsql_prepare(H->isc_status, &H->tr, &stmt, (short) sql_len, const_cast(sql),
			PDO_FB_DIALECT, &out_sqlda)) {
		RECORD_ERROR(dbh);
		return -1;
	}

	/* execute the statement */
	if (isc_dsql_execute2(H->isc_status, &H->tr, &stmt, PDO_FB_SQLDA_VERSION, &in_sqlda, &out_sqlda)) {
		RECORD_ERROR(dbh);
		return -1;
	}
	
	/* find out how many rows were affected */
	if (isc_dsql_sql_info(H->isc_status, &stmt, sizeof(info_count), info_count, sizeof(result),
			result)) {
		RECORD_ERROR(dbh);
		return -1;
	}

	if (result[0] == isc_info_sql_records) {
		unsigned i = 3, result_size = isc_vax_integer(&result[1],2);

		while (result[i] != isc_info_end && i < result_size) {
			short len = (short)isc_vax_integer(&result[i+1],2);
			if (result[i] != isc_info_req_select_count) {
				ret += isc_vax_integer(&result[i+3],len);
			}
			i += len+3;
		}
	}
	
	/* commit if we're in auto_commit mode */
	if (dbh->auto_commit && isc_commit_retaining(H->isc_status, &H->tr)) {
		RECORD_ERROR(dbh);
	}

	return ret;
}
/* }}} */

/* called by the PDO SQL parser to add quotes to values that are copied into SQL */
static int firebird_handle_quoter(pdo_dbh_t *dbh, const char *unquoted, int unquotedlen, /* {{{ */
	char **quoted, int *quotedlen TSRMLS_DC)
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;
	int qcount = 0;
	char const *c;
	
	/* Firebird only requires single quotes to be doubled if string lengths are used */
	
	/* count the number of ' characters */
	for (c = unquoted; c = strchr(c,'\''); qcount++, c++);
	
	if (!qcount) {
		return 0;
	} else {
		char const *l, *r;
		char *c;
		
		*quotedlen = unquotedlen + qcount;
		*quoted = c = emalloc(*quotedlen+1);
		
		/* foreach (chunk that ends in a quote) */
		for (l = unquoted; r = strchr(l,'\''); l = r+1) {
			
			/* copy the chunk */
			strncpy(c, l, r-l);
			c += (r-l);
			
			/* add the second quote */
			*c++ = '\'';
		}
		
		/* copy the remainder */
		strncpy(c, l, *quotedlen-(c-*quoted));
		
		return 1;
	}			
}
/* }}} */

/* called by PDO to start a transaction */
static int firebird_handle_begin(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;

	if (isc_start_transaction(H->isc_status, &H->tr, 1, &H->db, 0, NULL)) {
		RECORD_ERROR(dbh);
		return 0;
	}
	return 1;
}
/* }}} */

/* called by PDO to commit a transaction */
static int firebird_handle_commit(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;

	if (isc_commit_transaction(H->isc_status, &H->tr)) {
		RECORD_ERROR(dbh);
		return 0;
	}
	return 1;
}
/* }}} */

/* called by PDO to rollback a transaction */
static int firebird_handle_rollback(pdo_dbh_t *dbh TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;

	if (isc_rollback_transaction(H->isc_status, &H->tr)) {
		RECORD_ERROR(dbh);
		return 0;
	}
	return 1;
}
/* }}} */

/* called by PDO to set a driver-specific dbh attribute */
static int firebird_handle_set_attribute(pdo_dbh_t *dbh, long attr, zval *val TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;

	switch (attr) {

		case PDO_ATTR_AUTOCOMMIT:

			convert_to_long(val);
	
			/* if (the value is really being changed and a transaction is open) */			
			if ((Z_LVAL_P(val)?1:0) ^ dbh->auto_commit && dbh->in_txn) {
				
				if (dbh->auto_commit = Z_BVAL_P(val)) {
					/* just keep the running transaction but commit it */
					if (isc_commit_retaining(H->isc_status, &H->tr)) {
						RECORD_ERROR(dbh);
						break;
					}
				} else {
					/* close the transaction */
					if (isc_commit_transaction(H->isc_status, &H->tr)) {
						RECORD_ERROR(dbh);
						break;
					}
					dbh->in_txn = 0;
				}
			}
			return 1;
	}
	return 0;
}
/* }}} */

/* called by PDO to get a driver-specific dbh attribute */
static int firebird_handle_get_attribute(pdo_dbh_t *dbh, long attr, zval *val TSRMLS_DC) /* {{{ */
{
	return 0;
}
/* }}} */

/* called by PDO to retrieve driver-specific information about an error that has occurred */
static int pdo_firebird_fetch_error_func(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info TSRMLS_DC) /* {{{ */
{
	pdo_firebird_db_handle *H = (pdo_firebird_db_handle *)dbh->driver_data;
	ISC_STATUS *s = H->isc_status;
	char buf[400];
	long i = 0, l, sqlcode = isc_sqlcode(s);

	if (sqlcode) {
		add_next_index_long(info, sqlcode);

		while (l = isc_interprete(&buf[i],&s)) {
			i += l;
			strcpy(&buf[i++], " ");
		}
		add_next_index_string(info, buf, 1);
	} else {
		add_next_index_long(info, -999);
		add_next_index_string(info, H->last_app_error,1);
	}
	return 1;
}
/* }}} */

static struct pdo_dbh_methods firebird_methods = { /* {{{ */
	firebird_handle_closer,
	firebird_handle_preparer,
	firebird_handle_doer,
	firebird_handle_quoter,
	firebird_handle_begin,
	firebird_handle_commit,
	firebird_handle_rollback,
	firebird_handle_set_attribute,
	NULL, /* last_id not supported */
	pdo_firebird_fetch_error_func,
	firebird_handle_get_attribute,
};
/* }}} */

/* the driver-specific PDO handle constructor */
static int pdo_firebird_handle_factory(pdo_dbh_t *dbh, zval *driver_options TSRMLS_DC) /* {{{ */
{
	struct pdo_data_src_parser vars[] = {
		{ "dbname", NULL, 0 },
		{ "charset",  NULL,	0 },
		{ "role", NULL,	0 }
	};
	int i, ret = 0;
	pdo_firebird_db_handle *H = dbh->driver_data = pecalloc(1,sizeof(*H),dbh->is_persistent);

	php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, vars, 2);
	
	do {
		static char const dpb_flags[] = { 
			isc_dpb_user_name, isc_dpb_password, isc_dpb_lc_ctype, isc_dpb_sql_role_name };
		char const *dpb_values[] = { dbh->username, dbh->password, vars[1].optval, vars[2].optval };
		char dpb_buffer[256] = { isc_dpb_version1 }, *dpb;
		short len;
		
		dpb = dpb_buffer + 1; 
		
		/* loop through all the provided arguments and set dpb fields accordingly */
		for (i = 0; i < sizeof(dpb_flags); ++i) {
			if (dpb_values[i]) {
				dpb += sprintf(dpb, "%c%c%s", dpb_flags[i], (unsigned char)strlen(dpb_values[i]),
					dpb_values[i]);
			}
		}
		
		/* fire it up baby! */
		if (isc_attach_database(H->isc_status, 0, vars[0].optval, &H->db,(short)(dpb-dpb_buffer),
				dpb_buffer)) {
			break;
		}
		
		dbh->methods = &firebird_methods;
		dbh->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL;
		dbh->native_case = PDO_CASE_UPPER;
		dbh->alloc_own_columns = 1;

		ret = 1;
		
	} while (0);
		
	for (i = 0; i < sizeof(vars)/sizeof(vars[0]); ++i) {
		if (vars[i].freeme) {
			efree(vars[i].optval);
		}
	}

	if (!ret) {
		firebird_handle_closer(dbh TSRMLS_CC);
	}

	return ret;
}
/* }}} */

pdo_driver_t pdo_firebird_driver = { /* {{{ */
	PDO_DRIVER_HEADER(firebird),
	pdo_firebird_handle_factory
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
