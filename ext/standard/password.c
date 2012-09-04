/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Anthony Ferrara <ircmaxell@php.net>                         |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include <stdlib.h>

#include "php.h"
#if HAVE_CRYPT

#include "fcntl.h"
#include "php_password.h"
#include "php_rand.h"
#include "php_crypt.h"
#include "base64.h"
#include "zend_interfaces.h"
#include "info.h"

#if PHP_WIN32
#include "win32/winutil.h"
#endif

PHP_MINIT_FUNCTION(password) /* {{{ */
{
	REGISTER_LONG_CONSTANT("PASSWORD_DEFAULT", PHP_PASSWORD_DEFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PASSWORD_BCRYPT", PHP_PASSWORD_BCRYPT, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

static char* php_password_get_algo_name(const int algo)
{
	switch (algo) {
		case PHP_PASSWORD_BCRYPT:
			return "bcrypt";
		default:
			return "unknown";
	}
}

static int php_password_determine_algo(const char *hash, const size_t len) 
{
	if (len < 3) {
		return 0;
	}
	if (hash[0] == '$' && hash[1] == '2' && hash[2] == 'y' && len == 60) {
		return PHP_PASSWORD_BCRYPT;
	}

	return 0;
}

static int php_password_salt_is_alphabet(const char *str, const size_t len) /* {{{ */
{
	size_t i = 0;

	for (i = 0; i < len; i++) {
		if (!((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= '0' && str[i] <= '9') || str[i] == '.' || str[i] == '/')) {
			return 0;
		}
	}
	return 1;
}
/* }}} */

static int php_password_salt_to64(const char *str, const size_t str_len, const size_t out_len, char *ret) /* {{{ */
{
	size_t pos = 0;
	size_t ret_len = 0;
	unsigned char *buffer;
	if ((int) str_len < 0) {
		return FAILURE;
	}
	buffer = php_base64_encode((unsigned char*) str, (int) str_len, (int*) &ret_len);
	if (ret_len < out_len) {
		/* Too short of an encoded string generated */
		efree(buffer);
		return FAILURE;
	}
	for (pos = 0; pos < out_len; pos++) {
		if (buffer[pos] == '+') {
			ret[pos] = '.';
		} else if (buffer[pos] == '=') {
			efree(buffer);
			return FAILURE;
		} else {
			ret[pos] = buffer[pos];
		}
	}
	efree(buffer);
	return SUCCESS;
}
/* }}} */

static int php_password_make_salt(size_t length, char *ret TSRMLS_DC) /* {{{ */
{
	int buffer_valid = 0;
	size_t i, raw_length;
	char *buffer;
	char *result;

	if (length > (INT_MAX / 3)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Length is too large to safely generate");
		return FAILURE;
	}

	raw_length = length * 3 / 4 + 1;

	buffer = (char *) safe_emalloc(raw_length, 1, 1);

#if PHP_WIN32
	{
		BYTE *iv_b = (BYTE *) buffer;
		if (php_win32_get_random_bytes(iv_b, raw_length) == SUCCESS) {
			buffer_valid = 1;
		}
	}
#else
	{
		int fd, n;
		size_t read_bytes = 0;
		fd = open("/dev/urandom", O_RDONLY);
		if (fd >= 0) {
			while (read_bytes < raw_length) {
				n = read(fd, buffer + read_bytes, raw_length - read_bytes);
				if (n < 0) {
					break;
				}
				read_bytes += (size_t) n;
			}
			close(fd);
		}
		if (read_bytes >= raw_length) {
			buffer_valid = 1;
		}
	}
#endif
	if (!buffer_valid) {
		for (i = 0; i < raw_length; i++) {
			buffer[i] ^= (char) (255.0 * php_rand(TSRMLS_C) / RAND_MAX);
		}
	}

	result = safe_emalloc(length, 1, 1); 
	if (php_password_salt_to64(buffer, raw_length, length, result) == FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Generated salt too short");
		efree(buffer);
		efree(result);
		return FAILURE;
	} else {
		memcpy(ret, result, (int) length);
	}
	efree(result);
	efree(buffer);
	ret[length] = 0;
	return SUCCESS;
}
/* }}} */

PHP_FUNCTION(password_get_info)
{
	long algo;
	int hash_len;
	char *hash, *algoName;
	zval *options;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hash, &hash_len) == FAILURE) {
		RETURN_NULL();
	}

	if (hash_len < 0 || (size_t) hash_len < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Supplied Password Hash Too Long To Safely Identify");
		RETURN_FALSE;
	}

	ALLOC_INIT_ZVAL(options);
	array_init(options);

	algo = php_password_determine_algo(hash, (size_t) hash_len);
	algoName = php_password_get_algo_name(algo);
	
	switch (algo) {
		case PHP_PASSWORD_BCRYPT:
			{
				long cost = PHP_PASSWORD_BCRYPT_COST;
				sscanf(hash, "$2y$%ld$", &cost);
				add_assoc_long(options, "cost", cost);
			}
		break;
	}

	array_init(return_value);
	
	add_assoc_long(return_value, "algo", algo);
	add_assoc_string(return_value, "algoName", algoName, 1);
	add_assoc_zval(return_value, "options", options);	
}

