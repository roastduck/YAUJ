#include "mystr.h"

void cat2(char **c, char * const *a, char * const *b)
{
	int l = strlen(*a) + strlen(*b);
	*c = (char*) malloc((l+1) * sizeof(char));
	strcpy(*c, *a);
	strcat(*c, *b);
	free(*a);
	free(*b);
	//*a = *b = 0;
}

void cat3(char **d, char * const *a, char * const *b, char * const *c)
{
	int l = strlen(*a) + strlen(*b) + strlen(*c);
	*d = (char*) malloc((l+1) * sizeof(char));
	strcpy(*d, *a);
	strcat(*d, *b);
	strcat(*d, *c);
	free(*a);
	free(*b);
	free(*c);
	//*a = *b = *c = 0;
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
