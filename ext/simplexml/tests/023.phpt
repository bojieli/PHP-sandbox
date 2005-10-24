--TEST--
SimpleXML: Attributes with entities
--SKIPIF--
<?php if (!extension_loaded("simplexml")) print "skip"; ?>
--FILE--
<?php 

$xml =<<<EOF
<?xml version='1.0'?>
<!DOCTYPE talks SYSTEM "nbsp.dtd" [
<!ELEMENT root  EMPTY>
<!ATTLIST root  attr1 CDATA #IMPLIED>
<!ENTITY  nbsp   "&#38;#x00A0;">
]>
<root attr='foo&nbsp;bar&nbsp;baz'></root>
EOF;

$sxe = simplexml_load_string($xml);

var_dump($sxe);
var_dump($sxe['attr']);
?>
===DONE===
--EXPECTF--
object(SimpleXMLElement)#%d (0) {
}
object(SimpleXMLElement)#%d (1) {
  [0]=>
  string(%d) "foo%sbar%sbaz"
}
===DONE===
