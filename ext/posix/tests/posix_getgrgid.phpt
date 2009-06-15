--TEST--
Test posix_getgrgid().
--CREDITS--
Till Klampaeckel, till@php.net
TestFest Berlin 2009
--SKIPIF--
<?php
if (!extension_loaded('posix')) {
    die('SKIP The posix extension is not loaded.');
}
?>
--FILE--
<?php
$grp = posix_getgrgid(0);
if (!isset($grp[b'name'])) {
    die('Array index "name" does not exist.');
}
if (!isset($grp[b'passwd'])) {
    die('Array index "passwd" does not exist.');
}
if (!isset($grp[b'members'])) {
    die('Array index "members" does not exist.');
} elseif (!is_array($grp[b'members'])) {
    die('Array index "members" must be an array.');
} else {
    if (count($grp[b'members']) > 0) {
        foreach ($grp[b'members'] as $idx => $username) {
            if (!is_int($idx)) {
                die('Index in members Array is not an int.');
            }
            if (!is_string($username)) {
                die('Username in members Array is not of type string.');
            }
        }
    }
}
if (!isset($grp[b'gid'])) {
    die('Array index "gid" does not exist.');
}
var_dump($grp[b'gid']);
?>
===DONE===
--EXPECT--
int(0)
===DONE===
