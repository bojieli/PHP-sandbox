--TEST--
Bug #30209 ()
--FILE--
<?php

class Foo
{
	private $name = 'testBAR';

	public function testBAR()
	{
		try
		{
			$class  = new ReflectionClass($this);
			var_dump($this->name);
			$method = $class->getMethod($this->name);
			var_dump($this->name);
		}

		catch (Exception $e) {}
	}
}

$foo = new Foo;
$foo->testBAR();
?>
===DONE===
--EXPECTF--
string(7) "testBAR"
string(7) "testBAR"
===DONE===
