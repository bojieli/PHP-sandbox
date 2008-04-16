--TEST--
Phar with meta-data (write) zip-based
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip");?>
--INI--
phar.require_hash=0
phar.readonly=0
--FILE--
<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip.php';
$f2name = dirname(__FILE__) . '/files/metadata.phar.zip';
$pname = 'phar://' . $fname;
$p2name = 'phar://' . $f2name;

$file = "<?php __HALT_COMPILER(); ?>";

$files = array();
$files['a'] = array('cont' => 'a');
$files['b'] = array('cont' => 'b', 'meta' => 'hi there');
$files['c'] = array('cont' => 'c', 'meta' => array('hi', 'there'));
$files['d'] = array('cont' => 'd', 'meta' => array('hi'=>'there','foo'=>'bar'));

foreach($files as $name => $cont) {
	var_dump(file_get_contents($p2name.'/'.$name));
}

$phar = new Phar($fname);
$phar->startBuffering();
$phar['a']->setMetadata(42);
$phar['b']->setMetadata(NULL);
$phar['c']->setMetadata(array(25, 'foo'=>'bar'));
$phar['d']->setMetadata(true);

foreach($files as $name => $cont) {
	var_dump($phar[$name]->getMetadata());
}
$phar->stopBuffering();

unset($phar);

copy($f2name, $fname);

$phar = new Phar($fname2);

foreach($files as $name => $cont) {
	var_dump(file_get_contents($pname.'/'.$name));
}

foreach($files as $name => $cont) {
	var_dump($phar[$name]->getMetadata());
}
?>
===DONE===
--CLEAN--
<?php unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.clean.php') . '.phar.zip.php'); ?>
--EXPECT--
string(1) "a"
string(1) "b"
string(1) "c"
string(1) "d"
int(42)
NULL
array(2) {
  [0]=>
  int(25)
  ["foo"]=>
  string(3) "bar"
}
bool(true)
string(1) "a"
string(1) "b"
string(1) "c"
string(1) "d"
int(42)
NULL
array(2) {
  [0]=>
  int(25)
  ["foo"]=>
  string(3) "bar"
}
bool(true)
===DONE===
