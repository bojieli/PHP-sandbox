dnl
dnl $Id$
dnl

PHP_ARG_ENABLE(xml,whether to enable XML support,
[  --disable-xml           Disable XML support.], yes)

PHP_ARG_WITH(libxml-dir, libxml install dir,
[  --with-libxml-dir=DIR     XML: libxml install prefix], no, no)

PHP_ARG_WITH(libexpat-dir, libexpat install dir,
[  --with-libexpat-dir=DIR   XML: libexpat install prefix (deprecated)], no, no)

if test "$PHP_XML" != "no"; then
  dnl 
  dnl Default to libxml2.
  dnl
  PHP_SETUP_LIBXML(XML_SHARED_LIBADD, [
    xml_extra_sources="compat.c"
  ], [
    if test "$PHP_EXPAT_DIR" = "no"; then
      AC_MSG_ERROR(xml2-config not found. Use --with-libxml-dir=<DIR>)
    fi
  ])

  dnl
  dnl Check for expat only if --with-libexpat-dir is used.
  dnl
  if test "$PHP_LIBEXPAT_DIR" != "no"; then
    for i in $PHP_XML $PHP_LIBEXPAT_DIR; do
      if test -f "$i/lib/libexpat.a" -o -f "$i/lib/libexpat.$SHLIB_SUFFIX_NAME"; then
        EXPAT_DIR=$i
      fi
    done

    if test -z "$EXPAT_DIR"; then
      AC_MSG_ERROR(not found. Please reinstall the expat distribution.)
    fi

    PHP_ADD_INCLUDE($EXPAT_DIR/include)
    PHP_ADD_LIBRARY_WITH_PATH(expat, $EXPAT_DIR/lib, XML_SHARED_LIBADD)
    AC_DEFINE(HAVE_LIBEXPAT, 1, [ ])
  fi

  PHP_NEW_EXTENSION(xml, xml.c $xml_extra_sources, $ext_shared)
  PHP_SUBST(XML_SHARED_LIBADD)
  AC_DEFINE(HAVE_XML, 1, [ ])
fi
