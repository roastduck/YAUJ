#include <cstring>
#include <vector>
#include <iostream>
#include "interpreter.h"
#include "function.h"

#define LINE_CAT(no) catch (const std::runtime_error &e) { throw std::runtime_error(std::string("line ")+no+" : "+e.what()); }

iter _v_filemode=_I_(new v_list(std::vector<iter>(5)));
iter _v_submission=_I_(new v_dict());
iter result=_I_(new v_list());

void usage()
{
	std::cout << "USAGE:" << std::endl;
	std::cout << "yauj_judger run [lang1 src1 [lang2 src2 ...]]" << std::endl;
	std::cout << "yauj_judger loadconf" << std::endl;
}

#include "decl_part"

void init()
{
	try
	{
#include "init_part"
	} catch (user_error) {}
}

void run()
{
	try
	{
#include "run_part"
	} catch (user_error) {{
}

int main(int argc, char **argv)
{
	if (argc==1) return usage(), 1;
	if (!strcmp(argv[1],"run"))
	{
		if (argc%2) return usage(), 1;
		for (int i=1;i<argc;i+=2) {
			//_v_submission->as_list().push_back(_I_(new v_dict()));
			//_v_submission->as_list().back()->as_dict()["language"]=_I_(new v_str(argv[i]));
			//_v_submission->as_list().back()->as_dict()["source"]=_I_(new v_str(argv[i+1]));
			_v_submission[_I_(new v_str(argv[i+1])]=_I_(new v_str(argv[i]));
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

