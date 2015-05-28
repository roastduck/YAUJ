#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <unistd.h>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include "interpreter.h"

#ifndef MAX_FETCH_TRIAL
	#define MAX_FETCH_TRIAL 720 // 3 minuates
#endif

#ifndef VJUDGE_CONFIG_FILE
	#define VJUDGE_CONFIG_FILE "/etc/yauj/vjudge_hack.json"
#endif

extern const char status_filter[];
Json::Value config;

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];
		while((i++ < 3))
			ret += '=';
	}
	return ret;
}

#define swl(c,cpp,c11,pas) (lang=="c"?(c):lang=="c++"?(cpp):lang=="c++11"?(c11):lang=="pascal"?pas:-1)

inline int language_code(const std::string &oj, const std::string &lang)
{
	int ret(-1);
	if (oj == "ACdream")     ret = swl(1,	2,	-1,	-1); else
	if (oj == "CSU")         ret = swl(0,	1,	-1,	2); else
	if (oj == "CodeChef")    ret = swl(11,	1,	44,	22); else
	if (oj == "CodeForces")  ret = swl(10,	1,	42,	4); else
	if (oj == "FZU")         ret = swl(1,	0,	-1,	2); else
	if (oj == "Gym")         ret = swl(10,	1,	42,	4); else
	if (oj == "HDU")         ret = swl(1,	0,	-1,	4); else
	if (oj == "HUST")        ret = swl(0,	1,	-1,	2); else
	if (oj == "NBUT")        ret = swl(1,	2,	-1,	4); else
	if (oj == "POJ")         ret = swl(1,	0,	-1,	3); else
	if (oj == "SPOJ")        ret = swl(11,	1,	44,	22); else
	if (oj == "UESTC")       ret = swl(1,	2,	-1,	-1); else
	if (oj == "URAL")        ret = swl(27,	26,	-1,	31); else
	if (oj == "UVA")         ret = swl(1,	3,	5,	4); else
	if (oj == "UVALive")     ret = swl(1,	3,	5,	4); else
	if (oj == "ZOJ")         ret = swl(1,	2,	9,	-1);
	if (!~ret) throw std::runtime_error("not supported oj or language");
}

#undef swl

CURL *init()
{
	if (curl_global_init(CURL_GLOBAL_NOTHING) != CURLE_OK)
		throw std::runtime_error("curl_global_init failed");
	CURL *handle = curl_easy_init();
	if (!handle)
		throw std::runtime_error("curl_easy_init failed");
	curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Origin: http://acm.hust.edu.cn");
	headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
	headers = curl_slist_append(headers, "Accept-Encoding: deflate");
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.152 Safari/537.36");
	curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
	return handle;
}

struct Result
{
	std::string content;
	
	Json::Value parse()
	{
		Json::Value ret;
		Json::Reader reader;
		if (!reader.parse(content, ret)) ret = Json::nullValue;
		return ret;
	}
};

struct JudgeResult
{
	int time, memory, codeLength;
	bool ok;
	std::string verdict;
	JudgeResult(bool _ok) : ok(_ok) {}
	JudgeResult(const std::string &_verdict, int _time, int _memory, int _codeLength) : 
		verdict(_verdict), time(_time), memory(_memory), codeLength(_codeLength), ok(true)
	{
		std::transform(verdict.begin(), verdict.end(), verdict.begin(), tolower);
	}
};

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	if (userdata)
		((Result*)userdata)->content += std::string(ptr, size*nmemb);
	return size*nmemb;
}

void load(CURL *handle, const char *url, Result *writeTo = NULL, const std::string &data = "")
{
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data.c_str());
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, writeTo);
	curl_easy_perform(handle);
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, "");
	usleep(500 * 1000);
}

template <class Tres, class Tcallback>
Tres check_status(CURL *handle, const std::string &username, Tcallback callback)
{
	Result res;
	std::ostringstream data;
	data << status_filter << username;
	load(handle, "http://acm.hust.edu.cn/vjudge/problem/fetchStatus.action", &res, data.str());
	return callback(res);
}

