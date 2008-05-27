--TEST--
stream_copy_to_stream() tests
--FILE--
<?php

$initial_file = dirname(__FILE__).'/bug38086.txt';
$new_file = dirname(__FILE__).'/bug38086_1.txt';

$src = fopen($initial_file, 'r');
stream_filter_append($src, "string.rot13", STREAM_FILTER_READ);

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest, 0));
fclose($src); fclose($dest);

var_dump(file_get_contents($new_file));
unlink($new_file);

/* --- */

$src = fopen($initial_file, 'r');
stream_filter_append($src, "string.rot13", STREAM_FILTER_READ);

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest, -1));
fclose($src); fclose($dest);

var_dump(file_get_contents($new_file));
unlink($new_file);

/* --- */

$src = fopen($initial_file, 'r');
stream_filter_append($src, "string.rot13", STREAM_FILTER_READ);

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest));
fclose($src); fclose($dest);

var_dump(file_get_contents($new_file));
unlink($new_file);

/* --- */

$src = fopen($initial_file, 'r');

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest));
fclose($src); fclose($dest);

var_dump(file_get_contents($new_file));
unlink($new_file);

/* --- */

$src = fopen($initial_file, 'r');

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest, 1000000));
fclose($src); fclose($dest);

var_dump(file_get_contents($new_file));
unlink($new_file);

/* --- */

$src = fopen($initial_file, 'r');

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest, 10));
fclose($src); fclose($dest);

var_dump(file_get_contents($new_file));
unlink($new_file);

/* --- */

$src = fopen($initial_file, 'r');

$dest = fopen($new_file, 'w');
var_dump(stream_copy_to_stream($src, $dest, -1));
fclose($src); fclose($dest);

var_dump(file_get_contents($new_file));
unlink($new_file);

echo "Done\n";
?>
--EXPECT--
int(0)
string(0) ""
int(134)
string(134) "Nabgure qnl
Jura gur cnvaf bs yvsr jba'g one zl jnl
V'yy oernx gurfr punvaf
Gung ubyq zr qbja
V'yy grne lbh qbja vagb zl cevingr uryy
"
int(134)
string(134) "Nabgure qnl
Jura gur cnvaf bs yvsr jba'g one zl jnl
V'yy oernx gurfr punvaf
Gung ubyq zr qbja
V'yy grne lbh qbja vagb zl cevingr uryy
"
int(134)
string(134) "Another day
When the pains of life won't bar my way
I'll break these chains
That hold me down
I'll tear you down into my private hell
"
int(134)
string(134) "Another day
When the pains of life won't bar my way
I'll break these chains
That hold me down
I'll tear you down into my private hell
"
int(10)
string(10) "Another da"
int(134)
string(134) "Another day
When the pains of life won't bar my way
I'll break these chains
That hold me down
I'll tear you down into my private hell
"
Done
