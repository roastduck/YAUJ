src = /home/judge/resource/src

yauj_judge : decl_part init_part run_part $(src)/main.cpp $(src)/interpreter.cpp $(src)/function.cpp $(src)/interpreter.h $(src)/function.h $(src)/config.h $(src)/uoj_env.h
	g++ $(src)/interpreter.cpp $(src)/function.cpp $(src)/main.cpp -o yauj_judge -std=c++11 -ljsoncpp -lboost_regex -I$(src) -I. -O2

decl_part init_part run_part : init.src run.src
	rm -f build/decl_part build/init_part build/run_part
	yauj_parser

% : %.cpp FORCE
	g++ $< -o $@ -O2

% : %.pas FORCE
	fpc $@ -O2

% : %.c FORCE
	gcc $< -o $@ -O2

FORCE :
