--TEST--
Test getimagesize() function : basic functionality for shockwave-flash
--SKIPIF--
<?php
	if (!defined("IMAGETYPE_SWC") || !extension_loaded('zlib')) {
		die("skip zlib extension is not available or SWC not supported");
	}
?>
--FILE--
<?php
/* Prototype  : array getimagesize(string imagefile [, array info])
 * Description: Get the size of an image as 4-element array 
 * Source code: ext/standard/image.c
 */

echo "*** Testing getimagesize() : basic functionality ***\n";

var_dump( getimagesize(dirname(__FILE__)."/test13pix.swf", $info) );
var_dump( $info );
?>
===DONE===
--EXPECTF--
*** Testing getimagesize() : basic functionality ***
array(5) {
  [0]=>
  int(550)
  [1]=>
  int(400)
  [2]=>
  int(13)
  [3]=>
  unicode(24) "width="550" height="400""
  [u"mime"]=>
  unicode(29) "application/x-shockwave-flash"
}
array(0) {
}
===DONE===
