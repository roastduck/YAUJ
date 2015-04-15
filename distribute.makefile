src = /home/judge/resource/src

yauj_judge : decl_part init_part run_part $(src)/main.cpp $(src)/interpreter.h $(src)/function.h $(src)/config.h $(src)/uoj_env.h
	g++ $(src)/main.cpp -o yauj_judge -std=c++11 -ljsoncpp -lyauj -I$(src) -I. -O2

decl_part init_part run_part : init.src run.src
	yauj_parser

% : %.cpp FORCE
	g++ $< -o $@ -O2

% : %.pas FORCE
	fpc $@ -O2

% : %.c FORCE
	gcc $< -o $@ -O2

FORCE:
