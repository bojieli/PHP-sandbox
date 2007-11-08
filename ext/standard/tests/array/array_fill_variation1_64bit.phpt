--TEST--
Test array_fill() function : usage variations  - unexpected values for 'start_key' argument
--SKIPIF--
<?php
if (PHP_INT_SIZE != 8) die("skip this test is for 64bit platform only");
?>
--FILE--
<?php
/* Prototype  : proto array array_fill(int start_key, int num, mixed val)
 * Description: Create an array containing num elements starting with index start_key each initialized to val 
 * Source code: ext/standard/array.c
 */

/*
 * testing array_fill() by passing different unexpected value for 'start_key' argument
 */

echo "*** Testing array_fill() : usage variations ***\n";

// Initialise function arguments not being substituted 
$num = 2;
$val = 100;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//get a resource variable
$fp = fopen(__FILE__, "r");

//define a class
class test
{
  var $t = 10;
  function __toString()
  {
    return "testObject";
  }
}


//array of different values for 'start_key' argument
$values = array(

            // float values
  /* 1  */  10.5,
            -10.5,
            12.3456789000e10,
            12.34567890006E-10,
            .5,

            // array values
  /* 6  */  array(),
            array(0),
            array(1),
            array(1, 2),
            array('color' => 'red', 'item' => 'pen'),

            // null values
  /* 11 */  NULL,
            null,

            // boolean values 
  /* 13 */  true,
            false,
            TRUE,
            FALSE,

            // empty string 
  /* 17 */  "",
            '',

            // string values
  /* 19 */  "string",
            'string',

            // objects
  /* 21 */  new test(),

            // undefined  variable
            @$undefined_var,

            // unset variable 
            @$unset_var,

            // resource variable
  /* 24 */  $fp
);

// loop through each element of the array for start_key 
// check the working of array_fill()
echo "--- Testing array_fill() with different values for 'start_key' arg ---\n";
$counter = 1;
for($index = 0; $index < count($values); $index ++)
{
  echo "-- Iteration $counter --\n";
  $start_key = $values[$index];
 
  var_dump( array_fill($start_key,$num,$val) );
 
  $counter ++;
}

// close the resource used
fclose($fp);

echo "Done";
?>
--EXPECTF--
*** Testing array_fill() : usage variations ***
--- Testing array_fill() with different values for 'start_key' arg ---
-- Iteration 1 --
array(2) {
  [10]=>
  int(100)
  [11]=>
  int(100)
}
-- Iteration 2 --
array(2) {
  [-10]=>
  int(100)
  [0]=>
  int(100)
}
-- Iteration 3 --
array(2) {
  [123456789000]=>
  int(100)
  [123456789001]=>
  int(100)
}
-- Iteration 4 --
array(2) {
  [0]=>
  int(100)
  [1]=>
  int(100)
}
-- Iteration 5 --
array(2) {
  [0]=>
  int(100)
  [1]=>
  int(100)
}
-- Iteration 6 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 7 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 8 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 9 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 10 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 11 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 12 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 13 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 14 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 15 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 16 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 17 --
array(2) {
  [0]=>
  int(100)
  [1]=>
  int(100)
}
-- Iteration 18 --
array(2) {
  [0]=>
  int(100)
  [1]=>
  int(100)
}
-- Iteration 19 --
array(2) {
  [0]=>
  int(100)
  [1]=>
  int(100)
}
-- Iteration 20 --
array(2) {
  [0]=>
  int(100)
  [1]=>
  int(100)
}
-- Iteration 21 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 22 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 23 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
-- Iteration 24 --

Warning: array_fill(): Wrong data type for start key in %s on line %d
bool(false)
Done
