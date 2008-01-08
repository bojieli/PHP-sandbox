--TEST--
mysqli fetch short values
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	if (!mysqli_query($link, "SET sql_mode=''"))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "DROP TABLE IF EXISTS test_bind_fetch"))
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$rc = mysqli_query($link, "CREATE TABLE test_bind_fetch(c1 smallint unsigned,
													 c2 smallint unsigned,
													 c3 smallint,
													 c4 smallint,
													 c5 smallint,
													 c6 smallint unsigned,
													 c7 smallint) ENGINE=" . $engine);
	if (!$rc)
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "INSERT INTO test_bind_fetch VALUES (-23,35999,NULL,-500,-9999999,+30,0)"))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$stmt = mysqli_prepare($link, "SELECT * FROM test_bind_fetch");
	mysqli_bind_result($stmt, $c1, $c2, $c3, $c4, $c5, $c6, $c7);
	mysqli_execute($stmt);
	mysqli_fetch($stmt);

	$test = array($c1,$c2,$c3,$c4,$c5,$c6,$c7);

	var_dump($test);

	mysqli_stmt_close($stmt);
	mysqli_query($link, "DROP TABLE IF EXISTS test_bind_fetch");
	mysqli_close($link);
	print "done!";
?>
--EXPECT--
array(7) {
  [0]=>
  int(0)
  [1]=>
  int(35999)
  [2]=>
  NULL
  [3]=>
  int(-500)
  [4]=>
  int(-32768)
  [5]=>
  int(30)
  [6]=>
  int(0)
}
done!