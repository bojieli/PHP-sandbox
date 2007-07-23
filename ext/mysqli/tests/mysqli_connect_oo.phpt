--TEST--
new mysqli()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
<?php require_once('skipifemb.inc'); ?>
--FILE--
<?php
	include "connect.inc";

	$tmp    = NULL;
	$link   = NULL;

	$obj = new stdClass();

	if ($mysqli = new mysqli($host, $user . 'unknown_really', $passwd . 'non_empty', $db, $port, $socket) && !mysqli_connect_errno())
		printf("[003] Can connect to the server using host=%s, user=%s, passwd=***non_empty, dbname=%s, port=%s, socket=%s\n",
			$host, $user . 'unknown_really', $db, $port, $socket);

	if (false !== $mysqli)
		printf("[004] Expecting boolean/false, got %s/%s\n", gettype($mysqli), $mysqli);

	// Run the following tests without an anoynmous MySQL user and use a password for the test user!
	ini_set('mysqli.default_socket', $socket);
	if (!is_object($mysqli = new mysqli($host, $user, $passwd, $db, $port)) || (0 !== mysqli_connect_errno())) {
		printf("[005] Usage of mysqli.default_socket failed\n") ;
	} else {
		$mysqli->close();
	}

	ini_set('mysqli.default_port', $port);
	if (!is_object($mysqli = new mysqli($host, $user, $passwd, $db)) || (0 !== mysqli_connect_errno())) {
		printf("[006] Usage of mysqli.default_port failed\n") ;
	} else {
		$mysqli->close();
	}

	ini_set('mysqli.default_pw', $passwd);
	if (!is_object($mysqli = new mysqli($host, $user)) || (0 !== mysqli_connect_errno())) {
		printf("[007] Usage of mysqli.default_pw failed\n") ;
	} else {
		$mysqli->close();
	}

	ini_set('mysqli.default_user', $user);
	if (!is_object($mysqli = new mysqli($host)) || (0 !== mysqli_connect_errno())) {
		printf("[008] Usage of mysqli.default_user failed\n") ;
	} else {
		$mysqli->close();
	}

	ini_set('mysqli.default_host', $host);
	if (!is_object($mysqli = new mysqli()) || (0 !== mysqli_connect_errno())) {
		printf("[008] Usage of mysqli.default_host failed\n") ;
	} else {
		$mysqli->close();
	}

	if ($IS_MYSQLND) {
		ini_set('mysqli.default_host', 'p:' . $host);
		if (!is_object($mysqli = new mysqli()) || (0 !== mysqli_connect_errno())) {
			printf("[008b] Usage of mysqli.default_host failed\n") ;
		} else {
			$mysqli->close();
		}
	}

	print "... and now Exceptions\n";
	mysqli_report(MYSQLI_REPORT_OFF);
	mysqli_report(MYSQLI_REPORT_STRICT);

	try {
		$mysqli = new mysqli($host, $user . 'unknown_really', $passwd . 'non_empty', $db, $port, $socket);
		printf("[016] Can connect to the server using host=%s, user=%s, passwd=***non_empty, dbname=%s, port=%s, socket=%s\n",
			$host, $user . 'unknown_really', $db, $port, $socket);
		$mysqli->close();
	} catch (mysqli_sql_exception $e) {
		printf("%s\n", $e->getMessage());
	}

	ini_set('mysqli.default_socket', $socket);
	try {
		$mysqli = new mysqli($host, $user, $passwd, $db, $port);
		$mysqli->close();
	} catch (mysqli_sql_exception $e) {
		printf("%s\n", $e->getMessage());
		printf("[017] Usage of mysqli.default_socket failed\n") ;
	}

	ini_set('mysqli.default_port', $port);
	try {
		$mysqli = new mysqli($host, $user, $passwd, $db);
		$mysqli->close();
	} catch (mysqli_sql_exception $e) {
		printf("%s\n", $e->getMessage());
		printf("[018] Usage of mysqli.default_port failed\n") ;
	}

	ini_set('mysqli.default_pw', $passwd);
	try {
		$mysqli = new mysqli($host, $user);
		$mysqli->close();
	} catch (mysqli_sql_exception $e) {
		printf("%s\n", $e->getMessage());
		printf("[019] Usage of mysqli.default_pw failed\n");
	}

	ini_set('mysqli.default_user', $user);
	try {
		$mysqli = new mysqli($host);
		$mysqli->close();
	} catch (mysqli_sql_exception $e) {
		printf("%s\n", $e->getMessage());
		printf("[020] Usage of mysqli.default_user failed\n") ;
	}

	ini_set('mysqli.default_host', $host);
	try {
		/* NOTE that at this point one must use a different syntax! */
		$mysqli = mysqli_init();
		$mysqli->real_connect();
		assert(0 === mysqli_connect_errno());
		$mysqli->close();
		assert(0 === mysqli_connect_errno());
	} catch (mysqli_sql_exception $e) {
		printf("%s\n", $e->getMessage());
		printf("[021] Usage of mysqli.default_host failed\n");
	}

	print "done!";
?>
--EXPECTF--
Warning: mysqli::mysqli(): (%d/%d): Access denied for user '%sunknown_real'@'%s' (using password: %s) in %s on line %d

Warning: mysqli::close(): Couldn't fetch mysqli in %s on line %d
... and now Exceptions
Access denied for user '%s'@'%s' (using password: %s)
done!