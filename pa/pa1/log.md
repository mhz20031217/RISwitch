---
title: Learning log
author: Caterpillar
date: 2023-08-09
---
1. Test if preprocessor symbol is defined inside macro  
    Take advantage of the macro expanding process. If the macro is expanded (the literal string and the expanded result differ), then it's defined. The comparing is done through `strcmp` two constant strings.  
    - [macro.h:39](../../nemu/include/macro.h)
    - [Test if preprocessor symbol is defined inside macro](https://stackoverflow.com/questions/26099745/test-if-preprocessor-symbol-is-defined-inside-macro)

2. Parsing cmd arguments with `getopt()`, `getopt_long()`  
    [ 命令行选项解析函数(C语言)：getopt()和getopt_long()](https://www.cnblogs.com/chenliyang/p/6633739.html)
