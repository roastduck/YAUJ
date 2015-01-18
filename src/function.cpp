#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <regex>
#include <string>
#include <iostream>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <boost/regex.hpp>
#include "function.h"
#include "uoj_env.h"

#define FUNC_END(name) catch (const std::runtime_error &e) { throw std::runtime_error(#name" : "+std::string(e.what())); }

static void mode1filter(char *s)
{
	/*for (char *i=s; ; i++)
	{
		if (*i=='\n')
			for (char *j=i-1; j>=s && (*j==' ' || *j=='\r'); j--) *j=0;
		if (!*i)
		{
			for (char *j=i-1; j>=s && *j=='\n'; j--) *j=0;
			t=i;
			break;
		}
	}*/
	char *t = s+strlen(s), *i = t-1;
	while (i>=s)
		if (*i=='\n')
			for (i--; i>=s && (*i==' ' || *i == '\r'); i--) *i = 0;
		else
			i--;
	char *j=s;
	for (char *i=s; i<t; i++) if (*i)
		*(j++) = *i;
	*j=0;
	for (j--; j>=s && *j=='\n'; j--) *j=0;
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

static double getFloat(char *st, char *en)
{
	int dot(0);
	for (char *i=st; i<en; i++)
		if (!isdigit(*i))
			if (*i!='.' || ++dot>1)
				return nan("");
	return atof(st);
}

static std::vector<iter> toVector(const iter &src)
{
	//if (!src.ptr) return std::vector<iter>();
	if (src->to() & LIST) return src->as_list();
	return std::vector<iter>(1,src);
}

namespace func
{
	// math

	iter ceil(const iter &x)
	{
		try { return v_base_ptr(new v_int(::ceil(x->as_float()))); }
		FUNC_END(ceil);
	}
	
	iter floor(const iter &x)
	{
		try { return v_base_ptr(new v_int(::floor(x->as_float()))); }
		FUNC_END(floor);
	}
	
	iter round(const iter &x)
	{
		try { return v_base_ptr(new v_int(::round(x->as_float()))); }
		FUNC_END(round);
	}

	// algorithm
	
	iter min(const iter &x, const iter &y)
	{
		try { return std::min(x,y); }
		FUNC_END(min);
	}
	
	iter max(const iter &x, const iter &y)
	{
		try { return std::max(x,y); }
		FUNC_END(max);
	}

	void swap(iter &x, iter &y)
	{
		std::swap(x,y);
	}
	
	void sort(const iter &x)
	{
		try
		{
			if (x->to() & LIST)
				sort(x->as_list().begin(),x->as_list().end());
			else
				throw std::runtime_error("cannot sort this type");
		}
		FUNC_END(sort);
	}
	
	// misc
	iter range(const iter &lo, const iter &hi)
	{
		iter ret=_I_(new v_list());
		for (int i=lo->as_int();i<hi->as_int();i++) ret.add(_I_(new v_int(i)));
		return ret;
	}

	iter len(const iter &x)
	{
		try
		{
			if (x->to() & LIST)
				return _I_(new v_int(x->as_list().size()));
			if (x->to() & DICT)
				return _I_(new v_int(x->as_dict().size()));
			if (x->to() & STR)
				return _I_(new v_int(x->as_str().size()));
			throw std::runtime_error("this type has no length");
		}
		FUNC_END(len);
	}

	iter read(const iter &file)
	{
		try
		{
			std::string F = file->as_str();
			//if (F[0]!='/') F = RUN_PATH + F;
			FILE *p = fopen(F.c_str(),"r");
			if (!p) return _I_(new v_str(""));
			static char buff[FUNC_READ_BUFF_MAX+1];
			buff[fread(buff,1,FUNC_READ_BUFF_MAX,p)]=0;
			if (!feof(p)) throw fclose(p), std::runtime_error("FUNC_READ_BUFF_MAX exceeded");
			fclose(p);
			return _I_(new v_str(buff));
		}
		FUNC_END(read);
	}
	
	void copy(const iter &src, const iter &tar)
	{
		try
		{
			system(("cp "+src->as_str()+" "+tar->as_str()).c_str());
		}
		FUNC_END(copy);
	}
	
	iter split(const iter &str, const iter &pat)
	{
		try
		{
			static boost::regex regi("[0-9]+"), regf("[0-9]*\\.[0-9]+");
			std::string s = str->as_str(), p = pat->as_str();
			std::vector<iter> ret;
			size_t pos(0);
			while (true)
			{
				size_t next;
				next = s.find_first_of(p,pos);
				if (next==std::string::npos)
				{
					if (pos<s.length())
					{
						std::string t = s.substr(pos);
						if (boost::regex_match(t,regi))
							ret.push_back(_I_(new v_int(stoi(t))));
						else if (boost::regex_match(t,regf))
							ret.push_back(_I_(new v_float(stod(t))));
						else
							ret.push_back(_I_(new v_str(t)));
					}
					break;
				}
				if (next>pos)
				{
					std::string t = s.substr(pos,next-pos);
					if (boost::regex_match(t,regi))
						ret.push_back(_I_(new v_int(stoi(t))));
					else if (boost::regex_match(t,regf))
						ret.push_back(_I_(new v_float(stod(t))));
					else
						ret.push_back(_I_(new v_str(t)));
				}
				pos = next+1;
			}
			return _I_(new v_list(ret));
		}
		FUNC_END(split);
	}
	
	// report

	/*void report(const iter &score, const iter &verdict, const iter &time, const iter &memory, const iter &message)
	{
		try
		{
			std::cout << score->as_float() << ' ' << verdict->as_str() << ' ' << time->as_int() << ' ' << memory->as_int() << ' ' << message->as_str() << std::endl;
		}
		FUNC_END(report);
	}*/

	void log(const iter &content)
	{
		try { std::clog << content->as_json() << std::endl; }
		FUNC_END(log);
	}
	
	// execute
	
	iter exec(const iter &cases, const iter &file, const iter &in, const iter &out, const iter &err, const iter &param)
	{
		try
		{
			std::map<std::string,iter> ret;
			std::string COMM, IN, OUT, ERR, TL, ML, SL, RB, WB, OTHER;
			iter src=_v_filemode[_I_(new v_int(4))][file][_I_(new v_str("source"))];
			COMM = file->as_str()+" "+param->as_str();
			IN = in->as_str();
			OUT = out->as_str();
			ERR = err->as_str();
			if (_v_filemode[_I_(new v_int(4))][file][_I_(new v_str("time"))][toVector(cases)[0]])
				TL = _v_filemode[_I_(new v_int(4))][file][_I_(new v_str("time"))][toVector(cases)[0]]->as_str();
			else
				TL = "5000"; // spj limit
			if (_v_filemode[_I_(new v_int(4))][file][_I_(new v_str("memory"))][toVector(cases)[0]])
				ML = _v_filemode[_I_(new v_int(4))][file][_I_(new v_str("memory"))][toVector(cases)[0]]->as_str();
			else
				ML = "524288";
			if (_v_filemode[_I_(new v_int(4))][file][_I_(new v_str("stack"))][toVector(cases)[0]])
				SL = _v_filemode[_I_(new v_int(4))][file][_I_(new v_str("stack"))][toVector(cases)[0]]->as_str();
			else
				SL = "8192";
			if (_v_filemode[_I_(new v_int(0))])
				for (auto &x : _v_filemode[_I_(new v_int(0))]->as_dict())
					for (const auto &y : toVector(x.second[_I_(new v_str("by"))]))
						if (y == src)
						{
							RB += " --add-readable="+x.first;
							break;
						}
			if (_v_filemode[_I_(new v_int(1))])
				for (auto &x : _v_filemode[_I_(new v_int(1))]->as_dict())
					for (const auto &y : toVector(x.second[_I_(new v_str("by"))]))
						if (y == src)
						{
							WB += " --add-writable="+x.first;
							system(("touch "+x.first).c_str());
							break;
						}
			if (!src || !_v_filemode[_I_(new v_int(2))][src])
				OTHER = " --unsafe ";
			//COMM="./"+COMM;
			while (COMM.back()==' ') COMM.pop_back();
			int u_stat, u_time, u_mem, u_ret;
			std::string sbcmd = "uoj_run -T "+TL+" -M "+ML+" -S "+SL+" -i "+IN+" -o "+OUT+" -e "+ERR+RB+WB+OTHER+" "+COMM;
#ifdef DEBUG
			std::clog << sbcmd << std::endl;
#endif
			FILE *sandbox = popen(sbcmd.c_str(),"r");
			fscanf(sandbox,"%d%d%d%d",&u_stat,&u_time,&u_mem,&u_ret);
#ifdef DEBUG
			std::clog << u_stat << ' ' << u_time << ' ' << u_mem << ' ' << u_ret << std::endl;
#endif
			pclose(sandbox);
			ret["status"] = _I_(new v_str(
						u_stat == RS_AC ? "accepted" :
						u_stat == RS_MLE ? "memory limit exceed" :
						u_stat == RS_OLE ? "output limit exceed" :
						u_stat == RS_TLE ? "time limit exceed" :
						u_stat == RS_RE ? "runtime error" :
						u_stat == RS_JGF ? "internal error" :
						u_stat == RS_DGS ? "dangerous syscall" :
						"internal error"
						));
			ret["time"] = _I_(new v_int(u_time));
			ret["memory"] = _I_(new v_int(u_mem));
			ret["exitcode"] = _I_(new v_int(u_ret));
			if (src && _v_filemode[_I_(new v_int(2))][src])
				for (const auto &x : toVector(cases))
				{
					_v_result[x][_I_(new v_str("time"))][_v_filemode[_I_(new v_int(4))][file][_I_(new v_str("source"))]]=ret["time"];
					_v_result[x][_I_(new v_str("memory"))][_v_filemode[_I_(new v_int(4))][file][_I_(new v_str("source"))]]=ret["memory"];
				}
			if (u_stat)
			{
				for (const auto &x : toVector(cases))
				{
					if (src && _v_filemode[_I_(new v_int(2))][src])
						_v_result[x][_I_(new v_str("status"))]=ret["status"];
					else
						_v_result[x][_I_(new v_str("status"))]=_I_(new v_str("spj error"));
					_v_result[x][_I_(new v_str("score"))]=_I_(new v_int(0));
				}
				throw user_error();
			}
			if (u_ret && src && _v_filemode[_I_(new v_int(2))][src])
			{
				for (const auto &x : toVector(cases))
				{
					_v_result[x][_I_(new v_str("status"))]=_I_(new v_str("runtime error"));
					_v_result[x][_I_(new v_str("score"))]=_I_(new v_int(0));
				}
				throw user_error();
			}
			return _I_(new v_dict(ret));
		}
		FUNC_END(exec);
	}
	
	// compile
	
	iter compile(const iter &cases, const iter &language, const iter &source, const iter &target, const iter &O2, const iter &define)
	{
		try
		{
			std::string _LANG, _TAR;
			bool _O2;
			std::vector<std::string> _SRC, _DEF;
			_LANG = language->as_str();
			for (const iter &x : toVector(source)) _SRC.push_back(x->as_str());
			_TAR = target->as_str();
			_O2 = O2->as_bool();
			for (const iter &x: toVector(define)) _DEF.push_back(x->as_str());
			_DEF.push_back("ONLINE_JUDGE");
			std::string cmd;
			if (_LANG == "c++")
			{
				cmd = "g++ -x c++ ";
				for (const std::string &x: _SRC) cmd += x + " ";
				cmd += " -o " + _TAR;
				if (_O2) cmd += " -O2 ";
				for (const std::string &x: _DEF) cmd += " -D" + x;
			} else
			if (_LANG == "c++11")
			{
				cmd = "g++ -x c++ -std=c++11 ";
				for (const std::string &x: _SRC) cmd += x + " ";
				cmd += " -o " + _TAR;
				if (_O2) cmd += " -O2 ";
				for (const std::string &x: _DEF) cmd += " -D" + x;
			} else
			if (_LANG == "c")
			{
				cmd = "gcc -x c ";
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
				throw std::runtime_error("unknown language : "+_LANG);
			cmd += " 2>&1 ";
			std::map<std::string,iter> ret;
			FILE *stat = popen(cmd.c_str(),"r");
			static char buff[PIPE_READ_BUFF_MAX+1]; // null-termination
			buff[fread(buff,1,PIPE_READ_BUFF_MAX,stat)]=0;
			if (!feof(stat))
				std::cerr << "[WARNING] compile : PIPE_READ_BUFF_MAX exceeded" << std::endl;
			ret["exitcode"]=_I_(new v_int(pclose(stat)));
			ret["message"]=_I_(new v_str(buff));
			if (ret["exitcode"])
			{
				for (const iter &x : toVector(cases))
				{
					_v_result[x][_I_(new v_str("status"))]=_I_(new v_str("compile error"));
					_v_result[x][_I_(new v_str("score"))]=_I_(new v_int(0));
					_v_result[x][_I_(new v_str("message"))]=ret["message"];
				}
				throw user_error();
			}
			return _I_(new v_dict(ret));
		}
		FUNC_END(compile);
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
			//if (F1[0]!='/') F1 = RUN_PATH + F1;
			//if (F2[0]!='/') F2 = RUN_PATH + F2;
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
			buff1[fread(buff1,1,DIFF_FILE_BUFF_MAX,f1_ptr)]=0;
			buff2[fread(buff2,1,DIFF_FILE_BUFF_MAX,f2_ptr)]=0;
			if ((ferror(f1_ptr) || !feof(f1_ptr)) && (ferror(f2_ptr) || !feof(f2_ptr)))
				throw fclose(f1_ptr), fclose(f2_ptr), std::runtime_error("DIFF_FILE_BUFF_MAX exceeded");
			fclose(f1_ptr);
			fclose(f2_ptr);
#ifdef DEBUG
			std::clog << "diff : have read files" << std::endl;
#endif
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
					if (r_first_diff.first.empty() && r_first_diff.second.empty())
						r_first_diff=std::make_pair(std::string(q1,p1),std::string(q2,p2));
				}
				double res1 = getFloat(q1,p1), res2 = getFloat(q2,p2);
				if (isnan(res1) || isnan(res2))
					r_max_abs_diff = r_max_rel_diff = INFINITY;
				else
				{
					r_max_abs_diff = std::max(std::abs(res1-res2),r_max_abs_diff);
					r_max_rel_diff = std::max(std::abs(res1-res2)/res2,r_max_rel_diff);
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
		}
		FUNC_END(diff);
	}
	
	iter bin_diff(const iter &f1, const iter &f2)
	{
		try
		{
			std::string F1, F2;
			F1 = f1->as_str();
			F2 = f2->as_str();
			//if (F1[0]!='/') F1 = RUN_PATH + F1;
			//if (F2[0]!='/') F2 = RUN_PATH + F2;
			return _I_(new v_bool(system(("diff "+F1+" "+F2+" >/dev/null 2>&1").c_str())));
		}
		FUNC_END(bin_diff);
	}

}

#undef FUNC_END

