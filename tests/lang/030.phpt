--TEST--
$this in constructor test
--POST--
--GET--
--FILE--
<?php
class foo {
	function foo($name) {
     	$GLOBALS['List']= &$this;
     	$this->Name = $name;
		$GLOBALS['List']->echoName(); }

	function echoName() {
     	$GLOBALS['names'][]=$this->Name; } }

function &foo2(&$foo)	{
	return $foo; }


$bar1 = new foo('constructor');
$bar1->Name = 'outside';
$bar1->echoName();

$bar1 = foo2(new foo('constructor'));
$bar1->Name = 'outside';
$bar1->echoName();

$List->echoName();

print ($names==array('constructor','constructor','constructor','constructor','constructor')) ? 'success:':'failure';
?>
--EXPECT--
success
