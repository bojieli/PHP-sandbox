--TEST--
Bug #62112: number_format() is not binary safe
--FILE--
<?php
var_dump(number_format(2.5, 2, "\0", "'"));
--EXPECT--
string(4) "2 50"
