--TEST--
gethostbyname() function - basic invalid parameter test
--CREDITS--
"Sylvain R." <sracine@phpquebec.org>
--FILE--
<?php
	$ip = gethostbyname("www.php.net");
	var_dump((bool) ip2long($ip));
?>
--EXPECT--
bool(true)
