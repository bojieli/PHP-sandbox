/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Thies C. Arntzen <thies@thieso.net>                          |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifndef PHP_DIR_H
#define PHP_DIR_H

/* directory functions */
PHP_MINIT_FUNCTION(dir);
PHP_RINIT_FUNCTION(dir);
PHP_FUNCTION(opendir);
PHP_FUNCTION(closedir);
PHP_FUNCTION(chdir);
#if defined(HAVE_CHROOT) && !defined(ZTS) && ENABLE_CHROOT_FUNC
PHP_FUNCTION(chroot);
#endif
PHP_FUNCTION(getcwd);
PHP_FUNCTION(rewinddir);
PHP_NAMED_FUNCTION(php_if_readdir);
PHP_FUNCTION(getdir);
PHP_FUNCTION(glob);
PHP_FUNCTION(scandir);

#endif /* PHP_DIR_H */
