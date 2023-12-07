--TEST--
Check if hooks is loaded
--EXTENSIONS--
hooks
--FILE--
<?php
echo 'The extension "hooks" is available';
?>
--EXPECT--
The extension "hooks" is available
