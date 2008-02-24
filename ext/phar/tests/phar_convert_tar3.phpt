--TEST--
Phar::convertToTar() gzipped
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
<?php if (!extension_loaded("zlib")) die("skip"); ?>
--INI--
phar.require_hash=0
phar.readonly=0
--FILE--
<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
$pname = 'phar://' . $fname;
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.tar';
$stub = '<?php echo "first stub\n"; __HALT_COMPILER(); ?>';
$file = $stub;

$files = array();
$files['a'] = 'a';
$files['b'] = 'b';
$files['c'] = 'c';

include 'files/phar_test.inc';

$phar = new Phar($fname);
var_dump($phar->isTar());
var_dump($phar->isCompressed());
var_dump($phar->getStub());

$phar->convertToTar(Phar::BZ2);
var_dump($phar->isTar());
var_dump($phar->isCompressed());
var_dump($phar->getStub());

copy($fname2 . '.bz2', $fname3);

$phar = new Phar($fname3);
var_dump($phar->isTar());
var_dump($phar->isCompressed() == Phar::BZ2);
var_dump($phar->getStub());

?>
===DONE===
--CLEAN--
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.2.phar.tar');
__HALT_COMPILER();
?>
--EXPECT--
bool(false)
bool(false)
string(48) "<?php echo "first stub\n"; __HALT_COMPILER(); ?>"
bool(true)
int(8192)
string(60) "<?php // tar-based phar archive stub file
__HALT_COMPILER();"
bool(true)
bool(true)
string(60) "<?php // tar-based phar archive stub file
__HALT_COMPILER();"
===DONE===
