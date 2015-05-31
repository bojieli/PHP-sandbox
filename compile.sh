#!/bin/sh
rm -rf autom4te.cache
./buildconf
./configure --disable-phar --enable-fpm --with-openssl --with-mcrypt --with-zlib --enable-mbstring --disable-pdo --with-mysql --without-mysqli --with-gd --with-jpeg-dir --with-png-dir --with-zlib-dir --disable-rpath --enable-inline-optimization --with-zlib --enable-mbregex --with-mhash --enable-zip --with-pcre-regex --enable-soap --enable-daemon --enable-sandbox --enable-blog --disable-debug --with-imagick --with-curl --enable-opcache
make -j8
