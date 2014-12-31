main : main.cpp src/interpreter.cpp src/function.cpp src/interpreter.h src/function.h src/config.h
	g++ main.cpp src/interpreter.cpp src/function.cpp -o main -std=c++11 -lsandbox -ljsoncpp -g -Wall

main.cpp : parser src.txt
	rm -f main.cpp
	./parser < src.txt > main.cpp

parser : lex.yy.c parser.tab.c src/mystr.c parser.tab.h src/mystr.h
	gcc lex.yy.c parser.tab.c src/mystr.c -o parser -g -DYYDEBUG
lex.yy.c : src/parser.l
	flex src/parser.l
parser.tab.c parser.tab.h : src/parser.y
	bison -d src/parser.y

