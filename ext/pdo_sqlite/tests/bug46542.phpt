--TEST--
Bug #46542 Extending PDO class with a __call() function
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pdo_sqlite')) print 'skip not loaded';
?>
--FILE--
<?php
class A extends PDO
{ function __call($m, $p) {print __CLASS__."::$m\n";} }

$a = new A('sqlite:dummy.db');

$a->truc();
$a->TRUC();

?>
--EXPECT--
A::truc
A::TRUC
