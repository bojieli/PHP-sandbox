--TEST--
Adobe Form Data Format functions
--SKIPIF--
<?php if (!extension_loaded("fdf")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php 
echo is_resource(fdf_create()) ? "OK" : "FAILURE";
?>
--EXPECT--
OK