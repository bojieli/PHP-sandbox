--TEST--
Gettext basic test
--SKIPIF--
<?php
	error_reporting(0);
	if (!extension_loaded("gettext")) {
		die("skip\n");
	}
	if (!setlocale(LC_ALL, 'fi_FI')) {
		die("skip fi_FI locale not supported.");
	}
?>
--FILE--
<?php // $Id$

chdir(dirname(__FILE__));
putenv("LANGUAGE=fi");
setlocale(LC_ALL, 'fi_FI');
bindtextdomain ("messages", "./locale");
textdomain ("messages");
echo gettext("Basic test"), "\n";
echo _("Basic test"), "\n";

?>
--EXPECT--
Perustesti
Perustesti
