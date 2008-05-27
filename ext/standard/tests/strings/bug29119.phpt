--TEST--
Bug #29119 (html_entity_decode() misbehaves with UTF-8)
--INI--
unicode.runtime_encoding=UTF-8
--FILE--
<?php
var_dump(bin2hex((binary)html_entity_decode('&ensp;&thinsp;&lsquo;&dagger;&prime;&frasl;&euro;', ENT_QUOTES, 'UTF-8')));
?>
--EXPECT--
unicode(42) "e28082e28089e28098e280a0e280b2e28184e282ac"
