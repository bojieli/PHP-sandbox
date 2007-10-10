--TEST--
Bug #35517 (mysqli_stmt_fetch returns NULL)
--SKIPIF--
<?php 
require_once('skipif.inc'); 
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	$mysql = new mysqli($host, $user, $passwd, $db, $port, $socket);

	$mysql->query("CREATE TABLE temp (id INT UNSIGNED NOT NULL)");
	$mysql->query("INSERT INTO temp (id) VALUES (3000000897),(3800001532),(3900002281),(3100059612)");

	$stmt = $mysql->prepare("SELECT id FROM temp");
	$stmt->execute();
	$stmt->bind_result($id);
	while ($stmt->fetch()) {
		var_dump($id);
	}
	$stmt->close();

	$mysql->query("DROP TABLE temp");
	$mysql->close();
?>
--EXPECTF--
%s(10) "3000000897"
%s(10) "3800001532"
%s(10) "3900002281"
%s(10) "3100059612"
