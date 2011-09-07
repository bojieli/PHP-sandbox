--TEST--
Test is_dir() function: usage variations - invalid arguments 
--FILE--
<?php
/* Prototype: bool is_dir ( string $dirname );
   Description: Tells whether the dirname is a directory
     Returns TRUE if the dirname exists and is a directory, FALSE  otherwise.
*/

/* Passing invalid arguments to is_dir() */

$dir_handle = opendir( dirname(__FILE__) );

echo "*** Testing is_dir() with Invalid arguments: expected bool(false) ***\n";
$dirnames = array(
  /* Invalid dirnames */
  -2.34555,
  TRUE,
  FALSE,
  NULL,
  " ",
  $dir_handle,

  /* scalars */
  0,
  1234
);

/* loop through to test each element the above array */
foreach($dirnames as $dirname) {
  var_dump( is_dir($dirname) );
}
closedir($dir_handle);

echo "\n*** Done ***";
?>
--EXPECTF--
*** Testing is_dir() with Invalid arguments: expected bool(false) ***
bool(false)
bool(false)
bool(false)
bool(false)

%s: is_dir() expects parameter 1 to be a valid path, string given in G:\php-sdk\php-src\branches\PHP_5_4\ext\standard\tests\file\is_dir_variation3.php on line %d
NULL

%s: is_dir() expects parameter 1 to be a valid path, resource given in G:\php-sdk\php-src\branches\PHP_5_4\ext\standard\tests\file\is_dir_variation3.php on line %d
NULL
bool(false)
bool(false)

*** Done ***

