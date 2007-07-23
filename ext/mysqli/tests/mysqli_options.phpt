--TEST--
mysqli_options()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
<?php require_once('skipifemb.inc'); ?>
--FILE--
<?php
	include "connect.inc";
	$valid_options = array(  MYSQLI_READ_DEFAULT_GROUP, MYSQLI_READ_DEFAULT_FILE,
		MYSQLI_OPT_CONNECT_TIMEOUT, MYSQLI_OPT_LOCAL_INFILE,
		MYSQLI_INIT_COMMAND, MYSQLI_READ_DEFAULT_GROUP,
		MYSQLI_READ_DEFAULT_FILE, MYSQLI_OPT_CONNECT_TIMEOUT,
		MYSQLI_OPT_LOCAL_INFILE, MYSQLI_INIT_COMMAND,
		MYSQLI_SET_CHARSET_NAME);

	if ($IS_MYSQLND && defined('MYSQLI_OPT_INT_AND_YEARS_AS_INT'))
		$valid_options[] = constant('MYSQLI_OPT_INT_AND_YEARS_AS_INT');
	if (defined('MYSQLI_OPT_NUMERIC_AND_DATETIME_AS_UNICODE'))
		$valid_options[] = constant('MYSQLI_OPT_NUMERIC_AND_DATETIME_AS_UNICODE');

	$tmp    = NULL;
	$link   = NULL;

	if (!is_null($tmp = @mysqli_options()))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_options($link)))
		printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	$link = mysqli_init();

	if (!is_null($tmp = @mysqli_options($link, MYSQLI_OPT_CONNECT_TIMEOUT)))
		printf("[003] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_options($link, "s", 'extra_my.cnf')))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_options($link, MYSQLI_INIT_COMMAND, 'SET AUTOCOMMIT=0', 'foo')))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	// print "run_tests.php don't fool me with your 'ungreedy' expression '.+?'!\n";
	var_dump("MYSQLI_READ_DEFAULT_GROUP",	mysqli_options($link, MYSQLI_READ_DEFAULT_GROUP, 'extra_my.cnf'));
	var_dump("MYSQLI_READ_DEFAULT_FILE",	mysqli_options($link, MYSQLI_READ_DEFAULT_FILE, 'extra_my.cnf'));
	var_dump("MYSQLI_OPT_CONNECT_TIMEOUT",	mysqli_options($link, MYSQLI_OPT_CONNECT_TIMEOUT, 10));
	var_dump("MYSQLI_OPT_LOCAL_INFILE",		mysqli_options($link, MYSQLI_OPT_LOCAL_INFILE, 1));
	var_dump("MYSQLI_INIT_COMMAND",			mysqli_options($link, MYSQLI_INIT_COMMAND, array('SET AUTOCOMMIT=0', 'SET AUTOCOMMIT=1')));
	var_dump("MYSQLI_READ_DEFAULT_GROUP",	mysqli_options($link, MYSQLI_READ_DEFAULT_GROUP, 'extra_my.cnf'));
	var_dump("MYSQLI_READ_DEFAULT_FILE",	mysqli_options($link, MYSQLI_READ_DEFAULT_FILE, 'extra_my.cnf'));
	var_dump("MYSQLI_OPT_CONNECT_TIMEOUT",	mysqli_options($link, MYSQLI_OPT_CONNECT_TIMEOUT, 10));
	var_dump("MYSQLI_OPT_LOCAL_INFILE",		mysqli_options($link, MYSQLI_OPT_LOCAL_INFILE, 1));
	var_dump("MYSQLI_INIT_COMMAND",			mysqli_options($link, MYSQLI_INIT_COMMAND, 'SET AUTOCOMMIT=0'));
	var_dump("MYSQLI_CLIENT_SSL",			mysqli_options($link, MYSQLI_CLIENT_SSL, 'not an mysqli_option'));

	if ($IS_MYSQLND && defined('MYSQLI_OPT_INT_AND_YEARS_AS_INT') &&
		!($tmp = mysqli_options($link, constant('MYSQLI_OPT_INT_AND_YEARS_AS_INT'), true)))
		printf("[006] Expecting boolean/true got %s/%s\n", gettype($tmp), $tmp);

	if (defined('MYSQLI_OPT_NUMERIC_AND_DATETIME_AS_UNICODE') &&
		!($tmp = mysqli_options($link, constant('MYSQLI_OPT_NUMERIC_AND_DATETIME_AS_UNICODE'), true)))
		printf("[006] Expecting boolean/true got %s/%s\n", gettype($tmp), $tmp);

	for ($flag = -10000; $flag < 10000; $flag++) {
		if (in_array($flag, $valid_options))
			continue;
		if (FALSE !== ($tmp = mysqli_options($link, $flag, 'definetely not an mysqli_option'))) {
			var_dump("SOME_FLAG", $flag, $tmp);
		}
	}

	mysqli_close($link);

	echo "Link closed";
	var_dump("MYSQLI_INIT_COMMAND", mysqli_options($link, MYSQLI_INIT_COMMAND, 'SET AUTOCOMMIT=1'));
	var_dump("SOME_RANDOM_FLAG", mysqli_options($link, $flag, 'definetly not an mysqli_option'));
	print "done!";
?>
--EXPECTF--
%s(25) "MYSQLI_READ_DEFAULT_GROUP"
bool(true)
%s(24) "MYSQLI_READ_DEFAULT_FILE"
bool(true)
%s(26) "MYSQLI_OPT_CONNECT_TIMEOUT"
bool(true)
%s(23) "MYSQLI_OPT_LOCAL_INFILE"
bool(true)
%s(19) "MYSQLI_INIT_COMMAND"
bool(true)
%s(25) "MYSQLI_READ_DEFAULT_GROUP"
bool(true)
%s(24) "MYSQLI_READ_DEFAULT_FILE"
bool(true)
%s(26) "MYSQLI_OPT_CONNECT_TIMEOUT"
bool(true)
%s(23) "MYSQLI_OPT_LOCAL_INFILE"
bool(true)
%s(19) "MYSQLI_INIT_COMMAND"
bool(true)
%s(17) "MYSQLI_CLIENT_SSL"
bool(false)
Link closed
Warning: mysqli_options(): Couldn't fetch mysqli in %s line %d
%s(19) "MYSQLI_INIT_COMMAND"
NULL

Warning: mysqli_options(): Couldn't fetch mysqli in %s line %d
%s(16) "SOME_RANDOM_FLAG"
NULL
done!