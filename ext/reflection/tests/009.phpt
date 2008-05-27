--TEST--
ReflectionMethod::__construct() tests
--FILE--
<?php

$a = array("", 1, "::", "a::", "::b", "a::b");

foreach ($a as $val) {
	try {
		new ReflectionMethod($val);
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}
}
 
$a = array("", 1, "");
$b = array("", "", 1);
 
foreach ($a as $key=>$val) {
	try {
		new ReflectionMethod($val, $b[$key]);
	} catch (Exception $e) {
		var_dump($e->getMessage());
	}
}

echo "Done\n";
?>
--EXPECT--	
unicode(20) "Invalid method name "
unicode(21) "Invalid method name 1"
unicode(21) "Class  does not exist"
unicode(22) "Class a does not exist"
unicode(21) "Class  does not exist"
unicode(22) "Class a does not exist"
unicode(21) "Class  does not exist"
unicode(66) "The parameter class is expected to be either a string or an object"
unicode(21) "Class  does not exist"
Done