int no_pending(Result &res)
{
#ifdef DEBUG_VJUDGE_HACK
	std::clog << "no_pending: " << res.content << std::endl;
#endif
	Json::Value status = res.parse();
	for (Json::Value::iterator i=status["data"].begin(); i!=status["data"].end(); i++)
		if ((*i)[3] == "Submitted" || (*i)[3] == "Waiting" || (*i)[3] == "Pending") return -1;
	return status["data"][0][0].asInt();
}

struct fetch_result
{
	int len, minsid;
	fetch_result(int _len, int _minsid) : len(_len), minsid(_minsid) {}
	
	JudgeResult operator()(Result &res)
	{
#ifdef DEBUG_VJUDGE_HACK
	std::clog << "fetch_result: " << res.content << std::endl;
#endif
		Json::Value status = res.parse();
		for (Json::Value::iterator i=status["data"].begin(); i!=status["data"].end(); i++)
		{
			if ((*i)[0] == minsid) break;
			if ((*i)[7] == len)
				if ((*i)[3] == "Waiting" || (*i)[3] == "Submitted" || (*i)[3] == "Pending")
					return JudgeResult(false);
				else
					return JudgeResult((*i)[3].asString(), (*i)[5].asInt(),(*i)[4].asInt(),len);
		}
		return false;
	}
};

void login(CURL *handle, const std::string &username, const std::string &passwd)
{
	Result res;
	std::ostringstream data;
	data << "username=" << username << "&password=" << passwd;
	load(handle, "http://acm.hust.edu.cn/vjudge/user/login.action", &res, data.str());
#ifdef DEBUG_VJUDGE_HACK
	std::clog << "login: " << res.content << std::endl;
#endif
	if (res.content != "\"success\"") throw std::runtime_error("login failed");
}

void submit(CURL *handle, const std::string &oj, const std::string &lang, const std::string &src, int pid)
{
	std::ostringstream data;
	data << "language=" << language_code(oj, lang) << "&isOpen=0&source=" << base64_encode((const unsigned char *)(src.c_str()), src.length()) << "&id=" << pid;
	load(handle, "http://acm.hust.edu.cn/vjudge/problem/submit.action", NULL, data.str());
}

void read_config()
{
	char buff[32768];
	FILE *f = fopen(VJUDGE_CONFIG_FILE, "r");
	buff[fread(buff,1,32767,f)]=0;
	if (!feof(f)) fclose(f), throw std::runtime_error("Filed to open config file");
	fclose(f);
	Json::Reader reader;
	if (!reader.parse(buff, config))
		throw std::runtime_error("Failed to read config file");
}

JudgeResult judge(const std::string &lang, const std::string &src, const std::string &oj, int pid)
{
	read_config();
	CURL *handle = init();
	load(handle, "http://acm.hust.edu.cn/vjudge/problem/status.action");
	while (true)
		for (Json::Value::iterator i=config.begin(); i!=config.end(); i++)
		{
			int max_sid = check_status<int>(handle, (*i)["username"].asString(), no_pending);
			if (!~max_sid) continue;
			login(handle, (*i)["username"].asString(),(*i)["passwd"].asString());
			submit(handle, oj, lang, src, pid);
			JudgeResult ret(false);
			int cnt = 0;
			while (!ret.ok)
			{
				if (++cnt == MAX_FETCH_TRIAL) break;
				ret = check_status<JudgeResult>(handle, (*i)["username"].asString(), fetch_result(src.length(), max_sid));
			}
			curl_easy_cleanup(handle);
			curl_global_cleanup();
			return ret;
		}
	curl_easy_cleanup(handle);
	curl_global_cleanup();
	return false;
}
namespace func
{
	iter vjudge_hack(const iter &lang, const iter &src, const iter &oj, const iter &pid)
	{
		try
		{
			JudgeResult got = judge(lang->as_str(), src->as_str(), oj->as_str(), pid->as_int());
			iter ret = _I_(new v_dict);
			ret->as_dict()["verdict"] = _I_(new v_str(got.verdict));
			ret->as_dict()["time"] = _I_(new v_int(got.time));
			ret->as_dict()["memory"] = _I_(new v_int(got.memory));
			ret->as_dict()["codeLength"] = _I_(new v_int(got.codeLength));
			ret->as_dict()["message"] = _I_(new v_str("judged by vjudge"));
			return ret;
		} catch (const std::runtime_error &e)
		{
			throw std::runtime_error("vjudge_hack : "+std::string(e.what()));
		}
	}
}

