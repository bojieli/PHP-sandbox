--TEST--
oci_lob_read() and friends
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--ENV--
return "
ORACLE_HOME=".(isset($_ENV['ORACLE_HOME']) ? $_ENV['ORACLE_HOME'] : '')."
NLS_LANG=".(isset($_ENV['NLS_LANG']) ? $_ENV['NLS_LANG'] : '')."
";
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

var_dump($blob->write("test"));
var_dump($blob->tell());
var_dump($blob->seek(10, OCI_SEEK_CUR));
var_dump($blob->write("string"));
var_dump($blob->flush());

oci_commit($c);

$select_sql = "SELECT blob FROM ".$schema.$table_name." FOR UPDATE";
$s = oci_parse($c, $select_sql);
oci_execute($s, OCI_DEFAULT);

var_dump($row = oci_fetch_array($s));

var_dump($row[0]->read(-1));
var_dump($row[0]->read(10000));

require dirname(__FILE__).'/drop_table.inc';

echo "Done\n";

?>
--EXPECTF--
object(OCI-Lob)#%d (1) {
  ["descriptor"]=>
  resource(%d) of type (oci8 descriptor)
}
int(4)
int(4)
bool(true)
int(6)
bool(false)
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

Warning: OCI-Lob::read(): Length parameter must be greater than 0 in %s on line %d
bool(false)
string(20) "test          string"
Done
