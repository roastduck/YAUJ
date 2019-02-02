# YAUJ

***一个用于高度定制评测过程的语言***

实现了一个具有动态类型的编译型类 C 的语言，内建若干评测相关的函数。

YAUJ 程序使用全局变量 submission，filemode 和 result 获取提交并输出结果。exec，compile 等函数会抛出自定义异常避免复杂的错误判断，此外为了简化用户的评测脚本，某些函数需传入数据编号以便在该函数中直接写入 result 变量而无需用户编写相关语句，但用户仍可以在脚本中手动给 result 赋值。

具体语法见 [src/parser.l](src/parser.l)，[src/parser.y](src/parser.y)，内建函数见 [src/function.h](src/function.h)，或参阅例子。

#### 源码说明

- src/daemon.cpp 为一个 jsonrpc 服务器，可接收网页服务器的评测请求并交予 YAUJ 执行
- editor 是一个配置好高亮的编辑器，可嵌入网页中使用
- src/vjudge\_hack.cpp 是一个将程序提交至 vjudge.net 的工具

其他：

- src/function.h 内建函数声明（有注释）
- src/function.cpp 内建函数实现
- src/interpreter.h 核心语言声明
- src/interpreter.cpp 核心语言实现
- src/mystr.c 语法分析辅助函数
- src/parser.l 词法文件
- src/parser.y 语法文件

注意：

- build/lex.yy.c
- build/parser.tab.h
- build/parser.tab.c

为语法分析器生成的文件，为了能在不安装 flex 和 bison 的情况下使用而上传。

## 安装

### 依赖

- g++, gcc, fpc：评测时需要的编译器
- dos2unix：执行Arbiter的SPJ需要用到的程序
- libjsonrpccpp-dev, libjsonrpccpp-tools：jsonrpc 服务器相关
- libboost-regex-dev
- libcurl（curl）：vjudge\_hack需要
- flex, bison：语法分析器，在有 build/lex.yy.c, build/parser.tab.h, build/parser.tab.c 的情况下没有必要安装。

评测使用 vfk 提供的闭源沙盒，~~目前不开放下载~~ [已开源](https://github.com/roastduck/vfk_uoj_sandbox)。也可以考虑使用 libsandbox 替代。

在 Ubuntu Bionic 下安装相关依赖，可以执行 init-env_bionic.sh 以快速安装（此脚本默认不安装 flex 和 bison）

### 编译

编译前请自行调整路径等参数

执行

```sh
make
sudo make install
```

会编译并安装 yauj\_daemon 作为 rpc 服务器接收请求，并建立 resource 文件夹包含导入试题时编译脚本所需文件。

若 make 编译不成功，考虑 `make cleanall` 清除 build/ 下所有文件，安装 flex 和 bison，再重新执行 make。

## 运行

启动 daemon 用 rpc 请求进行评测时，需按如下步骤发出请求：（具体 rpc 过程名参见 [src/spec.json](src/spec.json)）

1. 同步题目数据
2. preserve 提交，用以避免同时评测过多提交。同时在此过程中将会同步提交的文件。
3. 执行提交

如需手动评测，流程如下：

1. 建立空文件夹
2. 将数据（包括 YAUJ 脚本）复制进来，编译 YAUJ 脚本
3. 将选手提交复制进来
4. 运行 yauj\_judge

注意，daemon 将进行文件操作，使用过程中应注意避免使用不同权限执行程序造成的文件权限混乱。应使用同一个普通权限用户执行。此外，daemon 编译 YAUJ 时使用的 makefile 应预先生成，其中可引用 resource 文件夹中的 makefile，但还应该编译此评测所需的 SPJ。

**更多帮助请参阅 [wiki](https://github.com/roastduck/YAUJ/wiki)**