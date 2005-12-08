--TEST--
oci_bind_array_by_name() and SQLT_FLT
--SKIPIF--
<?php if (!extension_loaded('oci8')) die("skip no oci8 extension"); ?>
--FILE--
<?php

require dirname(__FILE__).'/connect.inc';

$drop = "DROP table bind_test";
$statement = oci_parse($c, $drop);
@oci_execute($statement);

$create = "CREATE table bind_test(name FLOAT)";
$statement = oci_parse($c, $create);
oci_execute($statement);

$create_pkg = "
CREATE OR REPLACE PACKAGE ARRAYBINDPKG1 AS 
  TYPE ARRTYPE IS TABLE OF FLOAT INDEX BY BINARY_INTEGER; 
  PROCEDURE iobind(c1 IN OUT ARRTYPE); 
END ARRAYBINDPKG1;";
$statement = oci_parse($c, $create_pkg);
oci_execute($statement);

$create_pkg_body = "
CREATE OR REPLACE PACKAGE BODY ARRAYBINDPKG1 AS 
  CURSOR CUR IS SELECT name FROM bind_test;
  PROCEDURE iobind(c1 IN OUT ARRTYPE) IS
    BEGIN
    FOR i IN 1..5 LOOP
      INSERT INTO bind_test VALUES (c1(i));
    END LOOP;
    IF NOT CUR%ISOPEN THEN
      OPEN CUR;
    END IF;
    FOR i IN REVERSE 1..5 LOOP
      FETCH CUR INTO c1(i);
      IF CUR%NOTFOUND THEN
        CLOSE CUR;
        EXIT;
      END IF;
    END LOOP;
  END iobind;
END ARRAYBINDPKG1;";
$statement = oci_parse($c, $create_pkg_body);
oci_execute($statement);

$statement = oci_parse($c, "BEGIN ARRAYBINDPKG1.iobind(:c1); END;");

$array = Array(1.243,2.5658,3.4234,4.2123,5.9999);

oci_bind_array_by_name($statement, ":c1", $array, 10, 5, SQLT_FLT);

oci_execute($statement);

var_dump($array);

echo "Done\n";
?>
--EXPECT--	
array(5) {
  [0]=>
  float(5.9999)
  [1]=>
  float(4.2123)
  [2]=>
  float(3.4234)
  [3]=>
  float(2.5658)
  [4]=>
  float(1.243)
}
Done
