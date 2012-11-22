<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('sandbox')) {
	dl('sandbox.' . PHP_SHLIB_SUFFIX);
}
$module = 'sandbox';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}

echo "my appid: ".get_appid()."\n";
echo "created appid: ".create_app('appname1', 'username1', 'email1', 'password1')."\n";
mysql_query("CREATE TABLE test (id INT(10))");
?>
