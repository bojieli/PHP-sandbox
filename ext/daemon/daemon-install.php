<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('daemon')) {
	dl('daemon.' . PHP_SHLIB_SUFFIX);
}
$module = 'daemon';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";

var_dump(install_blog_filesystem('test'));
var_dump(random_string(40));
var_dump(sendmail('boj@mail.ustc.edu.cn', 'Email Activation for blog.ustc.edu.cn', 'Please click the following link to activate: http://blog.ustc.edu.cn/activate.php?token='.random_string(40)));
