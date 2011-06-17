--TEST--
Test for date_diff with timezone abbreviations.
--INI--
date.timezone=Europe/London
--FILE--
<?php
$start = new DateTime('2010-10-04 02:18:48 EDT');
$end   = new DateTime('2010-11-06 18:38:28 EDT');
$int = $start->diff($end);
var_dump($start);
var_dump($end);
var_dump($int);
?>
--EXPECT--
object(DateTime)#1 (3) {
  ["date"]=>
  string(19) "2010-10-04 02:18:48"
  ["timezone_type"]=>
  int(2)
  ["timezone"]=>
  string(3) "EDT"
}
object(DateTime)#2 (3) {
  ["date"]=>
  string(19) "2010-11-06 18:38:28"
  ["timezone_type"]=>
  int(2)
  ["timezone"]=>
  string(3) "EDT"
}
object(DateInterval)#3 (8) {
  ["y"]=>
  int(0)
  ["m"]=>
  int(1)
  ["d"]=>
  int(2)
  ["h"]=>
  int(16)
  ["i"]=>
  int(19)
  ["s"]=>
  int(40)
  ["invert"]=>
  int(0)
  ["days"]=>
  int(33)
}