PHP_FUNCTION(password_needs_rehash)
{
	long new_algo = 0, algo = 0;
	int hash_len;
	char *hash;
	HashTable *options = 0;
	zval **option_buffer;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|H", &hash, &hash_len, &new_algo, &options) == FAILURE) {
		RETURN_NULL();
	}

	if (hash_len < 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Supplied Password Hash Too Long To Safely Identify");
		RETURN_FALSE;
	}

	algo = php_password_determine_algo(hash, (size_t) hash_len);
	
	if (algo != new_algo) {
		RETURN_TRUE;
	}

	switch (algo) {
		case PHP_PASSWORD_BCRYPT:
			{
				int newCost = PHP_PASSWORD_BCRYPT_COST, cost = 0;
				
				if (options && zend_symtable_find(options, "cost", 5, (void **) &option_buffer) == SUCCESS) {
					convert_to_long_ex(option_buffer);
					newCost = Z_LVAL_PP(option_buffer);
					zval_ptr_dtor(option_buffer);
				}

				sscanf(hash, "$2y$%d$", &cost);
				if (cost != newCost) {
					RETURN_TRUE;
				}
			}
			break;
	}
	RETURN_FALSE;
}

/* {{{ proto boolean password_make_salt(string password, string hash)
Verify a hash created using crypt() or password_hash() */
PHP_FUNCTION(password_verify)
{
	int status = 0, i;
	int password_len, hash_len;
	char *ret, *password, *hash;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &password, &password_len, &hash, &hash_len) == FAILURE) {
		RETURN_FALSE;
	}
	if (php_crypt(password, password_len, hash, hash_len, &ret) == FAILURE) {
		RETURN_FALSE;
	}

	if (strlen(ret) != hash_len || hash_len < 13) {
		efree(ret);
		RETURN_FALSE;
	}
	
	/* We're using this method instead of == in order to provide
	 * resistence towards timing attacks. This is a constant time
	 * equality check that will always check every byte of both
	 * values. */
	for (i = 0; i < hash_len; i++) {
		status |= (ret[i] ^ hash[i]);
	}

	efree(ret);

	RETURN_BOOL(status == 0);
	
}
/* }}} */

