--TEST--
Bug #45186 (__call depends on __callstatic in class scope)
--FILE--
<?php

class bar  {
	public function __call($a, $b) {
		print "__call:\n";
		var_dump($a);
	}
	static public function __callstatic($a, $b) {
		print "__callstatic:\n";
		var_dump($a);
	}
	public function test() {
		self::ABC();
		bar::ABC();
		call_user_func(array('BAR', 'xyz'));
		call_user_func('BAR::www');
		call_user_func(array('self', 'y'));
		call_user_func('self::y');
	}
	static function x() { 
		print "ok\n";
	}
}

$x = new bar;

$x->test();

call_user_func(array('BAR','x'));
call_user_func('BAR::www');
call_user_func('self::y');

?>
--EXPECTF--
__callstatic:
unicode(3) "ABC"
__callstatic:
unicode(3) "ABC"
__call:
unicode(3) "xyz"
__callstatic:
unicode(3) "www"
__call:
unicode(1) "y"
__callstatic:
unicode(1) "y"
ok
__callstatic:
unicode(3) "www"

Warning: call_user_func() expects parameter 1 to be a valid callback, cannot access self:: when no class scope is active in %sbug45186.php on line 31
