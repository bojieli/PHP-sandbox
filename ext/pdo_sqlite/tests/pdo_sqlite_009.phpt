--TEST--
PDO-SQLite: PDO_FETCH_CLASSTYPE
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded("pdo_sqlite")) print "skip"; ?>
--FILE--
<?php

$db =new pdo('sqlite::memory:');

$db->exec('CREATE TABLE classtypes(id int PRIMARY KEY, name VARCHAR(10) UNIQUE)');
$db->exec('INSERT INTO classtypes VALUES(0, "stdClass")'); 
$db->exec('INSERT INTO classtypes VALUES(1, "Test1")'); 
$db->exec('INSERT INTO classtypes VALUES(2, "Test2")'); 
$db->exec('CREATE TABLE test(id int PRIMARY KEY, classtype int, val VARCHAR(10))');
$db->exec('INSERT INTO test VALUES(1, 0, "A")'); 
$db->exec('INSERT INTO test VALUES(2, 1, "B")'); 
$db->exec('INSERT INTO test VALUES(3, 2, "C")'); 
$db->exec('INSERT INTO test VALUES(4, 3, "D")'); 

class Test1
{
	public function __construct()
	{
		echo __METHOD__ . "()\n";
	}
}

class Test2
{
	public function __construct()
	{
		echo __METHOD__ . "()\n";
	}
}

class Test3
{
	public function __construct()
	{
		echo __METHOD__ . "()\n";
	}
}

$sql = 'SELECT classtypes.name, test.id AS id, test.val AS val FROM test LEFT JOIN classtypes ON test.classtype=classtypes.id';
var_dump($db->query($sql)->fetchAll(PDO_FETCH_NUM));
var_dump($db->query($sql)->fetchAll(PDO_FETCH_CLASS|PDO_FETCH_CLASSTYPE, 'Test3'));
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
array(4) {
  [0]=>
  array(3) {
    [0]=>
    string(8) "stdClass"
    [1]=>
    string(1) "1"
    [2]=>
    string(1) "A"
  }
  [1]=>
  array(3) {
    [0]=>
    string(5) "Test1"
    [1]=>
    string(1) "2"
    [2]=>
    string(1) "B"
  }
  [2]=>
  array(3) {
    [0]=>
    string(5) "Test2"
    [1]=>
    string(1) "3"
    [2]=>
    string(1) "C"
  }
  [3]=>
  array(3) {
    [0]=>
    NULL
    [1]=>
    string(1) "4"
    [2]=>
    string(1) "D"
  }
}
Test1::__construct()
Test2::__construct()
Test3::__construct()
array(4) {
  [0]=>
  object(stdClass)#%d (2) {
    ["id"]=>
    string(1) "1"
    ["val"]=>
    string(1) "A"
  }
  [1]=>
  object(Test1)#%d (2) {
    ["id"]=>
    string(1) "2"
    ["val"]=>
    string(1) "B"
  }
  [2]=>
  object(Test2)#%d (2) {
    ["id"]=>
    string(1) "3"
    ["val"]=>
    string(1) "C"
  }
  [3]=>
  object(Test3)#%d (2) {
    ["id"]=>
    string(1) "4"
    ["val"]=>
    string(1) "D"
  }
}
===DONE===
