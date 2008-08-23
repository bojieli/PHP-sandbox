/* $Id$ */
#ifndef _CRYPT_FREESEC_H
#define _CRYPT_FREESEC_H

#include "php_config.h"

#if PHP_WIN32
# include "win32/php_stdint.h"
#elif HAVE_STDINT_H
# include <stdint.h>
#endif
#if !PHP_WIN32
# ifndef HAVE_U_INT32_T
   typedef uint32_t u_int32_t;
# endif
#endif

#define MD5_HASH_MAX_LEN 120

struct php_crypt_extended_data {
	int initialized;
	u_int32_t saltbits;
	u_int32_t old_salt;
	u_int32_t en_keysl[16], en_keysr[16];
	u_int32_t de_keysl[16], de_keysr[16];
	u_int32_t old_rawkey0, old_rawkey1;
	char output[21];
};

/*
 * _crypt_extended_init() must be called explicitly before first use of
 * _crypt_extended_r().
 */

void _crypt_extended_init(void);

char *_crypt_extended_r(const char *key, const char *setting,
	struct php_crypt_extended_data *data);

#endif
