dnl $Id$
dnl config.m4 for extension db
dnl don't forget to call PHP_EXTENSION(db)

divert(2)dnl

AC_CHECK_HEADERS(db1/ndbm.h)

# Checks for libraries.
# Prefer gdbm, Berkeley DB and ndbm/dbm, in that order
AC_DEFUN(AC_PREFERRED_DB_LIB,[
  AC_CHECK_LIB(gdbm, gdbm_open,[AC_DEFINE(GDBM) DBM_TYPE=gdbm; DBM_LIB=-lgdbm],
  [AC_CHECK_LIB(db, dbm_open,[AC_DEFINE(NDBM) DBM_TYPE=ndbm; DBM_LIB=-ldb],
   [AC_CHECK_LIB(c, dbm_open,[AC_DEFINE(NDBM) DBM_TYPE=ndbm; DBM_LIB=],
    [AC_CHECK_LIB(dbm, dbm_open,[AC_DEFINE(NDBM) DBM_TYPE=ndbm; DBM_LIB=-ldbm],
     [DBM_TYPE=""])])])])
  AC_MSG_CHECKING([preferred dbm library])
  if test "a$DBM_TYPE" = a; then
    AC_MSG_RESULT(none found)
    AC_MSG_WARN(No dbm library found - using built-in flatfile support)
  else
    AC_MSG_RESULT($DBM_TYPE chosen)
  fi
  PHP_SUBST(DBM_LIB)
  PHP_SUBST(DBM_TYPE)
])

AC_PREFERRED_DB_LIB

divert(4)dnl

if test "$DBM_LIB" = "-lgdbm"; then
  AC_CHECK_HEADER(gdbm.h, [ GDBM_INCLUDE="" ], [ 
    AC_MSG_RESULT("Try /usr/local/include/gdbm.h"); 
    AC_CHECK_HEADER(/usr/local/include/gdbm.h, [ GDBM_INCLUDE="-I/usr/local/include" ],[
      AC_MSG_RESULT("Try /opt/local/include/gdbm.h");
      AC_CHECK_HEADER(/opt/local/include/gdbm.h, [ GDBM_INCLUDE="-I/opt/local/include" ],[
        dnl if in /usr/pkg/include, do not add anything.  See above.
        AC_MSG_RESULT("Try /usr/pkg/include/gdbm.h");
        AC_CHECK_HEADER(/usr/pkg/include/gdbm.h, [ GDBM_INCLUDE="" ],[
          AC_MSG_RESULT("Giving up - You need to install gdbm.h somewhere");
          exit
        ])
      ])
    ])  
  ])
fi

if test -n "$DBM_LIB"; then
  INCLUDES="$INCLUDES $GDBM_INCLUDE"
  EXTRA_LIBS="$EXTRA_LIBS $DBM_LIB"
fi

PHP_EXTENSION(db)

