--TEST--
Phar web-based phar with fatal error
--INI--
default_charset=UTF-8
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/fatal_error_webphar.php
REQUEST_URI=/fatal_error_webphar.php/index.php
PATH_INFO=/index.php
--FILE_EXTERNAL--
files/pear2coverage.phar.php
--EXPECTHEADERS--
Content-type: text/html; charset=UTF-8
--EXPECTF--
string(9) "\Web\View"

Parse error: syntax error, unexpected %s, expecting %s in phar:///%sfatal_error_webphar.php/Web/View.php on line 380
