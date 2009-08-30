--TEST--
Test function getservbyname() by substituting argument 1 with array values.
--FILE--
<?php


echo "*** Test substituting argument 1 with array values ***\n";

$protocol = "tcp";


$index_array = array(1, 2, 3);
$assoc_array = array(1 => 'one', 2 => 'two');

$variation_array = array(
  'empty array' => array(),
  'int indexed array' => $index_array,
  'associative array' => $assoc_array,
  'nested arrays' => array('foo', $index_array, $assoc_array),
  );


foreach ( $variation_array as $var ) {
  var_dump(getservbyname( $var ,  $protocol ) );
}
?>
--EXPECTF--
*** Test substituting argument 1 with array values ***

Notice: Array to string conversion in %s.php on line %d
bool(false)

Notice: Array to string conversion in %s.php on line %d
bool(false)

Notice: Array to string conversion in %s.php on line %d
bool(false)

Notice: Array to string conversion in %s.php on line %d
bool(false)
