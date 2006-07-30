--TEST--
PDO Common: PHP Bug #38253: PDO produces segfault with default fetch mode
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded('pdo')) die('skip');
$dir = getenv('REDIR_TEST_DIR');
if (false == $dir) die('skip no driver');
require_once $dir . 'pdo_test.inc';
PDOTest::skip();
?>
--FILE--
<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$pdo = PDOTest::factory();

$pdo->exec ("create table test (id integer primary key, n text)");
$pdo->exec ("INSERT INTO test (n) VALUES ('hi')");

$pdo->setAttribute (PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_CLASS);
$stmt = $pdo->prepare ("SELECT * FROM test");
$stmt->execute();
var_dump($stmt->fetchAll());

$pdo = PDOTest::factory();

$pdo->exec ("create table test2 (id integer primary key, n text)");
$pdo->exec ("INSERT INTO test2 (n) VALUES ('hi')");

$pdo->setAttribute (PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_FUNC);
$stmt = $pdo->prepare ("SELECT * FROM test2");
$stmt->execute();
var_dump($stmt->fetchAll());

?>
--EXPECTF--
Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error: No fetch class specified in %s on line %d

Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error%s on line %d
array(0) {
}

Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error: No fetch function specified in %s on line %d

Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error%s on line %d
array(0) {
}
