--TEST--
time_sleep_until() function - error test for time_sleep_until()
--CREDITS--
Francesco Fullone ff@ideato.it
#PHPTestFest Cesena Italia on 2009-06-20
--FILE--
<?php
  var_dump(time_sleep_until());
?>
--EXPECTF--
Warning: time_sleep_until() expects exactly 1 parameter, 0 given in %s on line 2
NULL
