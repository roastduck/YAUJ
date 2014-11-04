#ifndef INCLUDED_FUNCTION_H
#define INCLUDED_FUNCTION_H

#include <cmath>
#include <ios>
#include <iostream>
#include "interpreter.h"

namespace func
{
	iter ceil(const iter &x);
	iter floor(const iter &x);
	iter round(const iter &x);
	
	void report(const iter &score);
	void report(const iter &score, const iter &message);
}

#endif
