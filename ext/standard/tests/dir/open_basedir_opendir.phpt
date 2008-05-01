--TEST--
Test open_basedir configuration
--INI--
open_basedir=.
--FILE--
<?php
require_once "open_basedir.inc";
test_open_basedir_before("opendir");
test_open_basedir_error("opendir");     

$directory = dirname(__FILE__);
var_dump(opendir($directory."/test/ok/"));
var_dump(opendir($directory."/test/ok"));
var_dump(opendir($directory."/test/ok/../ok"));

test_open_basedir_after("opendir");?>
--CLEAN--
<?php
require_once "open_basedir.inc";
delete_directories();
?>
--EXPECTF--
*** Testing open_basedir configuration [opendir] ***
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

Warning: opendir(): open_basedir restriction in effect. File(../bad) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(../bad): failed to open dir: Operation not permitted in %s on line %d
bool(false)

Warning: opendir(): open_basedir restriction in effect. File(../bad/bad.txt) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(../bad/bad.txt): failed to open dir: Operation not permitted in %s on line %d
bool(false)

Warning: opendir(): open_basedir restriction in effect. File(..) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(..): failed to open dir: Operation not permitted in %s on line %d
bool(false)

Warning: opendir(): open_basedir restriction in effect. File(../) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(../): failed to open dir: Operation not permitted in %s on line %d
bool(false)

Warning: opendir(): open_basedir restriction in effect. File(/) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(/): failed to open dir: Operation not permitted in %s on line %d
bool(false)

Warning: opendir(): open_basedir restriction in effect. File(../bad/.) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(../bad/.): failed to open dir: Operation not permitted in %s on line %d
bool(false)

Warning: opendir(): open_basedir restriction in effect. File(%s/test/bad/bad.txt) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(%s/test/bad/bad.txt): failed to open dir: Operation not permitted in %s on line %d
bool(false)

Warning: opendir(): open_basedir restriction in effect. File(%s/test/bad/../bad/bad.txt) is not within the allowed path(s): (.) in %s on line %d

Warning: opendir(%s/test/bad/../bad/bad.txt): failed to open dir: Operation not permitted in %s on line %d
bool(false)
resource(%d) of type (stream)
resource(%d) of type (stream)
resource(%d) of type (stream)
*** Finished testing open_basedir configuration [opendir] ***

