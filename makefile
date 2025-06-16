.PHONY : all install clean cleanall

config_file = /etc/yauj/daemon.json

all : build/abstractstubserver.h build/libyauj.so build/daemon build/parser

build/libyauj.so : src/interpreter.cpp src/interpreter.h src/function.cpp src/function.h src/config.h src/vjudge_hack.cpp
	g++ src/interpreter.cpp src/function.cpp src/vjudge_hack.cpp -std=c++14 -fPIC -shared -lboost_regex -ljsoncpp -lcurl -o build/libyauj.so -O2

build/daemon : src/daemon.cpp build/abstractstubserver.h src/config_daemon.h
	g++ src/daemon.cpp -o build/daemon -pthread -ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server -Isrc -Ibuild -O2

build/abstractstubserver.h : src/spec.json
	jsonrpcstub src/spec.json --cpp-server=AbstractStubServer --cpp-server-file=build/abstractstubserver.h

build/parser : build/lex.yy.c build/parser.tab.c src/mystr.c build/parser.tab.h src/mystr.h
	gcc build/lex.yy.c build/parser.tab.c src/mystr.c -o build/parser -Isrc -Ibuild -O2

build/lex.yy.c : | src/parser.l
	flex -o build/lex.yy.c src/parser.l

build/parser.tab.c build/parser.tab.h : | src/parser.y
	bison -d src/parser.y
	mv parser.tab.c parser.tab.h build/


install : build/daemon build/parser build/libyauj.so distribute.makefile
	cp build/daemon /usr/bin/yauj_daemon
	mkdir -p /home/judge/resource
	-chmod 777 /home/judge
	-chmod 777 /home/judge/resource
	cp distribute.makefile /home/judge/resource/makefile
	cp build/libyauj.so /usr/lib/
	cp -r src /home/judge/resource/
	cp build/parser /usr/bin/yauj_parser
	mkdir -p /etc/yauj
ifeq ($(config_file), $(wildcard $(config_file)))
	@echo "daemon.json already exist"
else
	cp daemon.json /etc/yauj/
endif

cleanall :
	rm -f build/*

clean :
	rm -f build/libyauj.so build/daemon build/abstractstubserver.h build/parser
