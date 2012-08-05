--TEST--
Phar front controller $_SERVER munging failure 3
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--INI--
detect_unicode=0
--ENV--
SCRIPT_NAME=/frontcontroller20.php
REQUEST_URI=/frontcontroller20.php/
PATH_INFO=/
--FILE_EXTERNAL--
files/frontcontroller11.phar
--EXPECTF--
Fatal error: Uncaught exception 'PharException' with message 'Non-string value passed to Phar::mungServer(), expecting an array of any of these strings: PHP_SELF, REQUEST_URI, SCRIPT_FILENAME, SCRIPT_NAME' in %sfrontcontroller20.php:2
Stack trace:
#0 %sfrontcontroller20.php(2): Phar::mungServer(Array)
#1 {main}
  thrown in %sfrontcontroller20.php on line 2
