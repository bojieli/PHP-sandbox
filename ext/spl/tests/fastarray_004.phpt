--TEST--
SPL: FastArray: adding new elements
--FILE--
<?php

$a = new SplFastArray(10);

try {
	$a[] = 1;
} catch (Exception $e) {
	var_dump($e->getMessage());
}

?>
===DONE===
--EXPECTF--
string(29) "Index invalid or out of range"
===DONE===