dnl $Id$

AC_DEFUN(AC_PDO_OCI_VERSION,[
  AC_MSG_CHECKING([Oracle version])
  if test -s "$PDO_OCI_DIR/orainst/unix.rgs"; then
    PDO_OCI_VERSION=`grep '"ocommon"' $PDO_OCI_DIR/orainst/unix.rgs | sed 's/[ ][ ]*/:/g' | cut -d: -f 6 | cut -c 2-4`
    test -z "$PDO_OCI_VERSION" && PDO_OCI_VERSION=7.3
  elif test -f $PDO_OCI_DIR/lib/libclntsh.$SHLIB_SUFFIX_NAME.10.1; then
    PDO_OCI_VERSION=10.1    
  elif test -f $PDO_OCI_DIR/lib/libclntsh.$SHLIB_SUFFIX_NAME.9.0; then
    PDO_OCI_VERSION=9.0
  elif test -f $PDO_OCI_DIR/lib/libclntsh.$SHLIB_SUFFIX_NAME.8.0; then
    PDO_OCI_VERSION=8.1
  elif test -f $PDO_OCI_DIR/lib/libclntsh.$SHLIB_SUFFIX_NAME.1.0; then
    PDO_OCI_VERSION=8.0
  elif test -f $PDO_OCI_DIR/lib/libclntsh.a; then 
    if test -f $PDO_OCI_DIR/lib/libcore4.a; then 
      PDO_OCI_VERSION=8.0
    else
      PDO_OCI_VERSION=8.1
    fi
  else
    AC_MSG_ERROR(Oracle-OCI needed libraries not found under $PDO_OCI_DIR)
  fi
  AC_MSG_RESULT($PDO_OCI_VERSION)
])                                                                                                                                                                

PHP_ARG_WITH(pdo-oci, Oracle OCI support for PDO,
[  --with-pdo-oci[=DIR]         Include Oracle-oci support. Default DIR is ORACLE_HOME.])

