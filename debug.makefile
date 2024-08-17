#debug mode

.PHONY : all

all : daemon yauj_judge

daemon : build/daemon
	cp build/daemon .

yauj_judge : build/yauj_judge
	cp build/yauj_judge .

build/daemon : src/daemon.cpp build/abstractstubserver.h src/config_daemon.h
	g++ src/daemon.cpp -o build/daemon -ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server -Isrc -Ibuild -g -DDEBUG

build/abstractstubserver.h : src/spec.json
	jsonrpcstub src/spec.json --cpp-server=AbstractStubServer --cpp-server-file=build/abstractstubserver.h

build/yauj_judge : build/decl_part build/init_part build/run_part src/main.cpp src/interpreter.cpp src/function.cpp src/interpreter.h src/function.h src/config.h src/uoj_env.h src/vjudge_hack.cpp
	g++ src/interpreter.cpp src/function.cpp src/vjudge_hack.cpp src/main.cpp -o build/yauj_judge -std=c++14 -ljsoncpp -lboost_regex -lcurl -Isrc -Ibuild -g -Wall -DDEBUG

build/decl_part build/init_part build/run_part : build/parser init.src run.src
	rm -f build/decl_part build/init_part build/run_part
	build/parser
	mv decl_part init_part run_part build/

build/parser : build/lex.yy.c build/parser.tab.c src/mystr.c build/parser.tab.h src/mystr.h
	gcc build/lex.yy.c build/parser.tab.c src/mystr.c -o build/parser -Isrc -Ibuild -g -DYYDEBUG

build/lex.yy.c : src/parser.l
	flex -o build/lex.yy.c src/parser.l

build/parser.tab.c build/parser.tab.h : src/parser.y
	bison -d src/parser.y
	mv parser.tab.c parser.tab.h build/
