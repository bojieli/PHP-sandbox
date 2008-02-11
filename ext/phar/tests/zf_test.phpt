--TEST--
test broken app
--INI--
phar.readonly=0
--FILE--
<?php
$file = "zfapp";
$tgz_file = dirname(__FILE__) . "/$file.tgz";
chdir(dirname(__FILE__));
$phar_file = basename(__FILE__, '.php') . '.phar.php';
@unlink($phar_file);
copy($tgz_file, $phar_file);
$a = new Phar($phar_file);
$a->startBuffering();
$a->setStub("<?php
Phar::interceptFileFuncs();
Phar::webPhar('$file.phar', 'html/index.php');
echo 'BlogApp is intended to be executed from a web browser\n';
exit -1;
__HALT_COMPILER();
");
$a->stopBuffering();
foreach(new RecursiveIteratorIterator($a) as $f) {
echo str_replace('\\', '/', $f->getPathName()) . "\n";
}
?>
===DONE===
--CLEAN--
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.phar.php');
__HALT_COMPILER();
?>
--EXPECTF--
phar://%szf_test.phar.php/application/default/controllers/ErrorController.php
phar://%szf_test.phar.php/application/default/controllers/IndexController.php
phar://%szf_test.phar.php/application/default/views/scripts/error/error.phtml
phar://%szf_test.phar.php/application/default/views/scripts/index/index.phtml
phar://%szf_test.phar.php/html/.htaccess
phar://%szf_test.phar.php/html/index.php
===DONE===
