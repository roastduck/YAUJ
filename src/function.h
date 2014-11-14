// the comment will be used by the editor in the future, do not change the format.

// the path memtioned below can be pull path or partial path.

#ifndef INCLUDED_FUNCTION_H
#define INCLUDED_FUNCTION_H

#include "interpreter.h"
#include "config.h"

namespace func
{
	// math
	iter ceil(const iter &x);
	iter floor(const iter &x);
	iter round(const iter &x);

	// algorithm
	iter min(const iter &x, const iter &y);
	iter max(const iter &x, const iter &y);
	void swap(iter &x, iter &y);
	void sort(const iter &x);

	// misc
	iter len(const iter &x);
	iter read(const iter &file);
	/*
	 *	$ split : split str into list
	 *	@1 : str : str.
	 *	@2 : pat : patterns. split by any character in it.
	 */
	iter split(const iter &str, const iter &pat = _I_(new v_str(" \t\r\n")));
	
	// report
	/*
	 * $ report :
	 * @1 score : float.
	 * @2 verdict : str.
	 * @3 time : int. in ms.
	 * @4 memory : int. int bytes.
	 * @5 message : str. extra message.
	 */
	void report(const iter &score, const iter &verdict, const iter &time, const iter &memory, const iter &message = _I_(new v_str("")));
	
	/*
	 * $ log : output a log
	 * @1 content : str.
	 */
	void log(const iter &content);

	// execute
	/*
	 *	$ exec : execute a command in sandbox.
	 *	@1 comm : str. command to be executed.
	 *	@2 tl : int. time limit in ms.
	 *	@3 ml : int. memory limit in bytes.
	 *	@4 in : str. stdin redirection path.
	 *	@5 out : str. stdout redirection path.
	 *	@6 err : str. stderr redirection path.
	 *	@return : dict.
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
	 *	$ compile : compile with certian options.
	 *	@1 language : str. "c++" / "c++11" / "c" / "pascal".
	 *	@2 source : list of str. path to source.
	 *	@3 target : str. path to binary target.
	 *	@4 O2 : bool. weather to compile with "O2"
	 *	@5 define : list of str. macros passed to compiler.
	 *	@return : dict
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
	
	// diff
	/*
	 *	$ diff : compare two files. WARNING : THIS diff IS NOT BINARY SAFE.
	 *	@1 f1 : str. path to file one.
	 *	@2 f2 : str. path to file two.
	 *	@3 w_mode : int.
	 *		0 : char by char (NOTE: newline character are different between Windows and Linux)/
	 *		1 : ignore spaces at the end of lines and '\n' or '\r' at the end of files /
	 *		2 : word by word.
	 *	@return : dict
	 *		["verdict"] : bool. true if the files differ.
	 *		["max_abs_diff"] : float. maximum absolute difference of the NUMBERS in the file.
	 *		["max_rel_diff"] : float. maximum relative difference of the NUMBERS in the file.
	 *			NOTE : use file two as denomiator.
	 *			NODE : if the files are too different, ["max_abs_diff"] and ["max_rel_diff"] will be set to infinity.
	 *			WARNING : THE DIFFERENCES ARE CALCULATED WITH 64-bit FLOATING-POINT NUMBER.
	 *		["first_diff"] : dict. first difference
	 *			["f1"] : str. word in file one.
	 *			["f2"] : str. word in file two.
	 */
	iter diff(const iter &f1, const iter &f2, const iter &w_mode = _I_(new v_int(DEFAULT_W_MODE)));
	
	/*
	 *	$ bin_diff : binary diff. this diff is binary safe.
	 *	@1 f1 : str. path to file one.
	 *	@2 f2 : str. path to file two.
	 *	@return : bool. weather they differ.
	 */
	iter bin_diff(const iter &f1, const iter &f2);
	
}

#endif
