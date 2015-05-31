#!/bin/sh
rm -rf autom4te.cache configure Makefile
./buildconf
./configure \
    --enable-bcmath \
    --enable-blog \
    --with-bz2 \
    --disable-cgi \
    --disable-cli \
    --with-curl \
    --enable-daemon \
    --enable-debug \
    --enable-dom \
    --enable-exif \
    --enable-ftp \
    --enable-fpm \
    --with-gd \
    --with-geoip \
    --with-gnupg \
    --without-imap \
    --without-imap-ssl \
    --with-imagick \
    --with-jpeg-dir \
    --without-ldap \
    --without-ldap-sasl \
    --enable-mbregex \
    --enable-mbstring \
    --with-mcrypt \
    --enable-memcached \
    --disable-memcached-sasl \
    --with-mhash \
    --with-mysql \
    --enable-mysqlnd \
    --with-mysqli \
    --enable-opcache \
    --with-openssl \
    --enable-inline-optimization \
    --enable-inotify \
    --with-pdo-mysql \
    --enable-pdo \
    --without-pear \
    --disable-phar \
    --with-png-dir \
    --disable-posix \
    --disable-rpath \
    --with-pcre-regex \
    --without-snmp \
    --enable-soap \
    --enable-sandbox \
    --with-xmlrpc \
    --with-xsl \
    --with-zlib \
    --with-zlib-dir \
    --enable-zip

make -j8
