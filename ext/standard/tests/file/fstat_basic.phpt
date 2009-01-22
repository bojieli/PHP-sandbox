--TEST--
Test function fstat() by calling it with its expected arguments
--FILE--
<?php
$fp = fopen (__FILE__, 'r');
var_dump(fstat( $fp ) );
fclose($fp);
?>
===DONE===
--EXPECTF--
array(26) {
  [0]=>
  int(%i)
  [1]=>
  int(%i)
  [2]=>
  int(%i)
  [3]=>
  int(%i)
  [4]=>
  int(%i)
  [5]=>
  int(%i)
  [6]=>
  int(%i)
  [7]=>
  int(%i)
  [8]=>
  int(%i)
  [9]=>
  int(%i)
  [10]=>
  int(%i)
  [11]=>
  int(%i)
  [12]=>
  int(%i)
  [u"dev"]=>
  int(%i)
  [u"ino"]=>
  int(%i)
  [u"mode"]=>
  int(%i)
  [u"nlink"]=>
  int(%i)
  [u"uid"]=>
  int(%i)
  [u"gid"]=>
  int(%i)
  [u"rdev"]=>
  int(%i)
  [u"size"]=>
  int(%i)
  [u"atime"]=>
  int(%i)
  [u"mtime"]=>
  int(%i)
  [u"ctime"]=>
  int(%i)
  [u"blksize"]=>
  int(%i)
  [u"blocks"]=>
  int(%i)
}
===DONE===