--TEST--
ZE2 factory and singleton, test 7
--SKIPIF--
<?php if (version_compare(zend_version(), '2.0.0-dev', '<')) die('skip ZendEngine 2 needed'); ?>
--FILE--
<?php
class test {

  protected function __clone() {
  }
}

$obj = new test;
$clone = $obj->__clone();
$obj = NULL;

echo "Done\n";
?>
--EXPECTF--
Fatal error: Call to protected test::__clone() from context '' %sfactory_and_singleton_007.php on line %d
