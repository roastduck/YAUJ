#include "function.h"

namespace func
{
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
	
	void report(const iter &score)
	{
		std::cout << std::fixed << score->as_float() << std::endl;
	}
	
	void report(const iter &score, const iter &message)
	{
		std::cout << std::fixed << score->as_float() << ' ' << message->as_str() << std::endl;
	}
}
