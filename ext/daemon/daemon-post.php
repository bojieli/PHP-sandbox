<?php
var_dump(http_get('http://api.wordpress.org/stats/php/1.0/'));

$url = "http://api.wordpress.org/plugins/info/1.0/";
$data = array(
    'action' => 'query_plugins',
    'request' => urldecode("O%3A8%3A%22stdClass%22%3A3%3A%7Bs%3A4%3A%22page%22%3Bi%3A1%3Bs%3A8%3A%22per_page%22%3Bi%3A30%3Bs%3A6%3A%22search%22%3Bs%3A6%3A%22google%22%3B%7D")
    );
var_dump($data);
$response = http_post($url, $data);
echo "Status: ".$response['status']."\n";
echo "Length: ".strlen($response['data'])."\n";
echo "First Line: ".substr($response['data'],0,100)."\n";

http_get('http://www.google.com/');

$data = http_get('http://cn.wordpress.org/feed/');
echo "length: ".strlen($data['data'])."\n";
?>
