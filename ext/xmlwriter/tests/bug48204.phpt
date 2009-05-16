--TEST--
xmlwriter_open_uri with PHP_MAXPATHLEN + 1
--SKIPIF--
<?php if (!extension_loaded("xmlwriter")) print "skip"; ?>
--FILE--
<?php 
$path = str_repeat('a', PHP_MAXPATHLEN + 1);
var_dump(xmlwriter_open_uri('file:///' . $path));
?>
--CREDIT--
Koen Kuipers koenk82@gmail.com
Theo van der Zee
#Test Fest Utrecht 09-05-2009
--EXPECTF--

Warning: xmlwriter_open_uri(): Unable to resolve file path in %s on line %d
bool(false)
