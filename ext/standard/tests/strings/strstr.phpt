--TEST--
Test strstr() function
--FILE--
<?php
/* Prototype: string strstr ( string $haystack, string $needle );
   Description: Find first occurrence of a string 
   and reurns the rest of the string from that string 
*/

echo "*** Testing basic functionality of strstr() ***\n";
var_dump( strstr("test string", "test") );
var_dump( strstr("test string", "string") );
var_dump( strstr("test string", "strin") );
var_dump( strstr("test string", "t s") );
var_dump( strstr("test string", "g") );
var_dump( md5(strstr("te".chr(0)."st", chr(0))) );
var_dump( strstr("tEst", "test") );
var_dump( strstr("teSt", "test") );
var_dump( @strstr("", "") );
var_dump( @strstr("a", "") );
var_dump( @strstr("", "a") );


echo "\n*** Testing strstr() with various needles ***";
$string = 
"Hello world,012033 -3.3445     NULL TRUE FALSE\0 abcd\xxyz \x000 octal\n 
abcd$:Hello world";

/* needles in an array to get the string starts with needle, in $string */
$needles = array(
  "Hello world", 	
  "WORLD", 
  "\0", 
  "\x00", 
  "\x000", 
  "abcd", 
  "xyz", 
  "octal", 
  "-3", 
  -3, 
  "-3.344", 
  -3.344, 
  NULL, 
  "NULL",
  "0",
  0, 
  TRUE, 
  "TRUE",
  "1",
  1,
  FALSE,
  "FALSE",
  " ",
  "     ",
  'b',
  '\n',
  "\n",
  "12",
  "12twelve",
  $string
);

/* loop through to get the string starts with "needle" in $string */
for( $i = 0; $i < count($needles); $i++ ) {
  echo "\n-- Iteration $i --\n";
  var_dump( strstr($string, $needles[$i]) );
}  

	
echo "\n*** Testing Miscelleneous input data ***\n";

echo "-- Passing objects as string and needle --\n";
/* we get "Catchable fatal error: saying Object of class needle could not be 
converted to string" by default when an object is passed instead of string:
The error can be  avoided by chosing the __toString magix method as follows: */

class string 
{
  function __toString() {
    return "Hello, world";
  }
}
$obj_string = new string;

class needle 
{
  function __toString() {
    return "world";
  }
}
$obj_needle = new needle;

var_dump(strstr("$obj_string", "$obj_needle"));	


echo "\n-- passing an array as string and needle --\n";
$needles = array("hello", "?world", "!$%**()%**[][[[&@#~!");
var_dump( strstr($needles, $needles) );  // won't work
var_dump( strstr("hello?world,!$%**()%**[][[[&@#~!", "$needles[1]") );  // works
var_dump( strstr("hello?world,!$%**()%**[][[[&@#~!", "$needles[2]") );  // works


echo "\n-- passing Resources as string and needle --\n"; 
$resource1 = fopen(__FILE__, "r");
$resource2 = opendir(".");
var_dump( strstr($resource1, $resource1) );
var_dump( strstr($resource1, $resource2) );


echo "\n-- Posiibilities with null --\n";
var_dump( strstr("", NULL) );
var_dump( strstr(NULL, NULL) );
var_dump( strstr("a", NULL) );
var_dump( strstr("/x0", "0") );  // Hexadecimal NUL

echo "\n-- A longer and heredoc string --\n";
$string = <<<EOD
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
EOD;
var_dump( strstr($string, "abcd") );
var_dump( strstr($string, "1234") );		

echo "\n-- A heredoc null string --\n";
$str = <<<EOD
EOD;
var_dump( strstr($str, "\0") );
var_dump( strstr($str, NULL) );
var_dump( strstr($str, "0") );


echo "\n-- simple and complex syntax strings --\n";
$needle = 'world';

/* Simple syntax */
var_dump( strstr("Hello, world", "$needle") );  // works 
var_dump( strstr("Hello, world'S", "$needle'S") );  // works
var_dump( strstr("Hello, worldS", "$needleS") );  // won't work 

/* String with curly braces, complex syntax */
var_dump( strstr("Hello, worldS", "${needle}S") );  // works
var_dump( strstr("Hello, worldS", "{$needle}S") );  // works


