--TEST--
array_walk() and objects
--FILE--
<?php

function walk($key, $value) { 
	var_dump($value, $key); 
}

class test {
	private $var_pri = "test_private";
	protected $var_pro = "test_protected";
	public $var_pub = "test_public";
}

$stdclass = new stdclass;
$stdclass->foo = "foo";
$stdclass->bar = "bar";
array_walk($stdclass, "walk");

$t = new test;
array_walk($t, "walk");

$var = array();
array_walk($var, "walk");
$var = "";
array_walk($var, "walk");

echo "Done\n";
?>
--EXPECTF--
unicode(3) "foo"
unicode(3) "foo"
unicode(3) "bar"
unicode(3) "bar"
unicode(13) " test var_pri"
unicode(12) "test_private"
unicode(10) " * var_pro"
unicode(14) "test_protected"
unicode(7) "var_pub"
unicode(11) "test_public"

Warning: array_walk(): The argument should be an array in %s on line %d
Done
