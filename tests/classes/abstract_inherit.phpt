--TEST--
A class that inherits an abstract method is abstract
--SKIPIF--
<?php if (version_compare(zend_version(), '2.0.0-dev', '<')) die('skip ZendEngine 2 needed'); ?>
--FILE--
<?php

abstract class pass {
	abstract function show();
}

abstract class fail extends pass {
}

$t = new fail();
$t = new pass();

echo "Done\n"; // Shouldn't be displayed
?>
--EXPECTF--

Fatal error: Cannot instantiate abstract class fail in %s on line %d 
