--TEST--
aggregating default properties matching regular expression
--POST--
--GET--
--FILE--
<?php
include "./aggregate.lib.php";

$obj = new simple();
aggregate_properties_by_regexp($obj, 'helper', '/^my/');
print implode(',', array_keys(get_object_vars($obj)))."\n";
$obj2 = new simple();
aggregate_properties_by_regexp($obj2, 'helper', '/^my/', true);
print implode(',', array_keys(get_object_vars($obj2)))."\n";
?>
--EXPECT--
I'm alive!
simple_prop,my_prop
I'm alive!
simple_prop,your_prop,our_prop
