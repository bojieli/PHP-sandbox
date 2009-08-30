--TEST--
file_get_contents() test using basic syntax
--CREDITS--
"Blanche V.N." <valerie_nare@yahoo.fr>
--FILE--
<?php
	$file_content = "Bienvenue au CodeFest a Montreal";
	$temp_filename = dirname(__FILE__)."/fichier_a_lire.txt";
	$handle = fopen($temp_filename,"w");
	fwrite($handle,$file_content);
	fclose($handle);
	$var = file_get_contents($temp_filename);
	echo $var;
?>
--CLEAN--
<?php
	$temp_filename = dirname(__FILE__)."/fichier_a_lire.txt";
	unlink($temp_filename);
?>
--EXPECTF--
Notice: fwrite(): 32 character unicode buffer downcoded for binary stream runtime_encoding in %s.php on line %d
Bienvenue au CodeFest a Montreal
