dnl $Id$
dnl config.m4 for extension wddx

AC_MSG_CHECKING(whether to include WDDX support)
AC_ARG_WITH(wddx,
[  --enable-wddx           Include WDDX support],[
  if test "$enableval" = "yes"; then
    if test "${enable_xml+set}" != "set" -o "$enable_xml" = "no"; then
	AC_MSG_ERROR(WDDX requires --enable-xml)
    else
        AC_DEFINE(HAVE_WDDX, 1, [ ])
        AC_MSG_RESULT(yes)
        PHP_EXTENSION(wddx)
    fi
  else
    AC_MSG_RESULT(no)
  fi
],[
  AC_MSG_RESULT(no)
]) 
