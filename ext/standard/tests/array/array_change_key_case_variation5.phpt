--TEST--
Test array_change_key_case() function : usage variations - position of internal pointer
--FILE--
<?php
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased] 
 * Source code: ext/standard/array.c
 */

/*
 * Check the position of the internal array pointer after calling the function
 */

echo "*** Testing array_change_key_case() : usage variations ***\n";

$input = array ('one' => 'un', 'two' => 'deux', 'three' => 'trois');

echo "\n-- Call array_change_key_case() --\n";
var_dump($result = array_change_key_case($input, CASE_UPPER));

echo "-- Position of Internal Pointer in Result: --\n";
echo key($result) . " => " . current($result) . "\n";
echo "\n-- Position of Internal Pointer in Original Array: --\n";
echo key($input) . " => " . current ($input) . "\n";

echo "Done";
?>

--EXPECTF--
*** Testing array_change_key_case() : usage variations ***

-- Call array_change_key_case() --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}
-- Position of Internal Pointer in Result: --
ONE => un

-- Position of Internal Pointer in Original Array: --
one => un
Done
