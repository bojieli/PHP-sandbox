--TEST--
Phar front controller with no extension [cache_list]
--INI--
default_charset=
phar.cache_list={PWD}/frontcontroller27.php
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller27.php
REQUEST_URI=/frontcontroller27.php/noext
PATH_INFO=/noext
--FILE_EXTERNAL--
files/frontcontroller8.phar
--EXPECTHEADERS--
Content-type: text/plain
--EXPECTF--
hi