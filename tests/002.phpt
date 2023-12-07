--TEST--
test1() Basic test
--EXTENSIONS--
hooks
--FILE--
<?php
$ret = test1();

var_dump($ret);
?>
--EXPECT--
The extension hooks is loaded and working!
NULL
