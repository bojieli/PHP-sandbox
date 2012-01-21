--TEST--
tidy_diagnose()
--SKIPIF--
<?php if (!extension_loaded("tidy")) print "skip"; ?>
--FILE--
<?php 
$a = tidy_parse_string('<HTML></HTML>');
var_dump(tidy_diagnose($a));
echo tidy_get_error_buffer($a);

$html = <<< HTML
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2//EN">
<html>
<head><title>foo</title></head>
<body><p>hello</p></body>
</html>
HTML;
$a = tidy_parse_string($html);
var_dump(tidy_diagnose($a));
echo tidy_get_error_buffer($a);
?>
--EXPECT--
bool(true)
line 1 column 1 - Warning: missing <!DOCTYPE> declaration
line 1 column 7 - Warning: discarding unexpected </html>
line 1 column 14 - Warning: inserting missing 'title' element
Info: Document content looks like HTML 3.2
3 warnings, 0 errors were found!
bool(true)
Info: Document content looks like HTML 3.2
No warnings or errors were found.
