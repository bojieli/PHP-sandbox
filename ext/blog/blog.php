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

$ret = request_daemon('sync', 'http-get', array('url'=>'http://api.wordpress.org/stats/php/1.0/'));
var_dump($ret);

var_dump(http_get('stats/php/1.0/'));
var_dump(http_post('plugins/info/1.0/', array('action' => 'test', 'request' => 1000)));
var_dump(install_blog_filesystem(1));
?>
