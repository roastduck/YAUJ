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

	void log(const iter &content)
	{
		std::cerr << content->as_str() << std::endl;
	}
	
	// execute

	iter exec(const iter &comm, const iter &tl, const iter &ml, const iter &in, const iter &out, const iter &err)
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
		result_t res = *sandbox_execute(&sbox);
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
		sandbox_fini(&sbox);
		close(in_no);
		close(out_no);
		close(err_no);
		return _I_(new v_dict(ret));
	}
}

