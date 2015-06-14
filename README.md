YAUJ
====

***一个用于高度定制评测过程的语言***  
实现了一个具有动态类型的编译型类C的语言，内建若干评测相关的函数。YAUJ程序使用全局变量submission，filemode和result获取提交并输出结果。exec，compile等函数会抛出自定义异常避免复杂的错误判断，此外为了简化用户的评测脚本，某些函数需传入数据编号以便在该函数中直接写入result变量而无需用户编写相关语句，但用户仍可以在脚本中手动给result赋值。具体语法见src/parser.l,src/parser.y，内建函数见src/function.h，或参阅例子。  
*附件*：  
src/daemon.cpp为一个jsonrpc服务器，可接收网页服务器的评测请求并交予YAUJ执行。 
editor是一个配置好高亮的编辑器，可嵌入网页中使用。  
src/vjudge\_hack.cpp是一个将程序提交至vjudge.net的工具。  
*某些文件说明*：  
src/function.h 内建函数声明（有注释）  
src/function.cpp 内建函数实现  
src/interpreter.h 核心语言声明   
src/interpreter.cpp 核心语言实现  
src/mystr.c 语法分析辅助函数  
src/parser.l 词法文件  
src/parser.y 语法文件  
build/lex.yy.c build/parser.tab.h build/parser.tab.c 为语法分析器生成的文件，为了能在不安装flex和bison的情况下使用而上传。  

安装
------
*依赖*：  
g++,gcc,fpc：评测时需要的编译器  
dos2unix：执行Arbiter的SPJ需要用到的程序  
libjson-rpc-cpp  
libboost-regex-dev  
libcurl：vjudge\_hack需要  
评测使用vfk提供的闭源沙盒，目前不开放下载。可以考虑使用libsandbox替代。
*编译*：  
编译前请自行调整路径等参数  
执行  

    make
    sudo make install

会编译并安装yauj\_daemon作为rpc服务器接收请求，并建立resource文件夹包含导入试题时编译脚本所需文件。  

运行
------
启动daemon用rpc请求进行评测时，需按如下步骤发出请求：（具体rpc过程名参见src/spec.json）  
1. 同步题目数据  
2. preserve提交，用以避免同时评测过多提交。同时在此过程中将会同步提交的文件。  
3. 执行提交  
如需手动评测，流程如下：  
1. 建立空文件夹  
2. 将数据（包括YAUJ脚本）复制进来，编译YAUJ脚本  
3. 将选手提交复制进来  
4. 运行yauj\_judge   
注意，daemon将进行文件操作，使用过程中应注意避免使用不同权限执行程序造成的文件权限混乱。应使用同一个普通权限用户执行。此外，daemon编译YAUJ时使用的makefile应预先生成，其中可引用resource文件夹中的makefile，但还应该编译此评测所需的SPJ。  
