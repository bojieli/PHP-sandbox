--TEST--
time_sleep_until() function - error test for time_sleep_until()
--SKIPIF--
<?php
	function_exists('time_sleep_until') or die('skip time_sleep_until() is not supported in this build.');
?>
--CREDITS--
Filippo De Santis fd@ideato.it
#PHPTestFest Cesena Italia on 2009-06-20
--FILE--
<?php
  var_dump(time_sleep_until('goofy'));
?>
--EXPECTF--
Warning: time_sleep_until() expects parameter 1 to be double, Unicode string given in %s on line 2
NULL
