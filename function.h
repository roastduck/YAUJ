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
	iter exec(
			const iter &comm,
			const iter &tl = _I_(new v_int(DEFAULT_CPU_LIMIT)),
			const iter &ml = _I_(new v_int(DEFAULT_MEMORY_LIMIT)),
			const iter &in = _I_(new v_str("/dev/null")),
			const iter &out = _I_(new v_str("/dev/null")),
			const iter &err = _I_(new v_str("/dev/null"))
		    ); 
}

#endif
