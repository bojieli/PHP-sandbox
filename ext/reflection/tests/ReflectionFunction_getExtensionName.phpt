--TEST--
ReflectionFunction::getExtensionName()
--FILE--
<?php
function foo() {}

$function = new ReflectionFunction('sort');
var_dump($function->getExtensionName());

$function = new ReflectionFunction('foo');
var_dump($function->getExtensionName());
?>
--EXPECT--
unicode(8) "standard"
bool(false)

