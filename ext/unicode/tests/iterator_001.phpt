--TEST--
Unicode: Basic iterator tests
--SKIPIF--
<?php if (!unicode_semantics()) die('skip unicode.semantics=off'); ?>
--FILE--
<?php

foreach(new TextIterator("123") as $char)
{
	var_dump($char);
}

foreach(new ReverseTextIterator("123") as $char)
{
	var_dump($char);
}

?>
===DONE===
--EXPECTF--
unicode(1) "1"
unicode(1) "2"
unicode(1) "3"
unicode(1) "3"
unicode(1) "2"
unicode(1) "1"
===DONE===
