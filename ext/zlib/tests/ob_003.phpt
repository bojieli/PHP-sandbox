--TEST--
zlib.output_compression
--SKIPIF--
<?php
if (!extension_loaded("zlib")) die("skip need ext/zlib");
if (false === stristr(PHP_SAPI, "cgi)) die("skip need sapi/cgi");
?>
--INI--
zlib.output_compression=0
--ENV--
HTTP_ACCEPT_ENCODING=gzip
--FILE--
<?php
ini_set("zlib.output_compression", 1);
echo "hi\n";
?>
--EXPECTF--
%s
Content-Encoding: gzip
Vary: Accept-Encoding
%s

�%s
