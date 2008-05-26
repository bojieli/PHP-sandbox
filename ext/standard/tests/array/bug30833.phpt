--TEST--
Bug #30833 (array_count_values() modifies input array)
--FILE--
<?php

$foo = array('abc', '0000');
var_dump($foo);

$count = array_count_values( $foo );
var_dump($count);

var_dump($foo);

echo "Done\n";
?>
--EXPECT--
array(2) {
  [0]=>
  unicode(3) "abc"
  [1]=>
  unicode(4) "0000"
}
array(2) {
  [u"abc"]=>
  int(1)
  [u"0000"]=>
  int(1)
}
array(2) {
  [0]=>
  unicode(3) "abc"
  [1]=>
  unicode(4) "0000"
}
Done
