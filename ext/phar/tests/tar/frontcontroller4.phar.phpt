--TEST--
Phar front controller index.php relocate (no /) tar-based
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller4.phar.php
REQUEST_URI=/frontcontroller4.phar.php
--FILE_EXTERNAL--
frontcontroller.phar.tar
--EXPECTHEADERS--
Status: 301 Moved Permanently
Location: /frontcontroller4.phar.php/index.php
--EXPECT--
