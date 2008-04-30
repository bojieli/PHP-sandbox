--TEST--
Phar front controller with cwd
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller34.php
REQUEST_URI=/frontcontroller34.php/start/index.php
PATH_INFO=/start/index.php
--EXPECTHEADERS--
Content-type: text/html
--FILE_EXTERNAL--
files/frontcontroller19.phar
--EXPECT--
start/index.php
start/another.php
another.php
