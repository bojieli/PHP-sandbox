--TEST--
IntlCalendar::getTimeZone(): bad arguments
--SKIPIF--
<?php
if (!extension_loaded('intl'))
	die('skip intl extension not enabled');
--FILE--
<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getTimeZone(1));

var_dump(intlcal_get_time_zone($c, 1));
var_dump(intlcal_get_time_zone(1));

--EXPECTF--

Warning: IntlCalendar::getTimeZone() expects exactly 0 parameters, 1 given in %s on line %d

Warning: IntlCalendar::getTimeZone(): intlcal_get_time_zone: bad arguments in %s on line %d
bool(false)

Warning: intlcal_get_time_zone() expects exactly 1 parameter, 2 given in %s on line %d

Warning: intlcal_get_time_zone(): intlcal_get_time_zone: bad arguments in %s on line %d
bool(false)

Catchable fatal error: Argument 1 passed to intlcal_get_time_zone() must be an instance of IntlCalendar, integer given in %s on line %d
