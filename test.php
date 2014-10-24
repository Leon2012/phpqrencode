<?php 
echo qrencode_apiversion();
echo qrencode("hello world!", 10, "./test.png");
?>