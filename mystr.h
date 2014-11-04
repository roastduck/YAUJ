#ifndef included_mystr_h
#define included_mystr_h

#include<string.h>
#include<stdlib.h>

extern void cat2(char **c, char * const *a, char * const *b);
extern void cat3(char **d, char * const *a, char * const *b, char * const *c);
extern void cn3(char **a, char *s, int l);
extern void cn2(char **a, char *s);

#define YYSTYPE char*

#endif
