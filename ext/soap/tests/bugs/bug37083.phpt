--TEST--
Bug #37083 (Frequent crashs in SOAP extension with new WSDL caching code in multithread WS)
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--INI--
soap.wsdl_cache=3
--FILE--
<?php
class TestSoapClient extends SoapClient {
	function __doRequest($request, $location, $action, $version, $one_way = 0) {
		return <<<EOF
<?xml version="1.0" encoding="utf-8"?>
<soapenv:Envelope xmlns:soapenv="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
<soapenv:Body>
<ns1:searchResponse soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:ns1="urn:java:de.pangaea.metadataportal.search.SearchService">
<searchReturn href="#id0"/>
</ns1:searchResponse>
<multiRef id="id0" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns2:SearchResponse" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/" xmlns:ns2="urn:java:de.pangaea.metadataportal.search.SearchService"><offset xsi:type="xsd:int">0</offset><queryTime xsi:type="xsd:long">34</queryTime><results soapenc:arrayType="ns2:SearchResponseItem[10]" xsi:type="soapenc:Array"><results href="#id1"/><results href="#id2"/><results href="#id3"/><results href="#id4"/><results href="#id5"/><results href="#id6"/><results href="#id7"/><results href="#id8"/><results href="#id9"/><results href="#id10"/></results><totalCount xsi:type="xsd:int">3501</totalCount></multiRef>
<multiRef id="id9" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns3:SearchResponseItem" xmlns:ns3="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id11"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:5900168</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml1</xml></multiRef>
<multiRef id="id7" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns4:SearchResponseItem" xmlns:ns4="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id12"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:5900039</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml2</xml></multiRef>
<multiRef id="id6" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns5:SearchResponseItem" xmlns:ns5="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id13"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:5900040</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml3</xml></multiRef>
<multiRef id="id10" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns6:SearchResponseItem" xmlns:ns6="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id14"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:41534</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml4</xml></multiRef>
<multiRef id="id8" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns7:SearchResponseItem" xmlns:ns7="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id15"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:5900038</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml5</xml></multiRef>
<multiRef id="id4" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns8:SearchResponseItem" xmlns:ns8="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id16"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:2900229</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml6</xml></multiRef>
<multiRef id="id5" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns9:SearchResponseItem" xmlns:ns9="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id17"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:2900228</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml7</xml></multiRef>
<multiRef id="id3" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns10:SearchResponseItem" xmlns:ns10="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id18"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:2900230</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml8</xml></multiRef>
<multiRef id="id2" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns11:SearchResponseItem" xmlns:ns11="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id19"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:2900235</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml9</xml></multiRef>
<multiRef id="id1" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns12:SearchResponseItem" xmlns:ns12="urn:java:de.pangaea.metadataportal.search.SearchService" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><fields href="#id20"/><identifier xsi:type="xsd:string">oai:dlmd.ifremer.fr:5900196</identifier><score xsi:type="xsd:float">0.13684115</score><xml xsi:type="xsd:string">xml10</xml></multiRef>
<multiRef id="id15" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns13:Map" xmlns:ns13="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2004-12-01T04:58:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">105.539</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">112.283</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-30.024</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-34.788</value></value></item></multiRef>
<multiRef id="id20" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns14:Map" xmlns:ns14="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2004-06-07T17:41:44.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">129.882</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">133.635</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">39.529</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">36.419</value></value></item></multiRef>
<multiRef id="id18" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns15:Map" xmlns:ns15="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2004-03-10T01:40:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">63.121</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">80.92</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">0.158</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-3.675</value></value></item></multiRef>
<multiRef id="id14" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns16:Map" xmlns:ns16="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2003-01-12T05:15:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-42.88</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-20.85</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">61.41</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">43.2</value></value></item></multiRef>
<multiRef id="id13" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns17:Map" xmlns:ns17="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2004-12-01T16:50:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">108.962</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">114.713</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-35.262</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-42.756</value></value></item></multiRef>
<multiRef id="id17" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns18:Map" xmlns:ns18="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2003-10-10T11:52:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">68.14</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">95.987</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">3.97</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-1.242</value></value></item></multiRef>
<multiRef id="id16" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns19:Map" xmlns:ns19="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2004-01-10T10:35:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">67.71</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">77.743</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">3.546</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-1.965</value></value></item></multiRef>
<multiRef id="id19" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns20:Map" xmlns:ns20="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2004-04-09T23:00:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">59.116</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">69.206</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-2.274</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-4.093</value></value></item></multiRef>
<multiRef id="id11" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns21:Map" xmlns:ns21="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2002-11-09T23:10:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">156.013</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">160.042</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">5.648</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">4.773</value></value></item></multiRef>
<multiRef id="id12" soapenc:root="0" soapenv:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xsi:type="ns22:Map" xmlns:ns22="http://xml.apache.org/xml-soap" xmlns:soapenc="http://schemas.xmlsoap.org/soap/encoding/"><item><key xsi:type="soapenc:string">maxDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2038-12-31T22:59:59.000Z</value></value></item><item><key xsi:type="soapenc:string">minDateTime</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="xsd:dateTime">2004-11-01T16:58:00.000Z</value></value></item><item><key xsi:type="soapenc:string">minLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">108.11</value></value></item><item><key xsi:type="soapenc:string">maxLongitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">113.383</value></value></item><item><key xsi:type="soapenc:string">maxLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-31.666</value></value></item><item><key xsi:type="soapenc:string">minLatitude</key><value soapenc:arrayType="xsd:anyType[1]" xsi:type="soapenc:Array"><value xsi:type="soapenc:double">-35.075</value></value></item></multiRef>
</soapenv:Body></soapenv:Envelope>
EOF;
	}
}
for ($i = 0; $i < 10; $i++) {
	$ws=new TestSoapClient(dirname(__FILE__).'/bug37083.wsdl',
                   array('encoding'=>'ISO-8859-1',
                         'cache_wsdl'=>WSDL_CACHE_BOTH));
	$search=new stdClass();
	$search->queryString='argo';
	$search->ranges[]=$r=new stdClass();
	$r->field='maxDateTime';
	$r->min='2003-04-01';
	$search->index='all';
	$res=$ws->search($search,0,10);
}
echo "ok\n";
?>
--EXPECT--
ok
