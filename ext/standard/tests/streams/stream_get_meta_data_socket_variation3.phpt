--TEST--
Testing stream_get_meta_data() "blocked" field on a udp socket
--FILE--
<?php

/* Setup socket server */
$server = stream_socket_server('tcp://127.0.0.1:31337');

/* Connect to it */
$client = fsockopen('tcp://127.0.0.1:31337');

/* Accept that connection */
$socket = stream_socket_accept($server);

var_dump(stream_get_meta_data($client));

echo "\n\nSet blocking to false:\n";
var_dump(socket_set_blocking($client, 0));
var_dump(stream_get_meta_data($client));

echo "\n\nSet blocking to true:\n";
var_dump(socket_set_blocking($client, 1));
var_dump(stream_get_meta_data($client));

fclose($client);
fclose($socket);
fclose($server);

?>
--EXPECT--
array(8) {
  ["stream_type"]=>
  string(10) "tcp_socket"
  ["mode"]=>
  string(2) "r+"
  ["unread_bytes"]=>
  int(0)
  ["unread_chars"]=>
  int(0)
  ["seekable"]=>
  bool(false)
  ["timed_out"]=>
  bool(false)
  ["blocked"]=>
  bool(true)
  ["eof"]=>
  bool(false)
}


Set blocking to false:
bool(true)
array(8) {
  ["stream_type"]=>
  string(10) "tcp_socket"
  ["mode"]=>
  string(2) "r+"
  ["unread_bytes"]=>
  int(0)
  ["unread_chars"]=>
  int(0)
  ["seekable"]=>
  bool(false)
  ["timed_out"]=>
  bool(false)
  ["blocked"]=>
  bool(false)
  ["eof"]=>
  bool(false)
}


Set blocking to true:
bool(true)
array(8) {
  ["stream_type"]=>
  string(10) "tcp_socket"
  ["mode"]=>
  string(2) "r+"
  ["unread_bytes"]=>
  int(0)
  ["unread_chars"]=>
  int(0)
  ["seekable"]=>
  bool(false)
  ["timed_out"]=>
  bool(false)
  ["blocked"]=>
  bool(true)
  ["eof"]=>
  bool(false)
}
--UEXPECT--
array(8) {
  [u"stream_type"]=>
  unicode(10) "tcp_socket"
  [u"mode"]=>
  unicode(2) "r+"
  [u"unread_bytes"]=>
  int(0)
  [u"unread_chars"]=>
  int(0)
  [u"seekable"]=>
  bool(false)
  [u"timed_out"]=>
  bool(false)
  [u"blocked"]=>
  bool(true)
  [u"eof"]=>
  bool(false)
}


Set blocking to false:
bool(true)
array(8) {
  [u"stream_type"]=>
  unicode(10) "tcp_socket"
  [u"mode"]=>
  unicode(2) "r+"
  [u"unread_bytes"]=>
  int(0)
  [u"unread_chars"]=>
  int(0)
  [u"seekable"]=>
  bool(false)
  [u"timed_out"]=>
  bool(false)
  [u"blocked"]=>
  bool(false)
  [u"eof"]=>
  bool(false)
}


Set blocking to true:
bool(true)
array(8) {
  [u"stream_type"]=>
  unicode(10) "tcp_socket"
  [u"mode"]=>
  unicode(2) "r+"
  [u"unread_bytes"]=>
  int(0)
  [u"unread_chars"]=>
  int(0)
  [u"seekable"]=>
  bool(false)
  [u"timed_out"]=>
  bool(false)
  [u"blocked"]=>
  bool(true)
  [u"eof"]=>
  bool(false)
}
