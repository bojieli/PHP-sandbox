--TEST--
Test array_values() function
--SKIPIF--
<?php
if (PHP_INT_SIZE != 8) die("skip this test is for 64bit platform only");
?>
--INI--
precision=14
--FILE--
<?php
/* Prototype: array array_values ( array $input );
   Discription: array_values() returns all the values from the input array 
                and indexes numerically the array
*/

echo "*** Testing array_values() on basic array ***\n"; 
$basic_arr = array( 1, 2, 2.0, "asdasd", array(1,2,3) );
var_dump( array_values($basic_arr) );

echo "\n*** Testing array_values() on various arrays ***";
$arrays = array (
  array(), 
  array(0),
  array(-1),
  array( array() ),
  array("Hello"),
  array(""),  
  array("", array()),
  array(1,2,3), 
  array(1,2,3, array()),
  array(1,2,3, array(4,6)),
  array("a" => 1, "b" => 2, "c" =>3),
  array(0 => 0, 1 => 1, 2 => 2),  
  array(TRUE, FALSE, NULL, true, false, null, "TRUE", "FALSE",
        "NULL", "\x000", "\000"),
  array("Hi" => 1, "Hello" => 2, "World" => 3),
  array("a" => "abcd", "a" => "", "ab" => -6, "cd" => -0.5 ),
  array(0 => array(), 1=> array(0), 2 => array(1), ""=> array(), ""=>"" )
);

$i = 0;
/* loop through to test array_values() with different arrays given above */
foreach ($arrays as $array) {
  echo "\n-- Iteration $i --\n";
  var_dump( array_values($array) );
  $i++;
}

echo "\n*** Testing array_values() with resource type ***\n";
$resource1 = fopen(__FILE__, "r");  // Creating a file resource
$resource2 = opendir(".");  // Creating a dir resource

/* creating an array with resources as elements */
$arr_resource = array( "a" => $resource1, "b" => $resource2);
var_dump( array_values($arr_resource) );

echo "\n*** Testing array_values() with range checking ***\n";
$arr_range = array(
  2147483647, 
  2147483648, 
  -2147483647,
  -2147483648,
  -0,
  0,
  -2147483649
);
var_dump( array_values($arr_range) );

echo "\n*** Testing array_values() on an array created on the fly ***\n";
var_dump( array_values(array(1,2,3)) );
var_dump( array_values(array()) );  // null array

echo "\n*** Testing error conditions ***\n";
/* Invalid number of args */
var_dump( array_values() );  // Zero arguments
var_dump( array_values(array(1,2,3), "") );  // No. of args > expected
/* Invalid types */
var_dump( array_values("") );  // Empty string
var_dump( array_values(100) );  // Integer
var_dump( array_values(new stdclass) );  // object

echo "Done\n";

--CLEAN--
/* Closing the resource handles */
fclose( $resource1 );
closedir( $resource2 );
?>
--EXPECTF--	
*** Testing array_values() on basic array ***
array(5) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  float(2)
  [3]=>
  string(6) "asdasd"
  [4]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
}

*** Testing array_values() on various arrays ***
-- Iteration 0 --
array(0) {
}

-- Iteration 1 --
array(1) {
  [0]=>
  int(0)
}

-- Iteration 2 --
array(1) {
  [0]=>
  int(-1)
}

-- Iteration 3 --
array(1) {
  [0]=>
  array(0) {
  }
}

-- Iteration 4 --
array(1) {
  [0]=>
  string(5) "Hello"
}

-- Iteration 5 --
array(1) {
  [0]=>
  string(0) ""
}

-- Iteration 6 --
array(2) {
  [0]=>
  string(0) ""
  [1]=>
  array(0) {
  }
}

-- Iteration 7 --
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}

-- Iteration 8 --
array(4) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  array(0) {
  }
}

-- Iteration 9 --
array(4) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  array(2) {
    [0]=>
    int(4)
    [1]=>
    int(6)
  }
}

-- Iteration 10 --
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}

-- Iteration 11 --
array(3) {
  [0]=>
  int(0)
  [1]=>
  int(1)
  [2]=>
  int(2)
}

-- Iteration 12 --
array(11) {
  [0]=>
  bool(true)
  [1]=>
  bool(false)
  [2]=>
  NULL
  [3]=>
  bool(true)
  [4]=>
  bool(false)
  [5]=>
  NULL
  [6]=>
  string(4) "TRUE"
  [7]=>
  string(5) "FALSE"
  [8]=>
  string(4) "NULL"
  [9]=>
  string(2) " 0"
  [10]=>
  string(1) " "
}

-- Iteration 13 --
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}

-- Iteration 14 --
array(3) {
  [0]=>
  string(0) ""
  [1]=>
  int(-6)
  [2]=>
  float(-0.5)
}

-- Iteration 15 --
array(4) {
  [0]=>
  array(0) {
  }
  [1]=>
  array(1) {
    [0]=>
    int(0)
  }
  [2]=>
  array(1) {
    [0]=>
    int(1)
  }
  [3]=>
  string(0) ""
}

*** Testing array_values() with resource type ***
array(2) {
  [0]=>
  resource(5) of type (stream)
  [1]=>
  resource(6) of type (stream)
}

*** Testing array_values() with range checking ***
array(7) {
  [0]=>
  int(2147483647)
  [1]=>
  int(2147483648)
  [2]=>
  int(-2147483647)
  [3]=>
  int(-2147483648)
  [4]=>
  int(0)
  [5]=>
  int(0)
  [6]=>
  int(-2147483649)
}

*** Testing array_values() on an array created on the fly ***
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}
array(0) {
}

*** Testing error conditions ***

Warning: Wrong parameter count for array_values() in %s on line %d
NULL

Warning: Wrong parameter count for array_values() in %s on line %d
NULL

Warning: array_values(): The argument should be an array in %s on line %d
NULL

Warning: array_values(): The argument should be an array in %s on line %d
NULL

Warning: array_values(): The argument should be an array in %s on line %d
NULL
Done
