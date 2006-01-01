--TEST--
PHP_Archive::mapPhar invalid file (gzipped file length is too short)
--SKIPIF--
<?php if (!extension_loaded("phar")) print "skip";
if (!PHP_Archive::canCompress()) print "skip"; ?>
--FILE--
<?php
function cleanup() { unlink(dirname(__FILE__) . '/008_phar.php'); }
register_shutdown_function('cleanup');
$file = "<?php
PHP_Archive::mapPhar('hio', true);
__HALT_COMPILER(); ?>";
// file length is too short
$manifest = pack('V', 1) . 'a' . pack('VVVV', 1, time(), 0, 9);
$file .= pack('VV', strlen($manifest) + 4, 1) . $manifest . pack('VV', crc32('a'), 1) . chr(0) . chr(4) . chr(0); // 'a' gzdeflated
file_put_contents(dirname(__FILE__) . '/008_phar.php', $file);
include dirname(__FILE__) . '/008_phar.php';
echo file_get_contents('phar://hio/a');
?>
--EXPECTF--
Warning: file_get_contents(phar://hio/a): failed to open stream: phar error: decompression failed in %s on line %d