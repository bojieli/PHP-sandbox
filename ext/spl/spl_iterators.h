/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2005 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Marcus Boerger <helly@php.net>                              |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifndef SPL_ITERATORS_H
#define SPL_ITERATORS_H

#include "php.h"
#include "php_spl.h"

#define spl_ce_Traversable   zend_ce_traversable
#define spl_ce_Iterator      zend_ce_iterator
#define spl_ce_Aggregate     zend_ce_aggregate
#define spl_ce_ArrayAccess   zend_ce_arrayaccess
#define spl_ce_Serializable  zend_ce_serializable

extern PHPAPI zend_class_entry *spl_ce_RecursiveIterator;
extern PHPAPI zend_class_entry *spl_ce_RecursiveIteratorIterator;
extern PHPAPI zend_class_entry *spl_ce_FilterIterator;
extern PHPAPI zend_class_entry *spl_ce_RecursiveFilterIterator;
extern PHPAPI zend_class_entry *spl_ce_ParentIterator;
extern PHPAPI zend_class_entry *spl_ce_SeekableIterator;
extern PHPAPI zend_class_entry *spl_ce_LimitIterator;
extern PHPAPI zend_class_entry *spl_ce_CachingIterator;
extern PHPAPI zend_class_entry *spl_ce_RecursiveCachingIterator;
extern PHPAPI zend_class_entry *spl_ce_OuterIterator;
extern PHPAPI zend_class_entry *spl_ce_IteratorIterator;
extern PHPAPI zend_class_entry *spl_ce_NoRewindIterator;
extern PHPAPI zend_class_entry *spl_ce_InfiniteIterator;
extern PHPAPI zend_class_entry *spl_ce_EmptyIterator;
extern PHPAPI zend_class_entry *spl_ce_AppendIterator;

PHP_MINIT_FUNCTION(spl_iterators);

PHP_FUNCTION(iterator_to_array);
PHP_FUNCTION(iterator_count);

typedef enum {
	DIT_Default = 0,
	DIT_LimitIterator,
	DIT_CachingIterator,
	DIT_RecursiveCachingIterator,
	DIT_IteratorIterator,
	DIT_NoRewindIterator,
	DIT_InfiniteIterator,
	DIT_AppendIterator,
} dual_it_type;

enum {
	/* public */
	CIT_CALL_TOSTRING   = 0x00000001,
	CIT_CATCH_GET_CHILD = 0x00000002,
	CIT_FULL_CACHE      = 0x00000004,
	CIT_PUBLIC          = 0x00FFFFFF,
	/* private */
	CIT_VALID           = 0x01000000,
	CIT_HAS_CHILDREN    = 0x02000000,
};

typedef struct _spl_dual_it_object {
	zend_object              std;
	struct {
		zval                 *zobject;
		zend_class_entry     *ce;
		zend_object          *object;
		zend_object_iterator *iterator;
	} inner;
	struct {
		zval                 *data;
		char                 *str_key;
		uint                 str_key_len;
		ulong                int_key;
		int                  key_type; /* HASH_KEY_IS_STRING or HASH_KEY_IS_LONG */
		int                  pos;
	} current;
	dual_it_type             dit_type;
	union {
		struct {
			long             offset;
			long             count;
		} limit;
		struct {
			int              flags; /* CIT_VALID, CIT_CALL_TOSTRING, CIT_CATCH_GET_CHILD, ... */
			zval             *zstr;
			zval             *zchildren;
			zval             *zcache;
		} caching;
		struct {
			zval                 *zarrayit;
			zend_object_iterator *iterator;
		} append;
	} u;
} spl_dual_it_object;

#endif /* SPL_ITERATORS_H */

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
