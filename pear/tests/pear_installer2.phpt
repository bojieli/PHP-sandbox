--TEST--
PEAR_Installer test #2
--SKIPIF--
skip
--FILE--
<?php
$temp_path = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'testinstallertemp';
if (!is_dir($temp_path)) {
    mkdir($temp_path);
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'php')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'php');
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'data')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'data');
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'doc')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'doc');
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'test')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'test');
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'ext')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'ext');
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'script')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'script');
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'tmp')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'tmp');
}
if (!is_dir($temp_path . DIRECTORY_SEPARATOR . 'bin')) {
    mkdir($temp_path . DIRECTORY_SEPARATOR . 'bin');
}
// make the fake configuration - we'll use one of these and it should work
$config = serialize(array('master_server' => 'pear.php.net',
    'php_dir' => $temp_path . DIRECTORY_SEPARATOR . 'php',
    'ext_dir' => $temp_path . DIRECTORY_SEPARATOR . 'ext',
    'data_dir' => $temp_path . DIRECTORY_SEPARATOR . 'data',
    'doc_dir' => $temp_path . DIRECTORY_SEPARATOR . 'doc',
    'test_dir' => $temp_path . DIRECTORY_SEPARATOR . 'test',
    'bin_dir' => $temp_path . DIRECTORY_SEPARATOR . 'bin',));
touch($temp_path . DIRECTORY_SEPARATOR . 'pear.conf');
$fp = fopen($temp_path . DIRECTORY_SEPARATOR . 'pear.conf', 'w');
fwrite($fp, $config);
fclose($fp);
touch($temp_path . DIRECTORY_SEPARATOR . 'pear.ini');
$fp = fopen($temp_path . DIRECTORY_SEPARATOR . 'pear.ini', 'w');
fwrite($fp, $config);
fclose($fp);

putenv('PHP_PEAR_SYSCONF_DIR='.$temp_path);
$home = getenv('HOME');
if (!empty($home)) {
    // for PEAR_Config initialization
    putenv('HOME="'.$temp_path);
}
require_once "PEAR/Installer.php";

// no UI is needed for these tests
$ui = false;
$installer = new PEAR_Installer($ui);
$curdir = getcwd();
chdir(dirname(__FILE__));

echo "test _installFile():\n";
$fp = fopen($temp_path . DIRECTORY_SEPARATOR . 'tmp' . DIRECTORY_SEPARATOR . 'installer2.phpt.testfile.php', 'w');
fwrite($fp, 'a');
fclose($fp);
// pretend we just parsed a package.xml
$installer->pkginfo = array('package' => 'Foo');

echo "install as role=\"php\":\n";
var_dump($installer->_installFile('installer2.phpt.testfile.php', array('role' => 'php'),
    $temp_path . DIRECTORY_SEPARATOR . 'tmp', array()));
echo 'file ext/.tmpinstaller2.phpt.testfile.php exists? => ';
echo (file_exists($temp_path . DIRECTORY_SEPARATOR . 'php' . DIRECTORY_SEPARATOR .
    '.tmpinstaller2.phpt.testfile.php') ? "yes\n" : "no\n");

echo "install as role=\"ext\":\n";
var_dump($installer->_installFile('installer2.phpt.testfile.php', array('role' => 'ext'),
    $temp_path . DIRECTORY_SEPARATOR . 'tmp', array()));
echo 'file php/.tmpinstaller2.phpt.testfile.php exists? => ';
echo (file_exists($temp_path . DIRECTORY_SEPARATOR . 'ext' . DIRECTORY_SEPARATOR .
    '.tmpinstaller2.phpt.testfile.php') ? "yes\n" : "no\n");

echo "install as role=\"data\":\n";
var_dump($installer->_installFile('installer2.phpt.testfile.php', array('role' => 'data'),
    $temp_path . DIRECTORY_SEPARATOR . 'tmp', array()));
echo 'file data/.tmpinstaller2.phpt.testfile.php exists? => ';
echo (file_exists($temp_path . DIRECTORY_SEPARATOR . 'data' . DIRECTORY_SEPARATOR .
    'Foo' . DIRECTORY_SEPARATOR . '.tmpinstaller2.phpt.testfile.php') ? "yes\n" : "no\n");

echo "install as role=\"doc\":\n";
var_dump($installer->_installFile('installer2.phpt.testfile.php', array('role' => 'doc'),
    $temp_path . DIRECTORY_SEPARATOR . 'tmp', array()));
echo 'file doc/.tmpinstaller2.phpt.testfile.php exists? => ';
echo (file_exists($temp_path . DIRECTORY_SEPARATOR . 'doc' . DIRECTORY_SEPARATOR .
    'Foo' . DIRECTORY_SEPARATOR . '.tmpinstaller2.phpt.testfile.php') ? "yes\n" : "no\n");

echo "install as role=\"test\":\n";
var_dump($installer->_installFile('installer2.phpt.testfile.php', array('role' => 'test'),
    $temp_path . DIRECTORY_SEPARATOR . 'tmp', array()));
echo 'file test/.tmpinstaller2.phpt.testfile.php exists? => ';
echo (file_exists($temp_path . DIRECTORY_SEPARATOR . 'test' . DIRECTORY_SEPARATOR .
    'Foo' . DIRECTORY_SEPARATOR . '.tmpinstaller2.phpt.testfile.php') ? "yes\n" : "no\n");

echo "install as role=\"script\":\n";
var_dump($installer->_installFile('installer2.phpt.testfile.php', array('role' => 'script'),
    $temp_path . DIRECTORY_SEPARATOR . 'tmp', array()));
echo 'file bin/.tmpinstaller2.phpt.testfile.php exists? => ';
echo (file_exists($temp_path . DIRECTORY_SEPARATOR . 'bin' . DIRECTORY_SEPARATOR .
    '.tmpinstaller2.phpt.testfile.php') ? "yes\n" : "no\n");

$installer->rollbackFileTransaction();
unlink($temp_path . DIRECTORY_SEPARATOR . 'tmp' . DIRECTORY_SEPARATOR . 'installer2.phpt.testfile.php');

//cleanup
chdir($curdir);
unlink ($temp_path . DIRECTORY_SEPARATOR . 'pear.conf');
unlink ($temp_path . DIRECTORY_SEPARATOR . 'pear.ini');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'php');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'data' . DIRECTORY_SEPARATOR . 'Foo');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'data');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'doc' . DIRECTORY_SEPARATOR . 'Foo');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'doc');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'test' . DIRECTORY_SEPARATOR . 'Foo');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'test');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'script');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'ext');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'tmp');
rmdir($temp_path . DIRECTORY_SEPARATOR . 'bin');
rmdir($temp_path);
?>
--GET--
--POST--
--EXPECT--
test _installFile():
install as role="php":
int(1)
file ext/.tmpinstaller2.phpt.testfile.php exists? => yes
install as role="ext":
int(1)
file php/.tmpinstaller2.phpt.testfile.php exists? => yes
install as role="data":
int(1)
file data/.tmpinstaller2.phpt.testfile.php exists? => yes
install as role="doc":
int(1)
file doc/.tmpinstaller2.phpt.testfile.php exists? => yes
install as role="test":
int(1)
file test/.tmpinstaller2.phpt.testfile.php exists? => yes
install as role="script":
int(1)
file bin/.tmpinstaller2.phpt.testfile.php exists? => yes