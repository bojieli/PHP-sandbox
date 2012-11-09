dnl $Id$
dnl config.m4 for extension daemon

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(daemon, for daemon support,
dnl Make sure that the comment is aligned:
dnl [  --with-daemon             Include daemon support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(daemon, whether to enable daemon support,
dnl Make sure that the comment is aligned:
[  --enable-daemon           Enable daemon support])

if test "$PHP_BLOG" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-daemon -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/daemon.h"  # you most likely want to change this
  dnl if test -r $PHP_BLOG/$SEARCH_FOR; then # path given as parameter
  dnl   BLOG_DIR=$PHP_BLOG
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for daemon files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       BLOG_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$BLOG_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the daemon distribution])
  dnl fi

  dnl # --with-daemon -> add include path
  dnl PHP_ADD_INCLUDE($BLOG_DIR/include)

  dnl # --with-daemon -> check for lib and symbol presence
  dnl LIBNAME=daemon # you may want to change this
  dnl LIBSYMBOL=daemon # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BLOG_DIR/lib, BLOG_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_BLOGLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong daemon lib version or lib not found])
  dnl ],[
  dnl   -L$BLOG_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(BLOG_SHARED_LIBADD)

  PHP_NEW_EXTENSION(daemon, daemon.c, $ext_shared)
fi
