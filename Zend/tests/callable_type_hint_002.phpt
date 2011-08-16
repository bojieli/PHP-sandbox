--TEST--
callable type hint#002 - Reflection
--FILE--
<?php

class bar {
    static function foo(callable $arg) {}
}
function foo(callable $bar) {
}
$closure = function (callable $arg) {};

$rf = new ReflectionFunction("foo");
var_dump($rf->getParameters()[0]->isCallable());

$rm = new ReflectionMethod("bar", "foo");
var_dump($rm->getParameters()[0]->isCallable());

$rc = new ReflectionFunction($closure);
var_dump($rc->getParameters()[0]->isCallable());

?>
--EXPECT--
bool(true)
bool(true)
bool(true)

