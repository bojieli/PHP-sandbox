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
$rand = random_string(5);
$appid = create_app('appname'.$rand, 'username'.$rand, 'email'.$rand, 'password1');
echo "created appid: $appid\n";
mysql_query("CREATE TABLE test (id INT(10))");
var_dump(get_appinfo($appid));
?>
