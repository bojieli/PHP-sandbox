--TEST--
Cookies test#1
--COOKIE--
cookie1=val1  ; cookie2=val2%20; cookie3=val 3.; cookie 4= value 4 %3B; cookie1=bogus; %20cookie1=ignore;+cookie1=ignore;cookie1;cookie  5=%20 value; cookie%206=���;cookie+7=;$cookie.8;cookie-9=1;;;- & % $cookie 10=10
--FILE--
<?php
var_dump($_COOKIE);
?>
--EXPECT--
array(10) {
  ["cookie1"]=>
  string(6) "val1  "
  ["cookie2"]=>
  string(5) "val2 "
  ["cookie3"]=>
  string(6) "val 3."
  ["cookie_4"]=>
  string(10) " value 4 ;"
  ["cookie__5"]=>
  string(7) "  value"
  ["cookie_6"]=>
  string(3) "���"
  ["cookie_7"]=>
  string(0) ""
  ["$cookie_8"]=>
  string(0) ""
  ["cookie-9"]=>
  string(1) "1"
  ["-_&_%_$cookie_10"]=>
  string(2) "10"
}
--UEXPECTF--
array(10) {
  [u"cookie1"]=>
  unicode(0) ""
  [u"cookie2"]=>
  unicode(5) "val2 "
  [u"cookie3"]=>
  unicode(6) "val 3."
  [u"cookie_4"]=>
  unicode(10) " value 4 ;"
  [u"cookie__5"]=>
  unicode(7) "  value"
  [u"cookie_6"]=>
  unicode(3) "%s"
  [u"cookie_7"]=>
  unicode(0) ""
  [u"$cookie_8"]=>
  unicode(0) ""
  [u"cookie-9"]=>
  unicode(1) "1"
  [u"-_&_%_$cookie_10"]=>
  unicode(2) "10"
}
