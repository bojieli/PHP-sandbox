--TEST--
023: __NAMESPACE__ constant
--FILE--
<?php
namespace test\foo;

var_dump(__NAMESPACE__);
--EXPECT--
unicode(8) "test\foo"
