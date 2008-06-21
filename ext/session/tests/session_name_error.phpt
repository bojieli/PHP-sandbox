--TEST--
Test session_name() function : error functionality
--INI--
session.save_path=
session.name=PHPSESSID
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php

ob_start();

/* 
 * Prototype : string session_name([string $name])
 * Description : Get and/or set the current session name
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_name() : error functionality ***\n";

// Get an unset variable
$unset_var = 10;
unset($unset_var);

class classA
{
    public function __toString() {
        return "Hello World!";
    }
}

$heredoc = <<<EOT
Hello World!
EOT;

$fp = fopen(__FILE__, "r");

// Unexpected values to be passed as arguments
$inputs = array(

       // Integer data
/*1*/  0,
       1,
       12345,
       -2345,

       // Float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // Null data
/*10*/ NULL,
       null,

       // Boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,
       
       // Empty strings
/*16*/ "",
       '',

       // Invalid string data
/*18*/ "Nothing",
       'Nothing',
       $heredoc,
       
       // Object data
/*21*/ new classA(),

       // Undefined data
/*22*/ @$undefined_var,

       // Unset data
/*23*/ @$unset_var,

       // Resource variable
/*24*/ $fp
);

$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(session_name($input));
    $iterator++;
};

fclose($fp);
echo "Done";
ob_end_flush();
?>
--EXPECTF--
*** Testing session_name() : error functionality ***

-- Iteration 1 --
string(9) "PHPSESSID"

-- Iteration 2 --
string(1) "0"

-- Iteration 3 --
string(1) "1"

-- Iteration 4 --
string(5) "12345"

-- Iteration 5 --
string(5) "-2345"

-- Iteration 6 --
string(4) "10.5"

-- Iteration 7 --
string(5) "-10.5"

-- Iteration 8 --
string(12) "123456789000"

-- Iteration 9 --
string(13) "1.23456789E-9"

-- Iteration 10 --
string(3) "0.5"

-- Iteration 11 --
string(0) ""

-- Iteration 12 --
string(0) ""

-- Iteration 13 --
string(1) "1"

-- Iteration 14 --
string(0) ""

-- Iteration 15 --
string(1) "1"

-- Iteration 16 --
string(0) ""

-- Iteration 17 --
string(0) ""

-- Iteration 18 --
string(0) ""

-- Iteration 19 --
string(7) "Nothing"

-- Iteration 20 --
string(7) "Nothing"

-- Iteration 21 --
string(12) "Hello World!"

-- Iteration 22 --
string(12) "Hello World!"

-- Iteration 23 --
string(0) ""

-- Iteration 24 --

Warning: session_name() expects parameter 1 to be string, resource given in /home/felipe/php5/ext/session/tests/session_name_error.php on line 81
NULL
Done

