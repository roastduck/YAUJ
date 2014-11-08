#include "function.h"

int ts2ms(timespec &ts)
{
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
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

	// execute

	iter exec(const iter &prog, const iter &in, const iter &out, const iter &tl, const iter &ml, const iter &err, const iter &param)
	{
		std::map<std::string,iter> ret;
		std::string PROG, IN, OUT, ERR, PARAM;
		int TL, ML;
		PROG = prog->as_str();
		IN = in->as_str();
		OUT = out->as_str();
		TL = tl->as_int();
		ML = ml->as_int();
		ERR = err->as_str();
		PARAM = param->as_str();
		int in_no, out_no, err_no;
		in_no = open(IN.c_str(), O_RDONLY);
		out_no = open(OUT.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		err_no = open(ERR.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
		if (!~in_no)
			throw std::runtime_error(std::string("ERROR OPEN STDIN: ") + strerror(errno));
		if (!~out_no)
			throw std::runtime_error(std::string("ERROR OPEN STDOUT: ") + strerror(errno));
		if (!~err_no)
			throw std::runtime_error(std::string("ERROR OPEN STDERR: ") + strerror(errno));
		sandbox_t sbox;
		sbox.task.ifd = in_no;
		sbox.task.ofd = out_no;
		sbox.task.efd = err_no;
		sbox.task.quota[S_QUOTA_WALLCLOCK] = 60000;
		sbox.task.quota[S_QUOTA_CPU] = TL;
		sbox.task.quota[S_QUOTA_MEMORY] = ML;
		sbox.task.quota[S_QUOTA_DISK] = 104857600;
		if (!sandbox_check(&sbox)) throw std::runtime_error("SANDBOX CHECK FAILED");
		result_t res = *sandbox_execute(&sbox);
		ret["status"] = _I_(new v_str(
					1 ? "accept" :
					2 ? "restricted function" :
					3 ? "memory limit exceed" :
					4 ? "output limit exceed" :
					5 ? "time limit exceed" :
					6 ? "run time error" :
					7 ? "abnormal termination" :
					8 ? "internal error" :
					9 ? "bad policy" :
					    "unknown error"
					));
		ret["time"] = _I_(new v_int(ts2ms(sbox.stat.cpu_info.clock)));
		ret["memory"] = _I_(new v_int(sbox.stat.mem_info.vsize_peak));
		sandbox_fini(&sbox);
		close(in_no);
		close(out_no);
		close(err_no);
		return _I_(new v_dict(ret));
	}
}

