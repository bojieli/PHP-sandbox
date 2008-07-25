--TEST--
SQLite3 open_basedir / safe_mode checks
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--INI--
open_basedir=.
--FILE--
<?php
$directory = dirname(__FILE__) . '/';
$file = uniqid() . '.db';

echo "Within test directory\n";
$db = new SQLite3($directory . $file);
var_dump($db);
var_dump($db->close());
unlink($directory . $file);

echo "Above test directory\n";
$db = new SQLite3('../bad' . $file);
var_dump($db);

echo "Done\n";
?>
--EXPECTF--
Within test directory
object(SQLite3)#%d (0) {
}
bool(true)
Above test directory

Warning: SQLite3::__construct(): open_basedir restriction in effect. File(%s) is not within the allowed path(s): (.) in %s on line %d
object(SQLite3)#%d (0) {
}
Done
