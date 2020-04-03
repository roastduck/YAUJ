src = /home/judge/resource/src

yauj_judge : decl_part init_part run_part $(src)/main.cpp $(src)/interpreter.h $(src)/function.h $(src)/config.h $(src)/uoj_env.h $(src)/interpreter.cpp $(src)/function.cpp $(src)/vjudge_hack.cpp
	g++ $(src)/main.cpp $(src)/interpreter.cpp $(src)/function.cpp $(src)/vjudge_hack.cpp -o yauj_judge -std=c++11 -ljsoncpp -lyauj -lcurl -lboost_regex -I$(src) -I. -g -Wall -DDEBUG

decl_part init_part run_part : init.src run.src
	yauj_parser

% : %.cpp FORCE
	g++ $< -o $@ -O2 -std=c++11

% : %.pas FORCE
	fpc $@ -O2

% : %.c FORCE
	gcc $< -o $@ -O2

FORCE:
