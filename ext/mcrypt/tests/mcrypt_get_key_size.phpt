--TEST--
mcrypt_get_key_size
--SKIPIF--
<?php if (!extension_loaded("mcrypt")) print "skip"; ?>
--FILE--
<?php
var_dump(mcrypt_get_key_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_CBC));
var_dump(mcrypt_get_key_size(MCRYPT_3DES, MCRYPT_MODE_CBC));
var_dump(mcrypt_get_key_size(MCRYPT_WAKE, MCRYPT_MODE_STREAM));
--EXPECT--
int(32)
int(24)
int(32)
