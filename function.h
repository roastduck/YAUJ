#ifndef INCLUDED_FUNCTION_H
#define INCLUDED_FUNCTION_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <sandbox.h>
#include "interpreter.h"
#include "config.h"

namespace func
{
	// math
	iter ceil(const iter &x);
	iter floor(const iter &x);
	iter round(const iter &x);

	// report
	void report(const iter &score);
	void report(const iter &score, const iter &message);
	void log(const iter &content);

	// execute
	/*
	 *	exec : execute a command in sandbox.
	 *	@1 comm : str. command to be executed.
	 *	@2 tl : int. time limit in ms.
	 *	@3 ml : int. memory limit in bytes.
	 *	@4 in : str. stdin redirection path.
	 *	@5 out : str. stdout redirection path.
	 *	@6 err : str. stderr redirection path.
	 *	return : list.
	 *		["status"] : str. "accept" / "restricted function" / "memory limit exceed" / "output limit exceed" / "time limit exceed" / "run time error" / "abnormal termination" / "internal error" / "bad policy" / "unknown error".
	 *		["time"] : int. cpu time in ms.
	 *		["memory"] : int. maximum memory use in byte.
	 *		["exitcode"] : int. exit code.
	 */
	iter exec
		(
		 const iter &comm,
		 const iter &tl = _I_(new v_int(DEFAULT_CPU_LIMIT)),
		 const iter &ml = _I_(new v_int(DEFAULT_MEMORY_LIMIT)),
		 const iter &in = _I_(new v_str("/dev/null")),
		 const iter &out = _I_(new v_str("/dev/null")),
		 const iter &err = _I_(new v_str("/dev/null"))
		);

	// compile
	/*
	 *	compile : compile with certian options.
	 *	@1 language : str. "c++" / "c++11" / "c" / "pascal".
	 *	@2 source : list of str. path to source.
	 *	@3 target : str. path to binary target.
	 *	@4 O2 : bool. weather to compile with "O2"
	 *	@5 define : list of str. macros passed to compiler.
	 *	return : list
	 *		["exitcode"] : int. exit code
	 *		["message"] : str. compiling message.
	 */
	iter compile
		(
		 const iter &language,
		 const iter &source,
		 const iter &target,
		 const iter &O2 = _I_(new v_bool(DEFAULT_O2)),
		 const iter &define = _I_(new v_list(std::vector<iter>(1,_I_(new v_str(DEFAULT_DEFINE)))))
		);
}

#endif
