--TEST--
Phar: copy-on-write test 1 [cache_list]
--INI--
default_charset=UTF-8
phar.cache_list={PWD}/copyonwrite1.phar.php
phar.readonly=0
detect_unicode=0
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--FILE_EXTERNAL--
files/write.phar
--EXPECT--
hi
changed
ok
