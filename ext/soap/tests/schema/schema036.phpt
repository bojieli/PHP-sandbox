--TEST--
SOAP XML Schema 36: Nested complex types (inline)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<sequence>
			<element name="int" type="int"/>
			<element name="testType2">
				<complexType>
					<sequence>
						<element name="int" type="int"/>
					</sequence>
				</complexType>
			</element>
		</sequence>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("int"=>123.5,"testType2"=>array("int"=>123.5)));
echo "ok";
?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://test-uri/" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><ns1:test><testParam xsi:type="ns1:testType"><int xsi:type="xsd:int">123</int><testType2 xsi:type="ns1:testType2"><int xsi:type="xsd:int">123</int></testType2></testParam></ns1:test></SOAP-ENV:Body></SOAP-ENV:Envelope>
object(stdClass)#5 (2) {
  ["int"]=>
  int(123)
  ["testType2"]=>
  object(stdClass)#6 (1) {
    ["int"]=>
    int(123)
  }
}
ok
