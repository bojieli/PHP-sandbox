--TEST--
PDO_sqlite: Testing sqliteCreateAggregate()
--SKIPIF--
<?php if (!extension_loaded('pdo_sqlite')) print 'skip not loaded'; ?>
--FILE--
<?php

$db = new pdo('sqlite:memory');

$db->query('CREATE TABLE IF NOT EXISTS foobar (id INT AUTO INCREMENT, name TEXT)');

$db->query('INSERT INTO foobar VALUES (NULL, "PHP")');
$db->query('INSERT INTO foobar VALUES (NULL, "PHP6")');

function test_a(&$a, $b) { $a .= $b; return $a; }
function test_b(&$a) { return $a; }
$db->sqliteCreateAggregate('testing', 'test_a', 'test_b');


foreach ($db->query('SELECT testing(name) FROM foobar') as $row) {
	var_dump($row);
}

$db->query('DROP TABLE foobar');

?>
--EXPECTF--
array(2) {
  ["testing(name)"]=>
  %string|unicode%(2) "12"
  [0]=>
  %string|unicode%(2) "12"
}
