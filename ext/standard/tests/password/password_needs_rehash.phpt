--TEST--
Test normal operation of password_needs_rehash()
--FILE--
<?php
//-=-=-=-

// Invalid Hash, always rehash
var_dump(password_needs_rehash('', PASSWORD_BCRYPT));

// Valid, as it's an unknown algorithm
var_dump(password_needs_rehash('', 0));

// Valid with cost the same
var_dump(password_needs_rehash('$2y$10$MTIzNDU2Nzg5MDEyMzQ1Nej0NmcAWSLR.oP7XOR9HD/vjUuOj100y', PASSWORD_BCRYPT, array('cost' => 10)));

// Valid with cost the same, additional params
var_dump(password_needs_rehash('$2y$10$MTIzNDU2Nzg5MDEyMzQ1Nej0NmcAWSLR.oP7XOR9HD/vjUuOj100y', PASSWORD_BCRYPT, array('cost' => 10, 'foo' => 3)));

// Invalid, different (lower) cost
var_dump(password_needs_rehash('$2y$10$MTIzNDU2Nzg5MDEyMzQ1Nej0NmcAWSLR.oP7XOR9HD/vjUuOj100y', PASSWORD_BCRYPT, array('cost' => 09)));

// Invalid, different (higher) cost
var_dump(password_needs_rehash('$2y$10$MTIzNDU2Nzg5MDEyMzQ1Nej0NmcAWSLR.oP7XOR9HD/vjUuOj100y', PASSWORD_BCRYPT, array('cost' => 11)));

// Valid with cost the default (may need to be updated as the default cost increases)
var_dump(password_needs_rehash('$2y$10$MTIzNDU2Nzg5MDEyMzQ1Nej0NmcAWSLR.oP7XOR9HD/vjUuOj100y', PASSWORD_BCRYPT));


echo "OK!";
?>
--EXPECT--
bool(true)
bool(false)
bool(false)
bool(false)
bool(true)
bool(true)
bool(false)
OK!
