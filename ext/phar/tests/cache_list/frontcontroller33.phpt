--TEST--
Phar front controller with valid callback that does not return any value [cache_list]
--INI--
default_charset=
phar.cache_list={PWD}/frontcontroller33.php
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller33.php
REQUEST_URI=/frontcontroller33.php
--EXPECTHEADERS--
Content-type: text/html
--FILE_EXTERNAL--
files/frontcontroller18.phar
--EXPECTF--
phar error: rewrite callback must return a string or false