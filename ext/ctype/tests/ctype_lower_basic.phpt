--TEST--
Test ctype_lower() function : basic functionality 
--SKIPIF--
<?php
if( ini_get("unicode.semantics") == "1") {
    die('skip do not run when unicode on');
}
?>
--FILE--
<?php
/* Prototype  : bool ctype_lower(mixed $c)
 * Description: Checks for lowercase character(s)  
 * Source code: ext/ctype/ctype.c
 */

echo "*** Testing ctype_lower() : basic functionality ***\n";

$orig = setlocale(LC_CTYPE, "C");

$c1 = 'helloworld';
$c2 = "Hello, world!\n";

var_dump(ctype_lower($c1));
var_dump(ctype_lower($c2));

setlocale(LC_CTYPE, $orig);
?>
===DONE===
--EXPECTF--
*** Testing ctype_lower() : basic functionality ***

Deprecated: setlocale(): deprecated in Unicode mode, please use ICU locale functions in %sctype_lower_basic.php on line 9
bool(true)
bool(false)

Deprecated: setlocale(): deprecated in Unicode mode, please use ICU locale functions in %sctype_lower_basic.php on line 17
===DONE===
