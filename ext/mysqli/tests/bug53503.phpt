--TEST--
Bug #53503 (mysqli::query returns false after successful LOAD DATA query)
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	require_once("connect.inc");

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	if (!$link->query("DROP TABLE IF EXISTS tlocaldata")) {
		printf("[002] [%d] %s\n", $link->errno, $link->error);
	}

	if (!$link->query("CREATE TABLE tlocaldata (dump1 INT UNSIGNED NOT NULL PRIMARY KEY) ENGINE=" . $engine)) {
		printf("[003] [%d] %s\n", $link->errno, $link->error);
	}

	file_put_contents('bug53503.data', 'jokijoki');

	if (!$link->query("LOAD DATA LOCAL INFILE 'bug53503.data' REPLACE INTO TABLE tlocaldata (dump1)")) {
		echo "bug";
	} else {
		echo "done";
	}
?>
--CLEAN--
<?php
require_once('connect.inc');

if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
	printf("[clean] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
		$host, $user, $db, $port, $socket);
}

if (!mysqli_query($link, 'DROP TABLE IF EXISTS tlocaldata')) {
	printf("[clean] Failed to drop old test table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
}

mysqli_close($link);

unlink('bug53503.data');
?>
--EXPECT--
done