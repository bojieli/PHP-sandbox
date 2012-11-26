<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('blog')) {
	dl('blog.' . PHP_SHLIB_SUFFIX);
}
$module = 'blog';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";

$name = 'boj'.'google';
var_dump(get_appid_by_field('username', $name));
var_dump(get_appid_by_field('username', 'boj'));
?>
