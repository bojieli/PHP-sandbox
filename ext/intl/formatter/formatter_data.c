/*
   +----------------------------------------------------------------------+
   | PHP Version 6                                                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Stanislav Malyshev <stas@zend.com>                          |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "formatter_data.h"

/* {{{ void formatter_data_init( formatter_data* nf_data )
 * Initialize internals of formatter_data.
 */
void formatter_data_init( formatter_data* nf_data TSRMLS_DC )
{
	if( !nf_data )
		return;

	nf_data->unum                = NULL;
	intl_error_reset( &nf_data->error TSRMLS_CC );
}
/* }}} */

/* {{{ void formatter_data_free( formatter_data* nf_data )
 * Clean up mem allocted by internals of formatter_data
 */
void formatter_data_free( formatter_data* nf_data TSRMLS_DC )
{
	if( !nf_data )
		return;

	if( nf_data->unum )
		unum_close( nf_data->unum );

	nf_data->unum = NULL;
	intl_error_reset( &nf_data->error TSRMLS_CC );
}
/* }}} */

/* {{{ formatter_data* formatter_data_create()
 * Alloc mem for formatter_data and initialize it with default values.
 */
formatter_data* formatter_data_create( TSRMLS_D )
{
	formatter_data* nf_data = ecalloc( 1, sizeof(formatter_data) );

	formatter_data_init( nf_data TSRMLS_CC );

	return nf_data;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
