--TEST--
sqlite: sqlite_open/close
--SKIPIF--
<?php if (!extension_loaded("sqlite")) print "skip"; ?>
--FILE--
<?php 
require_once('blankdb.inc');
echo "$db\n";
sqlite_close($db);
$db = NULL;
echo "Done\n";
?>
--EXPECTF--
Resource id #%d
Done