/* {{{ proto string password_hash(string password, int algo, array options = array())
Hash a password */
PHP_FUNCTION(password_hash)
{
	char *hash_format, *hash, *salt, *password, *result;
	long algo = 0;
	int password_len = 0, hash_len;
	size_t salt_len = 0, required_salt_len = 0, hash_format_len;
	HashTable *options = 0;
	zval **option_buffer;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|H", &password, &password_len, &algo, &options) == FAILURE) {
		RETURN_NULL();
	}

	switch (algo) {
		case PHP_PASSWORD_BCRYPT:
		{
			long cost = PHP_PASSWORD_BCRYPT_COST;
	
			if (options && zend_symtable_find(options, "cost", 5, (void **) &option_buffer) == SUCCESS) {
				convert_to_long_ex(option_buffer);
				cost = Z_LVAL_PP(option_buffer);
				zval_ptr_dtor(option_buffer);
			}
	
			if (cost < 4 || cost > 31) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid bcrypt cost parameter specified: %ld", cost);
				RETURN_NULL();
			}
			
			required_salt_len = 22;
			hash_format = emalloc(8);
			sprintf(hash_format, "$2y$%02ld$", cost);
			hash_format_len = 7;
		}
		break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unknown password hashing algorithm: %ld", algo);
			RETURN_NULL();
	}

	if (options && zend_symtable_find(options, "salt", 5, (void**) &option_buffer) == SUCCESS) {
		char *buffer;
		int buffer_len_int;
		size_t buffer_len;
		switch (Z_TYPE_PP(option_buffer)) {
			case IS_NULL:
			case IS_STRING:
			case IS_LONG:
			case IS_DOUBLE:
			case IS_BOOL:
			case IS_OBJECT:
				convert_to_string_ex(option_buffer);
				if (Z_TYPE_PP(option_buffer) == IS_STRING) {
					buffer = Z_STRVAL_PP(option_buffer);
					buffer_len_int = Z_STRLEN_PP(option_buffer);
					if (buffer_len_int < 0) {
						zval_ptr_dtor(option_buffer);
						efree(hash_format);
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Supplied salt is too long");
					}
					buffer_len = (size_t) buffer_len_int;
					break;
				}
			case IS_RESOURCE:
			case IS_ARRAY:
			default:
				zval_ptr_dtor(option_buffer);
				efree(hash_format);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Non-string salt parameter supplied");
				RETURN_NULL();
		}
		if (buffer_len < required_salt_len) {
			efree(hash_format);
			zval_ptr_dtor(option_buffer);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Provided salt is too short: %lu expecting %lu", (unsigned long) buffer_len, (unsigned long) required_salt_len);
			RETURN_NULL();
		} else if (0 == php_password_salt_is_alphabet(buffer, buffer_len)) {
			salt = safe_emalloc(required_salt_len, 1, 1);
			if (php_password_salt_to64(buffer, buffer_len, required_salt_len, salt) == FAILURE) {
				efree(hash_format);
				efree(salt);
				zval_ptr_dtor(option_buffer);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Provided salt is too short: %lu", (unsigned long) buffer_len);
				RETURN_NULL();
			}
			salt_len = required_salt_len;
		} else {
			salt = safe_emalloc(required_salt_len, 1, 1);
			memcpy(salt, buffer, (int) required_salt_len);
			salt_len = required_salt_len;
		}
		zval_ptr_dtor(option_buffer);
	} else {
		salt = safe_emalloc(required_salt_len, 1, 1);
		if (php_password_make_salt(required_salt_len, salt TSRMLS_CC) == FAILURE) {
			efree(hash_format);
			efree(salt);
			RETURN_FALSE;
		}
		salt_len = required_salt_len;
	}
	
	salt[salt_len] = 0;

	hash = safe_emalloc(salt_len + hash_format_len, 1, 1);
	sprintf(hash, "%s%s", hash_format, salt);
	hash[hash_format_len + salt_len] = 0;

	efree(hash_format);
	efree(salt);

	/* This cast is safe, since both values are defined here in code and cannot overflow */
	hash_len = (int) (hash_format_len + salt_len);

	if (php_crypt(password, password_len, hash, hash_len, &result) == FAILURE) {
		efree(hash);
		RETURN_FALSE;
	}

	efree(hash);

	if (strlen(result) < 13) {
		efree(result);
		RETURN_FALSE;
	}

	RETURN_STRING(result, 0);
}
/* }}} */

#endif /* HAVE_CRYPT */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
