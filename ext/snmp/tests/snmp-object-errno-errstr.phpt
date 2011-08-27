--TEST--                                 
OO API: getErrno & getError methods
--CREDITS--
Boris Lytochkin
--SKIPIF--
<?php
require_once(dirname(__FILE__).'/skipif.inc');
?>
--FILE--
<?php
require_once(dirname(__FILE__).'/snmp_include.inc');

//EXPECTF format is quickprint OFF
snmp_set_enum_print(false);
snmp_set_quick_print(false);
snmp_set_valueretrieval(SNMP_VALUE_PLAIN);
snmp_set_oid_output_format(SNMP_OID_OUTPUT_FULL);

echo "SNMP::ERRNO_NOERROR\n";
$session = new SNMP(SNMP::VERSION_2c, $hostname, $community, $timeout, $retries);
var_dump(@$session->get('.1.3.6.1.2.1.1.1.0'));
var_dump($session->getErrno() == SNMP::ERRNO_NOERROR);
var_dump($session->getError());
$session->close();

echo "SNMP::ERRNO_TIMEOUT\n";
$session = new SNMP(SNMP::VERSION_2c, $hostname, 'timeout_community_432', $timeout, $retries);
$session->valueretrieval = SNMP_VALUE_LIBRARY;
var_dump(@$session->get('.1.3.6.1.2.1.1.1.0'));
var_dump($session->getErrno() == SNMP::ERRNO_TIMEOUT);
var_dump($session->getError());
$session->close();

echo "SNMP::ERRNO_ERROR_IN_REPLY\n";
$session = new SNMP(SNMP::VERSION_2c, $hostname, $community, $timeout, $retries);
var_dump(@$session->get('.1.3.6.1.2.1.1.1.110'));
var_dump($session->getErrno() == SNMP::ERRNO_ERROR_IN_REPLY);
var_dump($session->getError());
$session->close();

echo "SNMP::ERRNO_GENERIC\n";
$session = new SNMP(SNMP::VERSION_3, $hostname, 'somebogususer', $timeout, $retries);
$session->setSecurity('authPriv', 'MD5', $auth_pass, 'AES', $priv_pass);
var_dump(@$session->get('.1.3.6.1.2.1.1.1.0'));
var_dump($session->getErrno() == SNMP::ERRNO_GENERIC);
var_dump($session->getError());
var_dump(@$session->get(array('.1.3.6.1.2.1.1.1.0')));
$session->close();

echo "SNMP::ERRNO_OID_PARSING_ERROR\n";
echo "GET: Single wrong OID\n";
$session = new SNMP(SNMP::VERSION_2c, $hostname, $community, $timeout, $retries);
var_dump(@$session->get('.1.3.6.1.2..1.1.1.0'));
var_dump($session->getErrno() == SNMP::ERRNO_OID_PARSING_ERROR);
var_dump($session->getError());
$session->close();
echo "GET: Miltiple OID, one wrong\n";
$session = new SNMP(SNMP::VERSION_2c, $hostname, $community, $timeout, $retries);
var_dump(@$session->get(array('.1.3.6.1.2.1.1.1.0', '.1.3.6.1.2..1.1.1.0')));
var_dump($session->getErrno() == SNMP::ERRNO_OID_PARSING_ERROR);
var_dump($session->getError());
$session->close();
echo "WALK: Single wrong OID\n";
$session = new SNMP(SNMP::VERSION_2c, $hostname, $community, $timeout, $retries);
var_dump(@$session->walk('.1.3.6.1.2..1.1'));
var_dump($session->getErrno() == SNMP::ERRNO_OID_PARSING_ERROR);
var_dump($session->getError());
$session->close();
echo "SET: Wrong type\n";
$session = new SNMP(SNMP::VERSION_3, $hostname, $rwuser, $timeout, $retries);
$session->setSecurity('authPriv', 'MD5', $auth_pass, 'AES', $priv_pass);
$oid1 = 'SNMPv2-MIB::sysContact.0';
var_dump(@$session->set($oid1, 'q', 'blah'));
var_dump($session->getErrno() == SNMP::ERRNO_OID_PARSING_ERROR);
var_dump($session->getError());
?>
--EXPECTF--
SNMP::ERRNO_NOERROR
%string|unicode%(%d) "%s"
bool(true)
%string|unicode%(0) ""
SNMP::ERRNO_TIMEOUT
bool(false)
bool(true)
%string|unicode%(%d) "No response from %s"
SNMP::ERRNO_ERROR_IN_REPLY
bool(false)
bool(true)
%string|unicode%(%d) "Error in packet %s"
SNMP::ERRNO_GENERIC
bool(false)
bool(true)
%string|unicode%(%d) "Fatal error: Unknown user name"
bool(false)
SNMP::ERRNO_OID_PARSING_ERROR
GET: Single wrong OID
bool(false)
bool(true)
string(46) "Invalid object identifier: .1.3.6.1.2..1.1.1.0"
GET: Miltiple OID, one wrong
bool(false)
bool(true)
string(46) "Invalid object identifier: .1.3.6.1.2..1.1.1.0"
WALK: Single wrong OID
bool(false)
bool(true)
string(42) "Invalid object identifier: .1.3.6.1.2..1.1"
SET: Wrong type
bool(false)
bool(true)
string(129) "Could not add variable: OID='.iso.org.dod.internet.mgmt.mib-2.system.sysContact.0' type='q' value='blah': Bad variable type ("q")"