--TEST--
IntlCalendar::isEquivalentTo(): bad arguments
--SKIPIF--
<?php
if (!extension_loaded('intl'))
	die('skip intl extension not enabled');
--FILE--
<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

function eh($errno, $errstr) {
echo "error: $errno, $errstr\n";
}
set_error_handler('eh');

var_dump($c->isEquivalentTo(0));
var_dump($c->isEquivalentTo($c, 1));
var_dump($c->isEquivalentTo(1));

var_dump(intlcal_is_equivalent_to($c));
var_dump(intlcal_is_equivalent_to($c, 1));
var_dump(intlcal_is_equivalent_to(1, $c));

--EXPECT--
error: 4096, Argument 1 passed to IntlCalendar::isEquivalentTo() must be an instance of IntlCalendar, integer given
error: 2, IntlCalendar::isEquivalentTo() expects parameter 1 to be IntlCalendar, integer given
error: 2, IntlCalendar::isEquivalentTo(): intlcal_is_equivalent_to: bad arguments
bool(false)
error: 2, IntlCalendar::isEquivalentTo() expects exactly 1 parameter, 2 given
error: 2, IntlCalendar::isEquivalentTo(): intlcal_is_equivalent_to: bad arguments
bool(false)
error: 4096, Argument 1 passed to IntlCalendar::isEquivalentTo() must be an instance of IntlCalendar, integer given
error: 2, IntlCalendar::isEquivalentTo() expects parameter 1 to be IntlCalendar, integer given
error: 2, IntlCalendar::isEquivalentTo(): intlcal_is_equivalent_to: bad arguments
bool(false)
error: 2, intlcal_is_equivalent_to() expects exactly 2 parameters, 1 given
error: 2, intlcal_is_equivalent_to(): intlcal_is_equivalent_to: bad arguments
bool(false)
error: 4096, Argument 2 passed to intlcal_is_equivalent_to() must be an instance of IntlCalendar, integer given
error: 2, intlcal_is_equivalent_to() expects parameter 2 to be IntlCalendar, integer given
error: 2, intlcal_is_equivalent_to(): intlcal_is_equivalent_to: bad arguments
bool(false)
error: 4096, Argument 1 passed to intlcal_is_equivalent_to() must be an instance of IntlCalendar, integer given
error: 2, intlcal_is_equivalent_to() expects parameter 1 to be IntlCalendar, integer given
error: 2, intlcal_is_equivalent_to(): intlcal_is_equivalent_to: bad arguments
bool(false)
