--TEST--
Bug #12556: fgetcvs ignores lengths when quotes not closed
--POST--
--GET--
--FILE--
<?php
$fp = fopen(dirname(__FILE__)."/test.csv", "r");
while($line = fgetcsv($fp, 24)) {
	var_dump($line);
}
fclose($fp);
?>
--EXPECT--
array(4) {
  [0]=>
  string(1) "6"
  [1]=>
  string(1) "7"
  [2]=>
  string(1) "8"
  [3]=>
  string(5) "line1"
}
array(4) {
  [0]=>
  string(1) "1"
  [1]=>
  string(1) "2"
  [2]=>
  string(1) "3"
  [3]=>
  string(186) "line2
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
2,4,5,line3
"
}
