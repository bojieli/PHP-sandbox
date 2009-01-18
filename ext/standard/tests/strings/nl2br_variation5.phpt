--TEST--
Test nl2br() function : usage variations - unexpected values for 'str' argument
--FILE--
<?php
/* Prototype  : string nl2br(string $str)
 * Description: Inserts HTML line breaks before all newlines in a string.
 * Source code: ext/standard/string.c
*/

/*
* Test nl2br() function by passing different types of values other than 
*   expected type for 'str' argument
*/

echo "*** Testing nl2br() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//getting resource
$file_handle = fopen(__FILE__, "r");

//defining class
class Sample {
  public function __toString() {
    return "My String";
  }
}

//array of values to iterate over
$values = array(

  // int data
  0,
  1,
  12345,
  -2345,

  // float data
  10.5,
  -10.5,
  10.1234567e10,
  10.7654321E-10,
  .5,

  // array data
  array(),
  array(0),
  array(1),
  array(1, 2),
  array('color' => 'red', 'item' => 'pen'),

  // null data
  NULL,
  null,

  // boolean data
  true,
  false,
  TRUE,
  FALSE,

  //resource
  $file_handle,

  // object data
  new Sample(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,
);

// loop through $values array to test nl2br() function with each element 
$count = 1;
foreach($values as $value) {
  echo "-- Iteration $count --\n";
  var_dump( nl2br($value) );
  $count ++ ;
};

//closing the file handle
fclose( $file_handle );

?>
===DONE===
--EXPECTF--
*** Testing nl2br() : usage variations ***
-- Iteration 1 --
string(1) "0"
-- Iteration 2 --
string(1) "1"
-- Iteration 3 --
string(5) "12345"
-- Iteration 4 --
string(5) "-2345"
-- Iteration 5 --
string(4) "10.5"
-- Iteration 6 --
string(5) "-10.5"
-- Iteration 7 --
string(12) "101234567000"
-- Iteration 8 --
string(13) "1.07654321E-9"
-- Iteration 9 --
string(3) "0.5"
-- Iteration 10 --

Notice: Array to string conversion in %s on line %d
string(5) "Array"
-- Iteration 11 --

Notice: Array to string conversion in %s on line %d
string(5) "Array"
-- Iteration 12 --

Notice: Array to string conversion in %s on line %d
string(5) "Array"
-- Iteration 13 --

Notice: Array to string conversion in %s on line %d
string(5) "Array"
-- Iteration 14 --

Notice: Array to string conversion in %s on line %d
string(5) "Array"
-- Iteration 15 --
string(0) ""
-- Iteration 16 --
string(0) ""
-- Iteration 17 --
string(1) "1"
-- Iteration 18 --
string(0) ""
-- Iteration 19 --
string(1) "1"
-- Iteration 20 --
string(0) ""
-- Iteration 21 --
string(%d) "Resource id #%d"
-- Iteration 22 --
string(9) "My String"
-- Iteration 23 --
string(0) ""
-- Iteration 24 --
string(0) ""
===DONE===