echo "\n-- complex strings containing other than 7-bit chars --\n";
$str = chr(0).chr(128).chr(129).chr(234).chr(235).chr(254).chr(255);
echo "- Positions of some chars in the string '$str' are as follows -\n";
echo chr(128)." => "; 
var_dump( strstr($str, chr(128)) );		
echo chr(255)." => "; 
var_dump( strstr($str, chr(255)) );
echo chr(256)." => "; 
var_dump( strstr($str, chr(256)) ); 

echo "\n*** Testing error conditions ***";
var_dump( strstr($string, ""));
var_dump( strstr() );  // zero argument
var_dump( strstr("") );  // null argument 
var_dump( strstr($string) );  // without "needle"
var_dump( strstr("a", "b", "c") );  // args > expected
var_dump( strstr(NULL, "") );

/* Cleaning the resources */
fclose($resource1);
closedir($resource2);

echo "\nDone";
?>
--EXPECTF--
*** Testing basic functionality of strstr() ***
unicode(11) "test string"
unicode(6) "string"
unicode(6) "string"
unicode(8) "t string"
unicode(1) "g"
unicode(32) "7272696018bdeb2c9a3f8d01fc2a9273"
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

*** Testing strstr() with various needles ***
-- Iteration 0 --
unicode(86) "Hello world,012033 -3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 1 --
bool(false)

-- Iteration 2 --
unicode(40) "  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 3 --
unicode(40) "  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 4 --
unicode(28) " 0 octal
 
abcd$:Hello world"

-- Iteration 5 --
unicode(38) "abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 6 --
unicode(32) "xyz  0 octal
 
abcd$:Hello world"

-- Iteration 7 --
unicode(25) "octal
 
abcd$:Hello world"

-- Iteration 8 --
unicode(67) "-3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 9 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)

-- Iteration 10 --
unicode(67) "-3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 11 --

Warning: Needle argument codepoint value out of range (0 - 0x10FFFF) in %s on line %d
bool(false)

-- Iteration 12 --
unicode(40) "  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 13 --
unicode(55) "NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 14 --
unicode(74) "012033 -3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 15 --
unicode(40) "  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 16 --
bool(false)

-- Iteration 17 --
unicode(50) "TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 18 --
unicode(73) "12033 -3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 19 --
bool(false)

-- Iteration 20 --
unicode(40) "  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 21 --
unicode(45) "FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 22 --
unicode(81) " world,012033 -3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 23 --
unicode(60) "     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 24 --
unicode(37) "bcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 25 --
bool(false)

-- Iteration 26 --
unicode(20) "
 
abcd$:Hello world"

-- Iteration 27 --
unicode(73) "12033 -3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

-- Iteration 28 --
bool(false)

-- Iteration 29 --
unicode(86) "Hello world,012033 -3.3445     NULL TRUE FALSE  abcd\xxyz  0 octal
 
abcd$:Hello world"

*** Testing Miscelleneous input data ***
-- Passing objects as string and needle --
unicode(5) "world"

-- passing an array as string and needle --

Warning: strstr() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
NULL
unicode(27) "?world,!$%**()%**[][[[&@#~!"
unicode(20) "!$%**()%**[][[[&@#~!"

-- passing Resources as string and needle --

Warning: strstr() expects parameter 1 to be string (Unicode or binary), resource given in %s on line %d
NULL

Warning: strstr() expects parameter 1 to be string (Unicode or binary), resource given in %s on line %d
NULL

-- Posiibilities with null --
bool(false)
bool(false)
bool(false)
unicode(1) "0"

-- A longer and heredoc string --
unicode(729) "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"
unicode(702) "123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789
abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789"

-- A heredoc null string --
bool(false)
bool(false)
bool(false)

-- simple and complex syntax strings --
unicode(5) "world"
unicode(7) "world'S"

Notice: Undefined variable: needleS in %s on line %d

Warning: strstr(): Empty delimiter in %s on line %d
bool(false)
unicode(6) "worldS"
unicode(6) "worldS"

-- complex strings containing other than 7-bit chars --
- Positions of some chars in the string ' êëþÿ' are as follows -
 => unicode(6) "êëþÿ"
ÿ => unicode(1) "ÿ"
Ā => bool(false)

*** Testing error conditions ***
Warning: strstr(): Empty delimiter in %s on line %d
bool(false)

Warning: strstr() expects at least 2 parameters, 0 given in %s on line %d
NULL

Warning: strstr() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: strstr() expects at least 2 parameters, 1 given in %s on line %d
NULL
bool(false)

Warning: strstr(): Empty delimiter in %s on line %d
bool(false)

Done
