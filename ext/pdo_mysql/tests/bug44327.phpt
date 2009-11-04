--TEST--
Bug #44327 (PDORow::queryString property & numeric offsets / Crash)
--SKIPIF--
<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'skipif.inc');
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
MySQLPDOTest::skip();
$db = MySQLPDOTest::factory();
?>
--FILE--
<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();

	$stmt = $db->prepare("SELECT 1 AS \"one\""); 
	$stmt->execute(); 
	$row = $stmt->fetch(PDO::FETCH_LAZY); 
	var_dump($row);
	var_dump($row->{0});
	var_dump($row->one); 
	var_dump($row->queryString);

	print "----------------------------------\n";

	@$db->exec("DROP TABLE test");
	$db->exec("CREATE TABLE test (id INT)");
	$db->exec("INSERT INTO test(id) VALUES (1)");
	$stmt = $db->prepare("SELECT id FROM test");
	$stmt->execute();
	$row = $stmt->fetch(PDO::FETCH_LAZY);
	var_dump($row);
	var_dump($row->queryString);
	@$db->exec("DROP TABLE test");

	print "----------------------------------\n";

	$stmt = $db->prepare('foo'); 
	@$stmt->execute();
	$row = $stmt->fetch();
	var_dump($row->queryString);
	
?>
--EXPECTF--
object(PDORow)#%d (2) {
  [%u|b%"queryString"]=>
  %unicode|string%(17) "SELECT 1 AS "one""
  [%u|b%"one"]=>
  %unicode|string%(1) "1"
}
%unicode|string%(1) "1"
%unicode|string%(1) "1"
%unicode|string%(17) "SELECT 1 AS "one""
----------------------------------
object(PDORow)#%d (2) {
  [%u|b%"queryString"]=>
  %unicode|string%(19) "SELECT id FROM test"
  [%u|b%"id"]=>
  %unicode|string%(1) "1"
}
%unicode|string%(19) "SELECT id FROM test"
----------------------------------

Notice: Trying to get property of non-object in %s on line %d
NULL
