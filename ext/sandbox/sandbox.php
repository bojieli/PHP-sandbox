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
include "verify.inc.php";
$appname = 'appname'.$rand;
$email = 'boj'.$rand.'@mail.ustc.edu.cn';
$username = 'user'.$rand;
var_dump(checkname($appname));
var_dump(checkemail($email));
var_dump(checkfolder($username));
$appid = create_app($appname, $username, $email, 'password1');
echo "created appid: $appid\n";
mysql_query("CREATE TABLE test (id INT(10))");
var_dump(get_appinfo($appid));

var_dump(install_blog_filesystem('appname'.$rand));
?>
