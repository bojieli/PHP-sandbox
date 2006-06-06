--TEST--
extending the same interface twice
--FILE--
<?php

interface foo {
}

interface bar extends foo, foo {
}	

echo "Done\n";
?>
--EXPECTF--	
Fatal error: Cannot implement previously implemented interface foo in %s on line %d