#ifdef DEBUG_VJUDGE_HACK
int main()
{
	FILE *f = fopen("testvjudge.cpp", "r");
	char buff[1048576];
	buff[fread(buff,1,1048575,f)]=0;
	fclose(f);
	JudgeResult ret = judge("c++", buff, "POJ", 10763);
	std::cout << ret.ok << std::endl;
	std::cout << ret.verdict << std::endl;
	std::cout << ret.time << std::endl;
	std::cout << ret.memory << std::endl;
	std::cout << ret.codeLength << std::endl;
	return 0;
}
#endif

const char status_filter[] = "draw=2&columns%5B0%5D%5Bdata%5D=0&columns%5B0%5D%5Bname%5D=&columns%5B0%5D%5Bsearchable%5D=true&columns%5B0%5D%5Borderable%5D=false&columns%5B0%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B0%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B1%5D%5Bdata%5D=1&columns%5B1%5D%5Bname%5D=&columns%5B1%5D%5Bsearchable%5D=true&columns%5B1%5D%5Borderable%5D=false&columns%5B1%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B1%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B2%5D%5Bdata%5D=2&columns%5B2%5D%5Bname%5D=&columns%5B2%5D%5Bsearchable%5D=true&columns%5B2%5D%5Borderable%5D=false&columns%5B2%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B2%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B3%5D%5Bdata%5D=3&columns%5B3%5D%5Bname%5D=&columns%5B3%5D%5Bsearchable%5D=true&columns%5B3%5D%5Borderable%5D=false&columns%5B3%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B3%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B4%5D%5Bdata%5D=4&columns%5B4%5D%5Bname%5D=&columns%5B4%5D%5Bsearchable%5D=true&columns%5B4%5D%5Borderable%5D=false&columns%5B4%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B4%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B5%5D%5Bdata%5D=5&columns%5B5%5D%5Bname%5D=&columns%5B5%5D%5Bsearchable%5D=true&columns%5B5%5D%5Borderable%5D=false&columns%5B5%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B5%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B6%5D%5Bdata%5D=6&columns%5B6%5D%5Bname%5D=&columns%5B6%5D%5Bsearchable%5D=true&columns%5B6%5D%5Borderable%5D=false&columns%5B6%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B6%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B7%5D%5Bdata%5D=7&columns%5B7%5D%5Bname%5D=&columns%5B7%5D%5Bsearchable%5D=true&columns%5B7%5D%5Borderable%5D=false&columns%5B7%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B7%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B8%5D%5Bdata%5D=8&columns%5B8%5D%5Bname%5D=&columns%5B8%5D%5Bsearchable%5D=true&columns%5B8%5D%5Borderable%5D=false&columns%5B8%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B8%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B9%5D%5Bdata%5D=9&columns%5B9%5D%5Bname%5D=&columns%5B9%5D%5Bsearchable%5D=true&columns%5B9%5D%5Borderable%5D=false&columns%5B9%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B9%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B10%5D%5Bdata%5D=10&columns%5B10%5D%5Bname%5D=&columns%5B10%5D%5Bsearchable%5D=true&columns%5B10%5D%5Borderable%5D=false&columns%5B10%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B10%5D%5Bsearch%5D%5Bregex%5D=false&columns%5B11%5D%5Bdata%5D=11&columns%5B11%5D%5Bname%5D=&columns%5B11%5D%5Bsearchable%5D=true&columns%5B11%5D%5Borderable%5D=false&columns%5B11%5D%5Bsearch%5D%5Bvalue%5D=&columns%5B11%5D%5Bsearch%5D%5Bregex%5D=false&order%5B0%5D%5Bcolumn%5D=0&order%5B0%5D%5Bdir%5D=desc&start=0&length=20&search%5Bvalue%5D=&search%5Bregex%5D=false&OJId=All&probNum=&res=0&language=&orderBy=run_id&un=";
