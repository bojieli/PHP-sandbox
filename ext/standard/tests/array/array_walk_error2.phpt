--TEST--
Test array_walk() function : error conditions - callback parameters(Bug#43558)
--FILE--
<?php
/* Prototype  : bool array_walk(array $input, string $funcname [, mixed $userdata])
 * Description: Apply a user function to every member of an array 
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_walk() by passing more number of parameters to callback function
 */
$input = array(1);

function callback1($value, $key, $user_data ) {
  echo "\ncallback1() invoked \n";
}

function callback2($value, $key, $user_data1, $user_data2) {
  echo "\ncallback2() invoked \n";
}
echo "*** Testing array_walk() : error conditions - callback parameters ***\n";

// expected: Missing argument Warning
var_dump( array_walk($input, "callback1") );
var_dump( array_walk($input, "callback2", 4) );

// expected: Warning is supressed
var_dump( @array_walk($input, "callback1") );  
var_dump( @array_walk($input, "callback2", 4) );  

echo "-- Testing array_walk() function with too many callback parameters --\n";
var_dump( array_walk($input, "callback1", 20, 10) );

echo "Done";
?>
--EXPECTF--
*** Testing array_walk() : error conditions - callback parameters ***

Warning: Missing argument 3 for callback1() in %s on line %d

callback1() invoked 
bool(false)

Warning: Missing argument 4 for callback2() in %s on line %d

callback2() invoked 
bool(false)

callback1() invoked 
bool(true)

callback2() invoked 
bool(true)
-- Testing array_walk() function with too many callback parameters --

Warning: array_walk() expects at most 3 parameters, 4 given in %s on line %d
NULL
Done
