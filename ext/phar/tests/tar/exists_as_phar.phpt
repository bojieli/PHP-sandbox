--TEST--
Phar: phar-based phar named with ".tar" fails
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--INI--
phar.readonly=0
phar.require_hash=0
--FILE--
<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$tname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.tar';
$alias = 'phar://hio';

$phar = new Phar($fname);
$phar['a.php'] = '<?php echo "This is a\n"; include "'.$alias.'/b.php"; ?>';
$phar->setAlias('hio');
$phar->addEmptyDir('test');
$phar->stopBuffering();
copy($fname, $tname);

try {
	$p = new Phar($tname);
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}

?>
===DONE===
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.phar.tar');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.phar');
?>
--EXPECTF--
phar tar error: "%sexists_as_phar.phar.tar" already exists as a regular phar and must be deleted from disk prior to creating as a tar-based phar
===DONE===
