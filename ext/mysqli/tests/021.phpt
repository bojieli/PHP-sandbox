--TEST--
mysqli bind_param+bind_result char/text
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_select_db($link, $db);

	mysqli_query($link,"DROP TABLE IF EXISTS test_bind_fetch");
	mysqli_query($link,"CREATE TABLE test_bind_fetch(c1 char(10), c2 text)");

	$stmt = mysqli_prepare($link, "INSERT INTO test_bind_fetch VALUES (?,?)");
	mysqli_bind_param($stmt, "ss", $q1, $q2);
	$q1 = "1234567890";
	$q2 = "this is a test";
	mysqli_execute($stmt);
	mysqli_stmt_close($stmt);

	$stmt = mysqli_prepare($link, "SELECT * FROM test_bind_fetch");
	mysqli_bind_result($stmt, $c1, $c2);
	mysqli_execute($stmt);
	mysqli_fetch($stmt);

	$test = array($c1,$c2);

	var_dump($test);

	mysqli_stmt_close($stmt);
	mysqli_query($link, "DROP TABLE IF EXISTS test_bind_fetch");
	mysqli_close($link);
	print "done!";
?>
--CLEAN--
<?php
include "connect.inc";
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_bind_fetch"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>
--EXPECTF--
array(2) {
  [0]=>
  %unicode|string%(10) "1234567890"
  [1]=>
  %unicode|string%(14) "this is a test"
}
done!