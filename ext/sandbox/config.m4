dnl $Id$
dnl config.m4 for extension sandbox

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(sandbox, for sandbox support,
dnl Make sure that the comment is aligned:
dnl [  --with-sandbox             Include sandbox support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(sandbox, whether to enable sandbox support,
dnl Make sure that the comment is aligned:
[  --enable-sandbox           Enable sandbox support])

if test "$PHP_SANDBOX" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-sandbox -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/sandbox.h"  # you most likely want to change this
  dnl if test -r $PHP_SANDBOX/$SEARCH_FOR; then # path given as parameter
  dnl   SANDBOX_DIR=$PHP_SANDBOX
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for sandbox files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SANDBOX_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SANDBOX_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the sandbox distribution])
  dnl fi

  dnl # --with-sandbox -> add include path
  dnl PHP_ADD_INCLUDE($SANDBOX_DIR/include)

  dnl # --with-sandbox -> check for lib and symbol presence
  dnl LIBNAME=sandbox # you may want to change this
  dnl LIBSYMBOL=sandbox # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SANDBOX_DIR/lib, SANDBOX_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SANDBOXLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong sandbox lib version or lib not found])
  dnl ],[
  dnl   -L$SANDBOX_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SANDBOX_SHARED_LIBADD)

  PHP_NEW_EXTENSION(sandbox, sandbox.c, $ext_shared)
fi
