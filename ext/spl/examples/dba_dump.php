<?php

/** dba dump utility
 *
 * Usage: php dba_dump.php <file> <handler> [<regex>]
 *
 * Show all groups in the ini file specified by <file>.
 * The regular expression <regex> is used to filter the by setting name.
 *
 * Note: configure with --enable-dba 
 *
 * (c) Marcus Boerger, 2003 - 2004
 */

if ($argc < 3) {
	echo <<<EOF
Usage: php ${_SERVER['PHP_SELF']} <file> <handler> [<regex>]

Show all groups in the ini file specified by <file>.
The regular expression <regex> is used to filter the by setting name.


EOF;
	exit(1);
}

if (!class_exists("DbaReader")) require_once("dbareader.inc");
if (!class_exists("KeyFilter")) require_once("keyfilter.inc");

$db = new DbaReader($argv[1], $argv[2]);

if ($argc>3) {
	$db = new KeyFilter($db, $argv[3]);
}

foreach($db as $key => $val) {
	echo "'$key' => '$val'\n";
}

?>