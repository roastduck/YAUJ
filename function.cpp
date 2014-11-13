#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <sandbox.h>
#include "function.h"

static int ts2ms(timespec &ts)
{
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void mode1filter(char *s)
{
	char *t;
	for (char *i=s; ; i++)
	{
		if (*i=='\n')
			for (char *j=i-1; j>=s && (*j==' ' || *j=='\r'); j--) *j=0;
		if (!*i)
		{
			for (char *j=i-1; j>=s && *j=='\n'; j--) *j=0;
			t=i;
			break;
		}
	}
	char *j=s;
	for (char *i=s; i<t; i++) if (*i)
		*(j++) = *i;
	*j=0;
}

static void mode2filter(char *s)
{
	char *j=s;
	for (char *i=s; *i; i++)
		if (isprint(*i) && !isblank(*i))
			*(j++) = *i;
		else if (j>s && *(j-1)!=' ')
			*(j++) = ' ';
	*j=0;
}

// 0=ok 1=fmt_err 2=eof
static int nextWord(char *&p1, char *&p2, char *&q1, char *&q2)
{
	for (;iscntrl(*p1) && iscntrl(*p2) || isblank(*p1) || isblank(*p2); p1++,p2++)
	{
		if (!*p1 && !*p2) return 2;
		if (*p1!=*p2) return 1;
	}
	q1 = p1, q2 = p2;
	while (isprint(*p1) && !isblank(*p1)) p1++;
	while (isprint(*p2) && !isblank(*p2)) p2++;
	return 0;
}

static std::pair<bool,double> getFloat(char *st, char *en)
{
	int dot(0);
	for (char *i=st; i<en; i++)
		if (*i=='.')
			dot++;
		else if (!isdigit(*i))
			return std::make_pair(false,0.0);
	double ret(0);
	sscanf(st,"%lf",&ret);
	return std::make_pair(true,ret);
}

namespace func
{
	// math

	iter ceil(const iter &x)
	{
		return v_base_ptr(new v_int(::ceil(x->as_float())));
	}
	
	iter floor(const iter &x)
	{
		return v_base_ptr(new v_int(::floor(x->as_float())));
	}
	
	iter round(const iter &x)
	{
		return v_base_ptr(new v_int(::round(x->as_float())));
	}
	
	// report

	void report(const iter &score)
	{
		std::cout << std::fixed << score->as_float() << std::endl;
	}
	
	void report(const iter &score, const iter &message)
	{
		std::cout << std::fixed << score->as_float() << ' ' << message->as_str() << std::endl;
	}

	void log(const iter &content)
	{
		try
		{
			std::clog << content->as_str() << std::endl;
		} catch (const std::runtime_error &e)
		{
			throw std::runtime_error("log : "+std::string(e.what()));
		}

	}
	
	// execute

	iter exec(const iter &comm, const iter &tl, const iter &ml, const iter &in, const iter &out, const iter &err)
	{
		try
		{
			std::map<std::string,iter> ret;
			std::string COMM, IN, OUT, ERR;
			int TL, ML;
			COMM = comm->as_str();
			IN = in->as_str();
			OUT = out->as_str();
			TL = tl->as_int();
			ML = ml->as_int();
			ERR = err->as_str();
			if (COMM[0]!='/') COMM=RUN_PATH+COMM;
			if (IN[0]!='/') IN=RUN_PATH+IN;
			if (OUT[0]!='/') OUT=RUN_PATH+OUT;
			if (ERR[0]!='/') ERR=RUN_PATH+ERR;
			int in_no, out_no, err_no;
			sandbox_t sbox;
			try
			{
				in_no = open(IN.c_str(), O_RDONLY);
				out_no = open(OUT.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
				err_no = open(ERR.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
				if (!~in_no)
					throw std::runtime_error(std::string("ERROR OPEN STDIN: ") + strerror(errno));
				if (!~out_no)
					throw std::runtime_error(std::string("ERROR OPEN STDOUT: ") + strerror(errno));
				if (!~err_no)
					throw std::runtime_error(std::string("ERROR OPEN STDERR: ") + strerror(errno));
				char _comm[COMMAND_BUFF_MAX], *_argv[2]={_comm,NULL};
				strcpy(_comm,COMM.c_str());
				if (sandbox_init(&sbox,(const char**)_argv)) throw std::runtime_error("SANDBOX INITIALIZATION FAILED");
				sbox.task.ifd = in_no;
				sbox.task.ofd = out_no;
				sbox.task.efd = err_no;
				sbox.task.quota[S_QUOTA_WALLCLOCK] = DEFAULT_WALLCLOCK_LIMIT;
				sbox.task.quota[S_QUOTA_CPU] = TL;
				sbox.task.quota[S_QUOTA_MEMORY] = ML;
				sbox.task.quota[S_QUOTA_DISK] = DEFAULT_OUTPUT_LIMIT;
				if (!sandbox_check(&sbox)) throw std::runtime_error("SANDBOX CHECK FAILED");
			} catch (const std::runtime_error &e)
			{
				if (~in_no) close(in_no);
				if (~out_no) close(out_no);
				if (~err_no) close(err_no);
				throw e;
			}
			if (!sandbox_execute(&sbox))
				throw std::runtime_error("SANDBOX EXECUTE FAILED");
			ret["status"] = _I_(new v_str(
						sbox.result == 1 ? "accept" :
						sbox.result == 2 ? "restricted function" :
						sbox.result == 3 ? "memory limit exceed" :
						sbox.result == 4 ? "output limit exceed" :
						sbox.result == 5 ? "time limit exceed" :
						sbox.result == 6 ? "run time error" :
						sbox.result == 7 ? "abnormal termination" :
						sbox.result == 8 ? "internal error" :
						sbox.result == 9 ? "bad policy" :
						"unknown error"
						));
			ret["time"] = _I_(new v_int(ts2ms(sbox.stat.cpu_info.clock)));
			ret["memory"] = _I_(new v_int(sbox.stat.mem_info.vsize_peak));
			ret["exitcode"] = _I_(new v_int(sbox.stat.exitcode));
			sandbox_fini(&sbox);
			close(in_no);
			close(out_no);
			close(err_no);
			return _I_(new v_dict(ret));
		} catch (const std::runtime_error &e)
		{
			throw std::runtime_error("exec : "+std::string(e.what()));
		}
	}
	
	// compile
	
	iter compile(const iter &language, const iter &source, const iter &target, const iter &O2, const iter &define)
	{
		try
		{
			std::string _LANG, _TAR;
			bool _O2;
			std::vector<std::string> _SRC, _DEF;
			_LANG = language->as_str();
			for (const iter &x: source->as_list())
			{
				_SRC.push_back(x->as_str());
				if (_SRC.back()[0]!='/') _SRC.back() = RUN_PATH + _SRC.back();
			}
			_TAR = target->as_str();
			if (_TAR[0]!='/') _TAR = RUN_PATH + _TAR;
			_O2 = O2->as_bool();
			for (const iter &x: define->as_list()) _DEF.push_back(x->as_str());
			std::string cmd;
			if (_LANG == "c++")
			{
				cmd = "g++ ";
				for (const std::string &x: _SRC) cmd += x + " ";
				cmd += " -o " + _TAR;
				if (_O2) cmd += " -O2 ";
				for (const std::string &x: _DEF) cmd += " -D" + x;
			} else
				if (_LANG == "c++11")
				{
					cmd = "g++ -std=c++11 ";
					for (const std::string &x: _SRC) cmd += x + " ";
					cmd += " -o " + _TAR;
					if (_O2) cmd += " -O2 ";
					for (const std::string &x: _DEF) cmd += " -D" + x;
				} else
					if (_LANG == "c")
					{
						cmd = "gcc ";
						for (const std::string &x: _SRC) cmd += x + " ";
						cmd += " -o " + _TAR;
						if (_O2) cmd += " -O2 ";
						for (const std::string &x: _DEF) cmd += " -D" + x;
					} else
						if (_LANG == "pascal")
						{
							cmd = "fpc ";
							for (const std::string &x: _SRC) cmd += x + " ";
							cmd += " -o" + _TAR;
							if (_O2) cmd += " -O2 ";
							for (const std::string &x: _DEF) cmd += " -d" + x;
						} else
							throw std::runtime_error("unknown language");
			cmd += " 2>&1 ";
			std::map<std::string,iter> ret;
			FILE *stat = popen(cmd.c_str(),"r");
			static char buff[PIPE_READ_BUFF_MAX+1]; // null-termination
			fread(buff,1,PIPE_READ_BUFF_MAX,stat);
			if (!feof(stat))
				std::cerr << "[WARNING] compile : PIPE_READ_BUFF_MAX exceeded" << std::endl;
			ret["exitcode"]=_I_(new v_int(pclose(stat)));
			ret["message"]=_I_(new v_str(buff));
			return _I_(new v_dict(ret));
		} catch (const std::runtime_error &e)
		{
			throw std::runtime_error("compile : "+std::string(e.what()));
		}
	}
	
	// diff
	
	iter diff(const iter &f1, const iter &f2, const iter &w_mode)
	{
		try
		{
			std::string F1, F2;
			int W_MODE;
			F1 = f1->as_str();
			F2 = f2->as_str();
			W_MODE = w_mode->as_int();
			if (W_MODE < 0 || W_MODE > 2)
				throw std::runtime_error("unknown w_mode");
			if (F1[0]!='/') F1 = RUN_PATH + F1;
			if (F2[0]!='/') F2 = RUN_PATH + F2;
			FILE *f1_ptr, *f2_ptr;
			f1_ptr = fopen(F1.c_str(),"r");
			f2_ptr = fopen(F2.c_str(),"r");
			if (!f1_ptr || !f2_ptr)
			{
				if (f1_ptr) fclose(f1_ptr);
				if (f2_ptr) fclose(f2_ptr);
				throw std::runtime_error("file not exist");
			}
			static char buff1[DIFF_FILE_BUFF_MAX+1], buff2[DIFF_FILE_BUFF_MAX+1]; // null-termination
			fread(buff1,1,DIFF_FILE_BUFF_MAX,f1_ptr);
			fread(buff2,1,DIFF_FILE_BUFF_MAX,f2_ptr);
			fclose(f1_ptr);
			fclose(f2_ptr);
			if ((ferror(f1_ptr) || !feof(f1_ptr)) && (ferror(f2_ptr) || !feof(f2_ptr)))
				throw std::runtime_error("DIFF_FILE_BUFF_MAX exceeded");
			if (W_MODE == 1)
				mode1filter(buff1), mode1filter(buff2);
			if (W_MODE == 2)
				mode2filter(buff1), mode2filter(buff2);
			char *p1=buff1, *p2=buff2;
			bool r_verdict(false);
			double r_max_abs_diff(0), r_max_rel_diff(0);
			std::pair<std::string,std::string> r_first_diff;
			while (true)
			{
				char *q1, *q2;
				int stat=nextWord(p1,p2,q1,q2);
				if (stat==2) break;
				if (stat==1)
				{
					r_verdict = true;
					r_max_abs_diff = r_max_rel_diff = INFINITY;
					break;
				}
				if (p1-q1!=p2-q2 || memcmp(q1,q2,p1-q1))
				{
					r_verdict = true;
					r_max_abs_diff = r_max_rel_diff = INFINITY;
					r_first_diff=std::make_pair(std::string(q1,p1),std::string(q2,p2));
				}
				std::pair<bool,double> res1, res2;
				res1 = getFloat(q1,p1);
				res2 = getFloat(q2,p2);
				if (res1.first != res2.first)
				{
					r_verdict = true;
					r_max_abs_diff = r_max_rel_diff = INFINITY;
					break;
				}
				if (res1.first)
				{
					r_max_abs_diff = std::max(std::abs(res1.second-res2.second),r_max_abs_diff);
					r_max_rel_diff = std::max(std::abs(res1.second-res2.second)/res2.second,r_max_rel_diff);
				}
			}
			iter ret = _I_(new v_dict());
			ret->as_dict()["verdict"] = _I_(new v_bool(r_verdict));
			ret->as_dict()["max_abs_diff"] = _I_(new v_float(r_max_abs_diff));
			ret->as_dict()["max_rel_diff"] = _I_(new v_float(r_max_rel_diff));
			ret->as_dict()["first_diff"] = _I_(new v_dict());
			ret->as_dict()["first_diff"]->as_dict()["f1"] = _I_(new v_str(r_first_diff.first));
			ret->as_dict()["first_diff"]->as_dict()["f2"] = _I_(new v_str(r_first_diff.second));
			return ret;
		} catch (const std::runtime_error &e)
		{
			throw std::runtime_error("diff : "+std::string(e.what()));
		}
	}
	
	iter bin_diff(const iter &f1, const iter &f2)
	{
		std::string F1, F2;
		F1 = f1->as_str();
		F2 = f2->as_str();
		if (F1[0]!='/') F1 = RUN_PATH + F1;
		if (F2[0]!='/') F2 = RUN_PATH + F2;
		return _I_(new v_bool(system(("diff "+F1+" "+F2+" >/dev/null 2>&1").c_str())));
	}
	
}

