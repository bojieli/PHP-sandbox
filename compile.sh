./buildconf --force
./configure --enable-blog --enable-fpm --with-mcrypt --with-zlib --enable-mbstring --disable-pdo --with-mysql --with-mysqli --with-gd --with-jpeg-dir --with-png-dir --with-zlib-dir --disable-debug --disable-rpath --enable-inline-optimization --with-zlib --enable-mbregex --with-mhash --enable-zip --with-pcre-regex --enable-soap
make -j5
