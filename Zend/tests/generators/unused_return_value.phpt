--TEST--
There shouldn't be any leaks when the genertor's return value isn't used
--FILE--
<?php

function *gen($foo) {}

gen('foo'); // return value not used

?>
===DONE===
--EXPECT--
===DONE===
