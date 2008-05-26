--TEST--
Test readdir() function : usage variations - empty directories
--FILE--
<?php
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass readdir() a directory handle pointing to an empty directory to test behaviour
 */

echo "*** Testing readdir() : usage variations ***\n";

$path = dirname(__FILE__) . '/readdir_variation2';
mkdir($path);
$dir_handle = opendir($path);

echo "\n-- Pass an empty directory to readdir() --\n";
while(FALSE !== ($file = readdir($dir_handle))){
	var_dump($file);
}

closedir($dir_handle);
?>
===DONE===
--CLEAN--
<?php
$path = dirname(__FILE__) . '/readdir_variation2';
rmdir($path);
?>
--EXPECT--
*** Testing readdir() : usage variations ***

-- Pass an empty directory to readdir() --
unicode(1) "."
unicode(2) ".."
===DONE===
