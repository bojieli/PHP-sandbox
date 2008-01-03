--TEST--
Test array_map() function : usage variations - object functionality
--FILE--
<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Testing array_map() for object functionalities:
 *   1) simple class with variable and method
 *   2) class without members
 *   3) class with only one method and no variable
 *   4) abstract and child class
 *   5) class with static and final members
 *   6) interface and implemented class
 */
echo "*** Testing array_map() : object functionality ***\n";

echo "-- simple class with public variable and method --\n";
class SimpleClass
{
  public $var1 = 1;
  public function square($n) {
    return $n * $n;
  }
}
var_dump( array_map(array('SimpleClass', 'square'), array(1, 2)) );

echo "\n-- simple class with private variable and method --\n";
class SimpleClassPri
{
  private $var1 = 10;
  private function add($n) {
    return $var + $n;
  }
}
var_dump( array_map(array('SimpleClassPri', 'add'), array(1)) );

echo "\n-- simple class with protected variable and method --\n";
class SimpleClassPro
{
  protected $var1 = 5;
  protected function mul($n) {
    return $var1 * $n;
  }
}
var_dump( array_map(array('SimpleClassPro', 'mul'), array(2)) );

echo "\n-- class without members --";
class EmptyClass
{
}
var_dump( array_map(array('EmptyClass'), array(1, 2)) );

echo "\n-- abstract class --";
abstract class AbstractClass
{
  protected $var2 = 5;
  abstract function emptyFunction();
}

// class deriving the above abstract class
class ChildClass extends AbstractClass
{
  private $var3;
  public function emptyFunction() {
    echo "defined in child";
  }
}
var_dump( array_map(array('ChildClass', 'emptyFunction'), array(1, 2)) );

echo "\n-- class with final method --";
class FinalClass
{
  private $var4;
  final function finalMethod() {
    echo "This function can't be overloaded";
  }
}
var_dump( array_map(array('FinalClass', 'finalMethod'), array(1, 2)) );

echo "\n-- class with static members --\n";
class StaticClass
{
  static $var5 = 2;
  public static function square($n) {
    return ($n * $n);
  }
  private static function cube($n) {
    return ($n * $n * $n);
  }
  protected static function retVal($n)  {
    return array($n);
  }
}
var_dump( array_map(array('StaticClass', 'square'), array(1, 2)) );
var_dump( array_map(array('StaticClass', 'cube'), array(2)) );
var_dump( array_map(array('StaticClass', 'retVal'), array(3, 4)) );

echo "-- class implementing an interface --\n";
interface myInterface
{
  public function toImplement();
}
class InterClass implements myInterface
{
  public static function square($n) {
    return ($n * $n);
  }
  public function toImplement() {
    return 1;
  }
}
var_dump( array_map(array('InterClass', 'square'), array(1, 2)));

echo "Done";
?>
--EXPECTF--
*** Testing array_map() : object functionality ***
-- simple class with public variable and method --

Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL

-- simple class with private variable and method --

Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL

-- simple class with protected variable and method --

Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL

-- class without members --
Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL

-- abstract class --
Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL

-- class with final method --
Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL

-- class with static members --
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(4)
}

Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL

Warning: array_map() expects parameter 1 to be valid callback, array given in %s on line %d%d
NULL
-- class implementing an interface --
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(4)
}
Done
