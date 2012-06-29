--TEST--
IntlBreakIterator::first(): basic test
--SKIPIF--
if (!extension_loaded('intl'))
	die('skip intl extension not enabled');
--FILE--
<?php
ini_set("intl.error_level", E_WARNING);

$bi = IntlBreakIterator::createWordInstance('pt');
$bi->setText('foo bar trans');

var_dump($bi->current());
var_dump($bi->next());
var_dump($bi->first());
var_dump($bi->current());
--EXPECT--
int(0)
int(3)
int(0)
int(0)
