--TEST--
mysqli bind_param/bind_result short values
--FILE--
<?php
	include "connect.inc";
	
	/*** test mysqli_connect 127.0.0.1 ***/
	$link = mysqli_connect("localhost", $user, $passwd);

	mysqli_select_db($link, "test");

  	mysqli_query($link,"DROP TABLE IF EXISTS test_bind_fetch");
  	mysqli_query($link,"CREATE TABLE test_bind_fetch(c1 smallint unsigned,
                                                     c2 smallint unsigned,
                                                     c3 smallint,
                                                     c4 smallint,
                                                     c5 smallint,
                                                     c6 smallint unsigned,
                                                     c7 smallint)");

	$stmt = mysqli_prepare($link, "INSERT INTO test_bind_fetch VALUES (?,?,?,?,?,?,?)");
	mysqli_bind_param($stmt, array(MYSQLI_BIND_INT,MYSQLI_BIND_INT,MYSQLI_BIND_INT,
				       MYSQLI_BIND_INT,MYSQLI_BIND_INT,MYSQLI_BIND_INT,
 				       MYSQLI_BIND_INT),
				$c1,$c2,$c3,$c4,$c5,$c6,$c7);

	$c1 = -23;
	$c2 = 35999;
	$c3 = NULL;
	$c4 = -500;
	$c5 = -9999999;
	$c6 = -0;
	$c7 = 0;

	mysqli_execute($stmt);
	mysqli_stmt_close($stmt);

	$stmt = mysqli_prepare($link, "SELECT * FROM test_bind_fetch");
	mysqli_bind_result($stmt, $c1, $c2, $c3, $c4, $c5, $c6, $c7);
	mysqli_execute($stmt);
	mysqli_fetch($stmt);

	$test = array($c1,$c2,$c3,$c4,$c5,$c6,$c7);

	var_dump($test);

	mysqli_stmt_close($stmt);
	mysqli_close($link);
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
  int(0)
  [6]=>
  int(0)
}