if test "$PHP_PDO_OCI" != "no"; then
  AC_MSG_CHECKING([Oracle Install-Dir])
  if test "$PHP_PDO_OCI" = "yes" -o -z "$PHP_PDO_OCI"; then
    PDO_OCI_DIR=$ORACLE_HOME
  else
    PDO_OCI_DIR=$PHP_PDO_OCI
  fi
  AC_MSG_RESULT($PDO_OCI_DIR :$PHP_PDO_OCI:)

  if test -d "$PDO_OCI_DIR/rdbms/public"; then
    PHP_ADD_INCLUDE($PDO_OCI_DIR/rdbms/public)
    PDO_OCI_INCLUDES="$PDO_OCI_INCLUDES -I$PDO_OCI_DIR/rdbms/public"
  fi
  if test -d "$PDO_OCI_DIR/rdbms/demo"; then
    PHP_ADD_INCLUDE($PDO_OCI_DIR/rdbms/demo)
    PDO_OCI_INCLUDES="$PDO_OCI_INCLUDES -I$PDO_OCI_DIR/rdbms/demo"
  fi
  if test -d "$PDO_OCI_DIR/network/public"; then
    PHP_ADD_INCLUDE($PDO_OCI_DIR/network/public)
    PDO_OCI_INCLUDES="$PDO_OCI_INCLUDES -I$PDO_OCI_DIR/network/public"
  fi
  if test -d "$PDO_OCI_DIR/plsql/public"; then
    PHP_ADD_INCLUDE($PDO_OCI_DIR/plsql/public)
    PDO_OCI_INCLUDES="$PDO_OCI_INCLUDES -I$PDO_OCI_DIR/plsql/public"
  fi

  if test -f "$PDO_OCI_DIR/lib/sysliblist"; then
    PHP_EVAL_LIBLINE(`cat $PDO_OCI_DIR/lib/sysliblist`, PDO_OCI_SYSLIB)
  elif test -f "$PDO_OCI_DIR/rdbms/lib/sysliblist"; then
    PHP_EVAL_LIBLINE(`cat $PDO_OCI_DIR/rdbms/lib/sysliblist`, PDO_OCI_SYSLIB)
  fi

  AC_PDO_OCI_VERSION($PDO_OCI_DIR)
  case $PDO_OCI_VERSION in
    8.0)
      PHP_ADD_LIBRARY_WITH_PATH(nlsrtl3, "", PDO_OCI_SHARED_LIBADD)
      PHP_ADD_LIBRARY_WITH_PATH(core4, "", PDO_OCI_SHARED_LIBADD)
      PHP_ADD_LIBRARY_WITH_PATH(psa, "", PDO_OCI_SHARED_LIBADD)
      PHP_ADD_LIBRARY_WITH_PATH(clntsh, $PDO_OCI_DIR/lib, PDO_OCI_SHARED_LIBADD)
      ;;

    8.1)
      PHP_ADD_LIBRARY(clntsh, 1, PDO_OCI_SHARED_LIBADD)
      PHP_ADD_LIBPATH($PDO_OCI_DIR/lib, PDO_OCI_SHARED_LIBADD)
      ;;

    9.0)
      PHP_ADD_LIBRARY(clntsh, 1, PDO_OCI_SHARED_LIBADD)
      PHP_ADD_LIBPATH($PDO_OCI_DIR/lib, PDO_OCI_SHARED_LIBADD)

      ;;
      
    10.1)
      PHP_ADD_LIBRARY(clntsh, 1, PDO_OCI_SHARED_LIBADD)
      PHP_ADD_LIBPATH($PDO_OCI_DIR/lib, PDO_OCI_SHARED_LIBADD)
      ;;
    *)
      AC_MSG_ERROR(Unsupported Oracle version!)
      ;;
  esac

  PHP_CHECK_LIBRARY(clntsh, OCIEnvCreate,
  [
    AC_DEFINE(HAVE_OCIENVCREATE,1,[ ])
  ], [], [
    -L$PDO_OCI_DIR/lib $PDO_OCI_SHARED_LIBADD
  ])

  PHP_CHECK_LIBRARY(clntsh, OCIEnvNlsCreate,
  [
    AC_DEFINE(HAVE_OCIENVNLSCREATE,1,[ ])
  ], [], [
    -L$PDO_OCI_DIR/lib $PDO_OCI_SHARED_LIBADD
  ])

  dnl
  dnl Check if we need to add -locijdbc8 
  dnl
  PHP_CHECK_LIBRARY(clntsh, OCILobIsTemporary,
  [
    AC_DEFINE(HAVE_OCILOBISTEMPORARY,1,[ ])
  ], [
    PHP_CHECK_LIBRARY(ocijdbc8, OCILobIsTemporary,
    [
      PHP_ADD_LIBRARY(ocijdbc8, 1, PDO_OCI_SHARED_LIBADD)
      AC_DEFINE(HAVE_OCILOBISTEMPORARY,1,[ ])
    ], [], [
      -L$PDO_OCI_DIR/lib $PDO_OCI_SHARED_LIBADD
    ])
  ], [
    -L$PDO_OCI_DIR/lib $PDO_OCI_SHARED_LIBADD
  ])

  dnl
  dnl Check if we have collections
  dnl
  PHP_CHECK_LIBRARY(clntsh, OCICollAssign,
  [
    AC_DEFINE(HAVE_OCICOLLASSIGN,1,[ ])
  ], [], [
    -L$PDO_OCI_DIR/lib $PDO_OCI_SHARED_LIBADD
  ])



  PHP_NEW_EXTENSION(pdo_oci, pdo_oci.c oci_driver.c oci_statement.c, $ext_shared,,-I\$prefix/include/php/ext)

  PHP_SUBST_OLD(PDO_OCI_SHARED_LIBADD)
  PHP_SUBST_OLD(PDO_OCI_DIR)
  PHP_SUBST_OLD(PDO_OCI_VERSION)
  
fi
