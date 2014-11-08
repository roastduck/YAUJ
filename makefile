main : main.cpp interpreter.cpp function.cpp interpreter.h function.h
	g++ main.cpp interpreter.cpp function.cpp -o main -std=c++11 -g -lsandbox

main.cpp : parser src.txt
	./parser < src.txt > main.cpp

parser : lex.yy.c parser.tab.c mystr.c parser.tab.h mystr.h
	gcc lex.yy.c parser.tab.c mystr.c -o parser -g
lex.yy.c : parser.l
	flex parser.l
parser.tab.c parser.tab.h : parser.y
	bison -d parser.y

