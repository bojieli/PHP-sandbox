--TEST--
Test if dngettext() returns the correct translations (optionally plural).
--SKIPIF--
<?php
if (!extension_loaded("gettext")) {
    die("skip gettext extension is not loaded.\n");
}
if (!setlocale(LC_ALL, 'en_US.UTF-8')) {
    die("skip en_US.UTF-8 locale not supported.");
}
--FILE--
<?php

// Using deprectated setlocale() in PHP6. The test needs to be changed
// when there is an alternative available.

chdir(dirname(__FILE__));
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain('dngettextTest', './locale');

var_dump(dngettext('dngettextTest', 'item', 'items', 1));
var_dump(dngettext('dngettextTest', 'item', 'items', 2));
--EXPECTF--
Deprecated: setlocale(): deprecated in Unicode mode, please use ICU locale functions in %s.php on line %d
string(7) "Produkt"
string(8) "Produkte"
--CREDITS--
Till Klampaeckel, till@php.net
PHP Testfest Berlin 2009-05-09
