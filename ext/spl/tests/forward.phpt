--TEST--
SPL: forward
--SKIPIF--
<?php if (!extension_loaded("spl")) print "skip"; ?>
--FILE--
<?php
class c implements spl::forward_assoc {

	public $max = 3;
	public $num = 0;

	function current() {
		echo __CLASS__ . '::' . __FUNCTION__ . "\n";
		return $this->num;
	}
	function next() {
		echo __CLASS__ . '::' . __FUNCTION__ . "\n";
		$this->num++;
	}
	function has_more() {
		echo __CLASS__ . '::' . __FUNCTION__ . "\n";
		return $this->num < $this->max;
	}
	function key() {
		echo __CLASS__ . '::' . __FUNCTION__ . "\n";
		switch($this->num) {
			case 0: return "1st";
			case 1: return "2nd";
			case 2: return "3rd";
			default: return "???";
		}
	}
}

$i = new c();

$c_info = array(class_name($i) => array('inheits' => class_parents($i), 'implements' => class_implements($i)));
print_r($c_info);

echo "1st try\n";
foreach($i as $w) {
	echo "object:$w\n";
}

echo "2nd try\n";

foreach($i as $v => $w) {
	echo "object:$v=>$w\n";
}

echo "3rd try\n";
$i->num = 0;

foreach($i as $v => $w) {
	echo "object:$v=>$w\n";
}

print "Done\n";
?>
--EXPECT--
Array
(
    [c] => Array
        (
            [inheits] => Array
                (
                )

            [implements] => Array
                (
                    [spl::forward_assoc] => spl::forward_assoc
                    [spl::assoc] => spl::assoc
                    [spl::forward] => spl::forward
                )

        )

)
1st try
c::has_more
c::current
c::key
object:0
c::next
c::has_more
c::current
c::key
object:1
c::next
c::has_more
c::current
c::key
object:2
c::next
c::has_more
2nd try
c::has_more
3rd try
c::has_more
c::current
c::key
object:1st=>0
c::next
c::has_more
c::current
c::key
object:2nd=>1
c::next
c::has_more
c::current
c::key
object:3rd=>2
c::next
c::has_more
Done