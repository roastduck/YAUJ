.PHONY : all install

LISTEN_PORT = 8388
DATA_PATH = \"/home/judge/newdata/yauj/data\"
RUN_PATH = \"/tmp/foj/yauj/run\"
SOURCE_PATH = \"/home/judge/newdata/yauj/code\"

all : build/daemon build/parser build/libyauj.so

build/libyauj.so : src/interpreter.cpp src/function.cpp
	g++ src/interpreter.cpp src/function.cpp -std=c++11 -fPIC -shared -lboost_regex -ljsoncpp -o build/libyauj.so

build/daemon : src/daemon.cpp build/abstractstubserver.h src/config_daemon.h
	g++ src/daemon.cpp -o build/daemon -pthread -ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server -Isrc -Ibuild -DLISTEN_PORT=$(LISTEN_PORT) -DDATA_PATH=$(DATA_PATH) -DRUN_PATH=$(RUN_PATH) -DSOURCE_PATH=$(SOURCE_PATH) -O2 -DDEBUG

build/abstractstubserver.h : src/spec.json
	jsonrpcstub src/spec.json --cpp-server=AbstractStubServer --cpp-server-file=build/abstractstubserver.h

build/parser : build/lex.yy.c build/parser.tab.c src/mystr.c build/parser.tab.h src/mystr.h
	gcc build/lex.yy.c build/parser.tab.c src/mystr.c -o build/parser -Isrc -Ibuild -O2

build/lex.yy.c : src/parser.l
	flex -o build/lex.yy.c src/parser.l

build/parser.tab.c build/parser.tab.h : src/parser.y
	bison -d src/parser.y
	mv parser.tab.c parser.tab.h build/


install : build/daemon distribute.makefile build/parser
	cp build/daemon /usr/bin/yauj_daemon
	cp distribute.makefile /home/judge/resource/makefile
	cp build/libyauj.so /usr/lib/
	cp -r src /home/judge/resource/
	cp build/parser /usr/bin/yauj_parser
