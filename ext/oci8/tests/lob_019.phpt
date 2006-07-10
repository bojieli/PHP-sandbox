--TEST--
oci_lob_write()/erase()/read() with BLOBs
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php
	
require dirname(__FILE__).'/connect.inc';
require dirname(__FILE__).'/create_table.inc';

$ora_sql = "INSERT INTO
                       ".$schema.$table_name." (blob)
                      VALUES (empty_blob())
                      RETURNING
                               blob
                      INTO :v_blob ";

$statement = oci_parse($c,$ora_sql);
$blob = oci_new_descriptor($c,OCI_D_LOB);
oci_bind_by_name($statement,":v_blob", $blob,-1,OCI_B_BLOB);
oci_execute($statement, OCI_DEFAULT);

var_dump($blob);

$str = "this is a biiiig faaat test string. why are you reading it, I wonder? =)";
var_dump($blob->write($str));
var_dump($blob->erase(0, 10));

oci_commit($c);

$select_sql = "SELECT blob FROM ".$schema.$table_name." FOR UPDATE";
$s = oci_parse($c, $select_sql);
oci_execute($s, OCI_DEFAULT);

var_dump($row = oci_fetch_array($s));

var_dump($row[0]->read(5));
var_dump($row[0]->read(5));
var_dump($row[0]->read(5));

require dirname(__FILE__).'/drop_table.inc';

echo "Done\n";

?>
--EXPECTF--
object(OCI-Lob)#%d (1) {
  ["descriptor"]=>
  resource(%d) of type (oci8 descriptor)
}
int(72)
int(10)
array(2) {
  [0]=>
  object(OCI-Lob)#%d (1) {
    ["descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
  ["BLOB"]=>
  object(OCI-Lob)#%d (1) {
    ["descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
string(5) "     "
string(5) "     "
string(5) "biiii"
Done
--UEXPECTF--
object(OCI-Lob)#%d (1) {
  [u"descriptor"]=>
  resource(%d) of type (oci8 descriptor)
}
int(72)
int(10)
array(2) {
  [0]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
  [u"BLOB"]=>
  object(OCI-Lob)#%d (1) {
    [u"descriptor"]=>
    resource(%d) of type (oci8 descriptor)
  }
}
string(5) "     "
string(5) "     "
string(5) "biiii"
Done
