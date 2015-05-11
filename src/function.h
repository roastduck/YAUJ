// the comment will be used by the editor in the future, do not change the format.

// the path memtioned below can be pull path or partial path.

#ifndef INCLUDED_FUNCTION_H
#define INCLUDED_FUNCTION_H

#include "interpreter.h"
#include "config.h"

extern iter _v_submission, _v_filemode, _v_result;

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
	iter range(const iter &lo, const iter &hi);
	iter len(const iter &x);
	/*
	 *	$ read : read file to str.
	 *	@1 : file : file name.
	 *   @return : content. empty string when file not exists.
	 */
	iter read(const iter &file);
	/*
	 *	$ copy : file copy
	 *	@1 : src : str
	 *	@2 : tar : str 
	 */
	void copy(const iter &src, const iter &tar);
	/*
	 *	$ split : split str into list, converting every single element of it to number if possible.
	 *	@1 : str : str.
	 *	@2 : pat : str. patterns. split by any character in it.
	 *   @return : the list.
	 */
	iter split(const iter &str, const iter &pat = _I_(new v_str(" \t\r\n")));
	
	/*
	 * $ log : output a log
	 * @1 content : str.
	 */
	void log(const iter &content);

	// execute
	/*
	 *	$ exec : execute a executable file in sandbox
	 *	@1 cases : null, int or list of int. testcases involved.
	 *	@2 file : str. file to be executed.
	 *	@3 in : str. stdin redirection.
	 *	@4 out : str. stdout redirection.
	 *	@5 err : str. stderr redirection.
	 *	@6 param : parameters.
	 *	@return : dict.
	 *		["status"] : str. "accepted" / "memory limit exceeded" / "output limit exceeded" / "time limit exceeded" / "runtime error" / "internal error" / "dangerous syscall". And you can add "partially accepted", "spj error". Function `compile` would generate "compile error".
	 *		["time"] : int. cpu time in ms.
	 *		["memory"] : int. maximum memory use in byte.
	 *		["exitcode"] : int. exit code.
	 */
	iter exec
		(
		 const iter &cases,
		 const iter &file,
		 const iter &in = _I_(new v_str("/dev/null")),
		 const iter &out = _I_(new v_str("/dev/null")),
		 const iter &err = _I_(new v_str("/dev/null")),
		 const iter &param = _I_(new v_str(""))
		);

	// compile
	/*
	 *	$ compile : compile with certian options.
	 *	@1 cases : null, int or list of int. testcases involved.
	 *	@2 language : str. "c++" / "c++11" / "c" / "pascal".
	 *	@3 source : str or list of str. path to source.
	 *	@4 target : str. path to binary target.
	 *	@5 O2 : bool. whether to compile with "O2"
	 *	@6 define : list of str. macros passed to compiler. ONLINE_JUDGE is always defined.
	 *	@return : dict
	 *		["exitcode"] : int. exit code
	 *		["message"] : str. compiling message.
	 */
	iter compile
		(
		 const iter &cases,
		 const iter &language,
		 const iter &source,
		 const iter &target,
		 const iter &O2 = _I_(new v_bool(DEFAULT_O2)),
		 const iter &define = _I_(new v_list(std::vector<iter>(1,_I_(new v_str(DEFAULT_DEFINE)))))
		);
	
	// diff
	/*
	 *	$ diff : compare two files. WARNING : THIS diff IS NOT BINARY SAFE AND NOT SUPPORTING NON-ASCII CHARATERS.
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
	 *	@return : bool. whether they differ.
	 */
	iter bin_diff(const iter &f1, const iter &f2);
	
}

#endif
