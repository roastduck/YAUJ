YAUJ
====

*一个用于高度定制评测过程的语言（未完工）*

简要概述
----

实现了一个类C的语言，内建了若干评测相关的函数，具体用法参见语法文件和例子。编译需安装libsandbox（https://github.com/openjudge/sandbox）。
计划实现一个，有语法高亮/补全/提示功能（用codemirror实现）的编辑器，并提供传统题目的模板生成。计划写成js类的形式方便移植。

文件结构  
src/config.h 编译期的配置  
src/function.h 内建函数声明（有注释）  
src/function.cpp 内建函数实现  
src/interpreter.h 核心语言声明  
src/interpreter.cpp 核心语言实现  
src/mystr.h  
src/mystr.c 语法分析辅助函数  
src/parser.l 词法文件  
src/parser.y 语法文件  
editor/ 编辑器（待完成）  
examples/ 例子  
lex.yy.c parser.tab.h parser.tab.c 为语法分析器生成的文件，为了能在不安装flex和bison的情况下使用。  
