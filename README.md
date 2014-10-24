phpqrencode
===========

基于PHP-CPP+libqrencode+libpng写的用于生成二维码的PHP扩展

用法：
echo qrencode("hello world!", 10, "./test.png");
返回1表示成功，返回0表示失败！

编译：
1, 编译php-cpp，详见我的博客
   http://blog.csdn.net/leonpengweicn/article/details/40425853
   
2, 编译libqrencode
   curl http://fukuchi.org/works/qrencode/qrencode-3.4.4.tar.gz -o qrencode-3.4.4.tar.gz
   tar zxvf qrencode-3.4.4.tar.gz
   cd qrencode-3.4.4
   make & sudo make install
   
3, 编译libpng
   brew update
   brew install libpng
   
4, 编译phpqrencode
   git clone https://github.com/Leon2012/phpqrencode
   cd phpqrencode
   #如果不是mac+xampp则需要修改Makefile里的EXTENSION_DIR内容
   make

5, 拷贝生成的phpqrencode.so 到php的扩展目录，修改php.ini
   extension=phpqrencode.so
   

   

