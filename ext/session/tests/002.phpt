--TEST--
session_unset() without any session
--SKIPIF--
<?php include('./ext/session/tests/skipif.inc'); ?>
--FILE--
<?php
session_unset();
print "ok\n";
--GET--
--POST--
--EXPECT--
ok
