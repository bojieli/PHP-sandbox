--TEST--
Phar::apiVersion
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--FILE--
<?php
var_dump(Phar::apiVersion());
?>
===DONE===
--EXPECT--
string(5) "1.1.0"
===DONE===
