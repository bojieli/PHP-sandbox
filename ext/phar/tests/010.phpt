--TEST--
Phar::mapPhar buffer overrun
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip");?>
--INI--
phar.require_hash=0
--FILE--
<?php
$file = (binary)"<?php
Phar::mapPhar('hio');
__HALT_COMPILER(); ?>";

// this fails because the manifest length does not include the other 10 byte manifest data

$manifest = pack('V', 1) . (binary)'a' . pack('VVVVVV', 0, time(), 0, crc32((binary)''), 0x00000000, 0);
$file .= pack('VVnVV', strlen($manifest), 1, 0x1000, 0x00000000, 3) . (binary)'hio' . pack('V', 0) . (binary)$manifest;

file_put_contents(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php', $file);
try {
include dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.php';
echo file_get_contents('phar://hio/a');
} catch (Exception $e) {
echo $e->getMessage();
}
?>
--CLEAN--
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.phar.php'); ?>
--EXPECTF--
internal corruption of phar "%s" (too many manifest entries for size of manifest)
