--TEST--
SPL: foreach and break
--SKIPIF--
<?php if (!extension_loaded("spl")) print "skip"; ?>
--FILE--
<?php
class c_iter implements spl::forward_assoc {

	private $obj;
	private $num = 0;

	function __construct($obj) {
		$this->obj = $obj;
	}
	function current() {
		echo __METHOD__ . "\n";
		return $this->num;
	}
	function next() {
		echo __METHOD__ . "\n";
		$this->num++;
	}
	function has_more() {
		$more = $this->num < $this->obj->max;
		echo __METHOD__ . ' = ' .($more ? 'true' : 'false') . "\n";
		return $more;
	}
	function key() {
		echo __METHOD__ . "\n";
		switch($this->num) {
			case 0: return "1st";
			case 1: return "2nd";
			case 2: return "3rd";
			default: return "???";
		}
	}
}
	
class c implements spl::iterator {

	public $max = 3;

	function new_iterator() {
		echo __METHOD__ . "\n";
		return new c_iter($this);
	}
}

$t = new c();

foreach($t as $v) {
	foreach($t as $w) {
		echo "double:$v:$w\n";
		break;
	}
}

print "Done\n";
?>
--EXPECT--
c::new_iterator
c_iter::has_more = true
c_iter::current
c_iter::key
c::new_iterator
c_iter::has_more = true
c_iter::current
c_iter::key
double:0:0
c_iter::next
c_iter::has_more = true
c_iter::current
c_iter::key
c::new_iterator
c_iter::has_more = true
c_iter::current
c_iter::key
double:1:0
c_iter::next
c_iter::has_more = true
c_iter::current
c_iter::key
c::new_iterator
c_iter::has_more = true
c_iter::current
c_iter::key
double:2:0
c_iter::next
c_iter::has_more = false
Done
