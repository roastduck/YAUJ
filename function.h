#ifndef INCLUDED_FUNCTION_H
#define INCLUDED_FUNCTION_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <sandbox.h>
#include "interpreter.h"

namespace func
{
	// math
	iter ceil(const iter &x);
	iter floor(const iter &x);
	iter round(const iter &x);

	// report
	void report(const iter &score);
	void report(const iter &score, const iter &message);

	// execute
	iter exec(
			const iter &prog,
			const iter &in = _I_(new v_str("/dev/null")),
			const iter &out = _I_(new v_str("/dev/null")),
			const iter &tl = _I_(new v_int(1000)),
			const iter &ml = _I_(new v_int(262144)),
			const iter &err = _I_(new v_str("/dev/null")),
			const iter &param = _I_(new v_str("/dev/null"))
		    ); 
}

#endif
