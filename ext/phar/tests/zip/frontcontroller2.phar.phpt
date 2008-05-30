--TEST--
Phar front controller PHP test zip-based
--INI--
default_charset=
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
<?php if (!extension_loaded("zlib")) die("skip zlib not available"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller2.phar.php
REQUEST_URI=/frontcontroller2.phar.php/a.php
PATH_INFO=/a.php
--FILE_EXTERNAL--
files/frontcontroller.phar.zip
--EXPECTHEADERS--
Content-type: text/html
--EXPECT--
hio