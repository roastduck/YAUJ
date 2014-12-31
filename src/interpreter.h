#ifndef INCLUDED_INTERPRETER_H
#define INCLUDED_INTERPRETER_H

#include<map>
#include<string>
#include<vector>
#include<memory>
#include<jsoncpp/json/json.h>

const int INT =	(1<<0);
const int FLOAT =	(1<<1);
const int BOOL	=	(1<<2);
const int STR =	(1<<3);
const int LIST =	(1<<4);
const int DICT =	(1<<5);

class v_base;
class v_int;
class v_float;
class v_bool;
class v_str;
class v_list;
class v_dict;
class iter;

struct user_error {};

typedef std::shared_ptr<v_base> v_base_ptr;

#define _I_(x) iter(v_base_ptr(x))

class v_base
{
	public :
		virtual ~v_base();
		virtual int to() const;
		virtual int as_int() const;
		virtual double as_float() const;
		virtual bool as_bool() const;
		virtual Json::Value as_json() const;
		virtual std::string as_str() const;
		virtual std::vector<iter> &as_list();
		virtual std::map<std::string,iter> &as_dict();
		virtual v_base_ptr clone() const;
};

class v_int : public v_base
{
	int data;
	public :
		v_int(int x);
		int to() const;
		int as_int() const;
		double as_float() const;
		bool as_bool() const;
		Json::Value as_json() const;
		std::string as_str() const;
		v_base_ptr clone() const;
};

class v_float : public v_base
{
	double data;
	public :
		v_float(double x);
		int to() const;
		double as_float() const;
		bool as_bool() const;
		Json::Value as_json() const;
		std::string as_str() const;
		v_base_ptr clone() const;
};

class v_bool : public v_base
{
	bool data;
	public :
		v_bool(bool x);
		int to() const;
		int as_int() const;
		double as_float() const;
		bool as_bool() const;
		Json::Value as_json() const;
		v_base_ptr clone() const;
};

class v_str : public v_base
{
	std::string data;
	public :
		v_str(std::string x);
		int to() const;
		bool as_bool() const;
		Json::Value as_json() const;
		std::string as_str() const;
		v_base_ptr clone() const;
};

class v_list : public v_base
{
	std::vector<iter> data;
	public :
		v_list();
		v_list(const std::vector<iter> &x);
		int to() const;
		bool as_bool() const;
		Json::Value as_json() const;
		std::vector<iter> &as_list();
		v_base_ptr clone() const;
};

class v_dict : public v_base
{
	std::map<std::string,iter> data;
	public :
		v_dict();
		v_dict(const std::map<std::string,iter> &x);
		int to() const;
		bool as_bool() const;
		Json::Value as_json() const;
		std::map<std::string,iter> &as_dict();
		v_base_ptr clone() const;
};

class iter
{
	v_base_ptr ptr;
	public :
		iter();
		iter(v_base_ptr x);
		iter(const iter &x);
		iter &operator=(const iter &x);
		v_base &operator*() const;
		v_base_ptr operator->() const;
		operator bool() const;
		iter operator!() const;
		iter &operator[](const iter &x) const;
		iter &operator++();
		iter &operator--();
		iter operator++(int);
		iter operator--(int);
		const iter &add(const iter &x = _I_(0)) const;
		const iter &add(const std::pair<std::string,iter> &x) const;
};

iter operator+(const iter &a, const iter &b);
iter operator-(const iter &a, const iter &b);
iter operator*(const iter &a, const iter &b);
iter operator/(const iter &a, const iter &b);
iter operator%(const iter &a, const iter &b);
iter operator<(const iter &a, const iter &b);
iter operator>(const iter &a, const iter &b);
iter operator<=(const iter &a, const iter &b);
iter operator>=(const iter &a, const iter &b);
iter operator==(const iter &a, const iter &b);
iter operator!=(const iter &a, const iter &b);
iter operator&&(const iter &a, const iter &b);
iter operator||(const iter &a, const iter &b);

iter FEQ(const iter &a, const iter &b);
iter NFEQ(const iter &a, const iter &b);
void foreach(const iter &a, void(*work)(const iter&));
void foreach(const iter &a, void(*work)(const iter&,const iter&));

#endif
