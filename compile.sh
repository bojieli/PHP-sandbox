./buildconf --force
./configure --disable-phar --enable-fpm --with-mcrypt --with-zlib --enable-mbstring --disable-pdo --with-mysql --with-mysqli --with-gd --with-jpeg-dir --with-png-dir --with-zlib-dir --disable-debug --disable-rpath --enable-inline-optimization --with-zlib --enable-mbregex --with-mhash --enable-zip --with-pcre-regex --enable-soap --enable-daemon --enable-sandbox --enable-blog --with-config-file-path=/usr/local/lib/php
make -j5
