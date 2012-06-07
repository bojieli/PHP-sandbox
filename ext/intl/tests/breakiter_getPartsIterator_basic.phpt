--TEST--
BreakIterator::getPartsIterator(): basic test
--FILE--
<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$bi = BreakIterator::createWordInstance('pt');
$pi = $bi->getPartsIterator();
var_dump(get_class($pi));
print_r(iterator_to_array($pi));

$bi->setText("foo bar");
$pi = $bi->getPartsIterator();
var_dump(get_class($pi->getBreakIterator()));
print_r(iterator_to_array($pi));
var_dump($pi->getRuleStatus());
?>
==DONE==
--EXPECT--
string(17) "IntlPartsIterator"
Array
(
)
string(22) "RuleBasedBreakIterator"
Array
(
    [0] => foo
    [1] =>  
    [2] => bar
)
int(0)
==DONE==