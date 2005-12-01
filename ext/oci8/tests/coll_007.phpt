--TEST--
collection methods
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--ENV--
return "
ORACLE_HOME=".(isset($_ENV['ORACLE_HOME']) ? $_ENV['ORACLE_HOME'] : '')."
NLS_LANG=".(isset($_ENV['NLS_LANG']) ? $_ENV['NLS_LANG'] : '')."
TNS_ADMIN=".(isset($_ENV['TNS_ADMIN']) ? $_ENV['TNS_ADMIN'] : '')."
";
--FILE--
<?php

require dirname(__FILE__)."/connect.inc";
require dirname(__FILE__)."/create_type.inc";

$coll1 = ocinewcollection($c, $type_name);

var_dump($coll1->size());
var_dump($coll1->max());
var_dump($coll1->trim(3));
var_dump($coll1->append(1));
var_dump($coll1->getElem(0));
var_dump($coll1->assignElem(0,2));

echo "Done\n";

require dirname(__FILE__)."/drop_type.inc";

?>
--EXPECTF--
int(0)
int(0)

Warning: OCI-Collection::trim(): OCI-22167: given trim size [3] must be less than or equal to [0] in %s on line %d
bool(false)
bool(true)
float(1)
bool(true)
Done
