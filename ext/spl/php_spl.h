/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Marcus Boerger <helly@php.net>                              |
   +----------------------------------------------------------------------+
 */

#ifndef PHP_SPL_H
#define PHP_SPL_H

#include "php.h"
#include <stdarg.h>

#if 0
#define SPL_DEBUG(x)	x
#else
#define SPL_DEBUG(x)
#endif

extern zend_module_entry spl_module_entry;
#define phpext_spl_ptr &spl_module_entry

#if defined(PHP_WIN32) && !defined(COMPILE_DL_SPL)
#undef phpext_spl
#define phpext_spl NULL
#endif

PHP_MINIT_FUNCTION(spl);
PHP_MSHUTDOWN_FUNCTION(spl);
PHP_RINIT_FUNCTION(spl);
PHP_RSHUTDOWN_FUNCTION(spl);
PHP_MINFO_FUNCTION(spl);

#define ZEND_EXECUTE_HOOK_PTR(name) \
	opcode_handler_t handler_ ## name

#define ZEND_EXECUTE_HOOK(name) \
	spl_globals->handler_ ## name = zend_opcode_handlers[name]; \
	zend_opcode_handlers[name] = spl_handler_ ## name

#define ZEND_EXECUTE_HOOK_RESTORE(name) \
	zend_opcode_handlers[name] = SPL_G(handler_ ## name)

#define ZEND_EXECUTE_HOOK_ORIGINAL(name) \
	return SPL_G(handler_ ## name)(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU)

#define ZEND_EXECUTE_HOOK_FUNCTION(name) \
	int spl_handler_ ## name(ZEND_OPCODE_HANDLER_ARGS)


ZEND_BEGIN_MODULE_GLOBALS(spl)
#ifdef SPL_FOREACH
	ZEND_EXECUTE_HOOK_PTR(ZEND_FE_RESET);
	ZEND_EXECUTE_HOOK_PTR(ZEND_FE_FETCH);
#endif
#if defined(SPL_ARRAY_READ) | defined(SPL_ARRAY_WRITE)
	ZEND_EXECUTE_HOOK_PTR(ZEND_FETCH_DIM_R);
	ZEND_EXECUTE_HOOK_PTR(ZEND_FETCH_DIM_W);
	ZEND_EXECUTE_HOOK_PTR(ZEND_FETCH_DIM_RW);
#endif
#ifdef SPL_ARRAY_WRITE
	ZEND_EXECUTE_HOOK_PTR(ZEND_ASSIGN);
#endif
ZEND_END_MODULE_GLOBALS(spl)

#ifdef ZTS
# define SPL_G(v) TSRMG(spl_globals_id, zend_spl_globals *, v)
extern int spl_globals_id;
#else
# define SPL_G(v) (spl_globals.v)
extern zend_spl_globals spl_globals;
#endif

extern zend_namespace   *spl_ns_spl;
extern zend_class_entry *spl_ce_iterator;
extern zend_class_entry *spl_ce_forward;
extern zend_class_entry *spl_ce_sequence;
extern zend_class_entry *spl_ce_assoc;
extern zend_class_entry *spl_ce_forward_assoc;
extern zend_class_entry *spl_ce_sequence_assoc;
extern zend_class_entry *spl_ce_array_read;
extern zend_class_entry *spl_ce_array_access;
extern zend_class_entry *spl_ce_array_access_ex;
extern zend_class_entry *spl_ce_array_writer;
#ifdef SPL_ARRAY_WRITE
extern zend_class_entry *spl_ce_array_writer_default;
#endif /* SPL_ARRAY_WRITE */

PHP_FUNCTION(spl_classes);
PHP_FUNCTION(class_name);
PHP_FUNCTION(class_parents);
PHP_FUNCTION(class_implements);

#endif /* PHP_SPL_H */

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
