--TEST--
Testing str_shuffle.
--FILE--
<?php
/* Do not change this test it is a README.TESTING example. */
$s = '123';
var_dump(str_shuffle($s));
var_dump($s);
?>
--EXPECTF--
string(3) %s
string(3) "123"
--UEXPECTF--
unicode(3) "%s"
unicode(3) "123"
