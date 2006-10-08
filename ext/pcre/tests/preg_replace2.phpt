--TEST--
preg_replace()
--FILE--
<?php

var_dump(preg_replace('', array(), ''));

var_dump(preg_replace(array('/\da(.)/ui', '@..@'), '$1', '12Abc'));
var_dump(preg_replace(array('/\da(.)/ui', '@(.)@'), '$1', array('x','a2aA', '1av2Ab')));


var_dump(preg_replace(array('/[\w]+/'), array('$'), array('xyz', 'bdbd')));
var_dump(preg_replace(array('/\s+/', '~[b-d]~'), array('$'), array('x y', 'bd bc')));

echo "==done==\n";

?>
--EXPECTF--
Warning: preg_replace(): Parameter mismatch, pattern is a string while replacement in an array in %spreg_replace2.php on line 3
bool(false)
string(1) "c"
array(3) {
  [0]=>
  string(1) "x"
  [1]=>
  string(2) "aA"
  [2]=>
  string(2) "vb"
}
array(2) {
  [0]=>
  string(1) "$"
  [1]=>
  string(1) "$"
}
array(2) {
  [0]=>
  string(3) "x$y"
  [1]=>
  string(1) "$"
}
==done==
--UEXPECTF--
Warning: preg_replace(): Parameter mismatch, pattern is a string while replacement in an array in %s on line %d
bool(false)
unicode(1) "c"
array(3) {
  [0]=>
  unicode(1) "x"
  [1]=>
  unicode(2) "aA"
  [2]=>
  unicode(2) "vb"
}
array(2) {
  [0]=>
  unicode(1) "$"
  [1]=>
  unicode(1) "$"
}
array(2) {
  [0]=>
  unicode(3) "x$y"
  [1]=>
  unicode(1) "$"
}
==done==
