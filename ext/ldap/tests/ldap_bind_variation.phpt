--TEST--
ldap_bind() - Advanced binding
--CREDITS--
Patrick Allaert <patrickallaert@php.net>
# Belgian PHP Testfest 2009
--SKIPIF--
<?php require_once dirname(__FILE__) .'/skipif.inc'; ?>
<?php require_once dirname(__FILE__) .'/skipifbindfailure.inc'; ?>
--FILE--
<?php
require "connect.inc";

$link = ldap_connect($host, $port);
ldap_set_option($link, LDAP_OPT_PROTOCOL_VERSION, $protocol_version);
var_dump(ldap_bind($link, $user, $passwd));
?>
===DONE===
--EXPECT--
bool(true)
===DONE===
