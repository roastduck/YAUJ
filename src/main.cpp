#include <cstring>
#include <vector>
#include <iostream>
#include "interpreter.h"
#include "function.h"

iter _v_filemode=_I_(new v_list(std::vector<iter>(5)));
iter _v_submission=_I_(new v_list());
iter result=_I_(new v_dict()); // recorded by report(). not a list because test cases are not always counted from 0

void usage()
{
	std::cout << "yauj_judger run [lang1 src1 [lang2 src2 ...]]" << std::endl;
	std::cout << "yauj_judger loadconf" << std::endl;
}

void init()
{
#include "init_part"	
}

void run()
{
#include "run_part"
}

int main(int argc, char **argv)
{
	if (argc==1) return usage(), 1;
	if (!strcmp(argv[1],"run"))
	{
		if (argc%2) return usage(), 1;
		for (int i=1;i<argc;i+=2) {
			_v_submission->as_list().push_back(_I_(new v_dict()));
			_v_submission->as_list().back()->as_dict()["language"]=_I_(new v_str(argv[i]));
			_v_submission->as_list().back()->as_dict()["source"]=_I_(new v_str(argv[i+1]));
		}
		init();
		run();
		std::cout << result->as_json().toStyledString() << endl;
	} else
	if (!strcmp(argv[1],"loadconf"))
	{
		init();
		std::cout << _v_filemode->as_json().toStyledString() << endl;
	} else
		return usage(), 1;
	return 0;
}

