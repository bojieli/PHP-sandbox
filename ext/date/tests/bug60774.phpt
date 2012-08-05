--TEST--
Bug #60774 (DateInterval::format("%a") is always zero when an interval is created using the createFromDateString method)
--FILE--
<?php
$i= DateInterval::createFromDateString('2 days');
var_dump($i);
echo $i->format("%d"), "\n";
echo $i->format("%a"), "\n";
?>
--EXPECT--
object(DateInterval)#1 (8) {
  ["y"]=>
  int(0)
  ["m"]=>
  int(0)
  ["d"]=>
  int(2)
  ["h"]=>
  int(0)
  ["i"]=>
  int(0)
  ["s"]=>
  int(0)
  ["invert"]=>
  int(0)
  ["days"]=>
  bool(false)
}
2
(unknown)
