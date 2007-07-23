--TEST--
mysqli_fetch_field_direct()
--SKIPIF--
<?php require_once('skipif.inc'); ?>
<?php require_once('skipifemb.inc'); ?>
--FILE--
<?php
	include "connect.inc";

	$tmp    = NULL;
	$link   = NULL;

	if (!is_null($tmp = @mysqli_fetch_field_direct()))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_fetch_field_direct($link)))
		printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_fetch_field_direct($link, $link)))
		printf("[003] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	require('table.inc');

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test AS TEST ORDER BY id LIMIT 1")) {
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	var_dump(mysqli_fetch_field_direct($res, -1));
	var_dump(mysqli_fetch_field_direct($res, 0));
	var_dump(mysqli_fetch_field_direct($res, 2));

	mysqli_free_result($res);

	if (NULL !== ($tmp = mysqli_fetch_field_direct($res, 0)))
		printf("Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);
	print "done!";
?>
--EXPECTF--
Warning: mysqli_fetch_field_direct(): Field offset is invalid for resultset in %s on line %d
bool(false)
object(stdClass)#%d (11) {
  ["name"]=>
  string(2) "ID"
  ["orgname"]=>
  string(2) "id"
  ["table"]=>
  string(4) "TEST"
  ["orgtable"]=>
  string(4) "test"
  ["def"]=>
  string(0) ""
  ["max_length"]=>
  int(%d)
  ["length"]=>
  int(%d)
  ["charsetnr"]=>
  int(%d)
  ["flags"]=>
  int(%d)
  ["type"]=>
  int(%d)
  ["decimals"]=>
  int(%d)
}

Warning: mysqli_fetch_field_direct(): Field offset is invalid for resultset in %s on line %d
bool(false)

Warning: mysqli_fetch_field_direct(): Couldn't fetch mysqli_result in %s on line %d
done!
--UEXPECTF--
Warning: mysqli_fetch_field_direct(): Field offset is invalid for resultset in %s on line %d
bool(false)
object(stdClass)#%d (11) {
  [u"name"]=>
  unicode(2) "ID"
  [u"orgname"]=>
  unicode(2) "id"
  [u"table"]=>
  unicode(4) "TEST"
  [u"orgtable"]=>
  unicode(4) "test"
  [u"def"]=>
  unicode(0) ""
  [u"max_length"]=>
  int(%d)
  [u"length"]=>
  int(%d)
  [u"charsetnr"]=>
  int(%d)
  [u"flags"]=>
  int(%d)
  [u"type"]=>
  int(%d)
  [u"decimals"]=>
  int(%d)
}

Warning: mysqli_fetch_field_direct(): Field offset is invalid for resultset in %s on line %d
bool(false)

Warning: mysqli_fetch_field_direct(): Couldn't fetch mysqli_result in %s on line %d
done!