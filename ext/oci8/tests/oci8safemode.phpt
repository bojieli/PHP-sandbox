--TEST--
Test functionality disabled in safe mode
--SKIPIF--
<?php if (!extension_loaded('oci8')){ die("skip no oci8 extension"); } if (PHP_VERSION_ID < 503099){ die("skip: safe_mode no longer available"); }  ?>
--INI--
safe_mode=On
oci8.privileged_connect=On
--FILE--
<?php

$c = oci_connect("hr", "hrpwd", "//localhost/XE", null, OCI_SYSDBA);

$r = oci_password_change($c, "hr", "hrpwd", "hrpwd");

echo "Done\n";
?>
--EXPECTF--
%sarning:%sDirective 'safe_mode' is deprecated in PHP 5.3 and greater in Unknown on line 0

Warning: oci_connect(): Privileged connect is disabled in Safe Mode in %s on line %d

Warning: oci_password_change(): is disabled in Safe Mode in %s on line %d
Done
