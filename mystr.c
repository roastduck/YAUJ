#include <stdarg.h>
#include "mystr.h"

void cat(char *pat, char **t, ...)
{
	va_list vl;
	va_start(vl,t);
	int l = 0;
	char *i;
	for (i=pat; *i; i++)
		l += strlen(va_arg(vl,char*));
	va_end(vl);
	*t = (char*) malloc((l+1) * sizeof(char));
	**t = 0;
	va_start(vl,t);
	for (i=pat; *i; i++)
	{
		char *p=va_arg(vl,char*);
		strcat(*t,p);
		if (*i=='-') free(p);
	}
	va_end(vl);
}

void cn3(char **a, char *s, int l)
{
	*a = (char*) malloc((l+1) * sizeof(char));
	strncpy(*a, s, l);
	*(*a+l) = 0;
}

void cn2(char **a, char *s)
{
	cn3(a, s, strlen(s));
}
