--TEST--
Bug #32290 (calling call_user_func_array() ends in infinite loop within child class)
--FILE--
<?php

function my_error_handler($errno, $errstr, $errfile, $errline) {
	var_dump($errstr);
}

set_error_handler('my_error_handler');

class TestA
{
	public function doSomething($i)
	{
		echo __METHOD__ . "($this)\n";
		return --$i;
	}
}

class TestB extends TestA
{
	public function doSomething($i)
	{
		echo __METHOD__ . "($this)\n";
		$i++;
		if ($i >= 5) return 5;
		return call_user_func_array(array("TestA","doSomething"), array($i));
	}
}

$x = new TestB();
var_dump($x->doSomething(1));

?>
===DONE===
--EXPECTF--
string(54) "Object of class TestB could not be converted to string"
TestB::doSomething()
string(54) "Object of class TestB could not be converted to string"
TestA::doSomething()
int(1)
===DONE===
