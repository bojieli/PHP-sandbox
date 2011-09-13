--TEST--
timezone configuration [1]
--INI-- 		
date.timezone=GMT
--FILE--
<?php
	putenv('TZ=Europe/Oslo');
	if (defined('PHP_WINDOWS_VERSION_MAJOR')) {
		date_default_timezone_set('Europe/Oslo');
	} else {
		putenv('TZ=Europe/Oslo');
	}
	echo strtotime("2005-06-18 22:15:44"), "\n";

	if (defined('PHP_WINDOWS_VERSION_MAJOR')) {
		date_default_timezone_set('Europe/London');
	} else {
		putenv('TZ=Europe/London');
	}
	echo strtotime("2005-06-18 22:15:44"), "\n";

	date_default_timezone_set('Europe/Oslo');
	echo strtotime("2005-06-18 22:15:44"), "\n";
?>
--EXPECT--
1119125744
1119129344
1119125744
