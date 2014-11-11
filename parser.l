%{

#include <stdio.h>
#include "mystr.h"
#include "parser.tab.h"

%}

%option nodefault
%option noyywrap
%option yylineno

A       [aA]
B       [bB]
C       [cC]
D       [dD]
E       [eE]
F       [fF]
G       [gG]
H       [hH]
I       [iI]
J       [jJ]
K       [kK]
L       [lL]
M       [mM]
N       [nN]
O       [oO]
P       [pP]
Q       [qQ]
R       [rR]
S       [sS]
T       [tT]
U       [uU]
V       [vV]
W       [wW]
X       [xX]
Y       [yY]
Z       [zZ]

%%

{I}{F}						{ cn2(&yylval," if "); return(IF); }
{D}{O}						{ cn2(&yylval," do "); return(DO); }
{W}{H}{I}{L}{E}				{ cn2(&yylval," while "); return(WHILE); }
{F}{O}{R}						{ cn2(&yylval," for "); return(FOR); }
{E}{L}{S}{E}					{ cn2(&yylval," else "); return(ELSE); }
{T}{R}{U}{E}					{ cn2(&yylval," true "); return(TRUE); }
{F}{A}{L}{S}{E}				{ cn2(&yylval," false "); return(FALSE); }
{F}{O}{R}{E}{A}{C}{H}			{ cn2(&yylval," foreach "); return(FOREACH); }
{A}{S}						{ cn2(&yylval," as "); return(AS); }
{B}{R}{E}{A}{K}				{ cn2(&yylval," break "); return(BREAK); }
{C}{O}{N}{T}{I}{N}{U}{E}			{ cn2(&yylval," continue "); return(CONTINUE); }
"->"							{ cn3(&yylval,yytext,yyleng); return(ARROW); }
"+"							{ cn3(&yylval,yytext,yyleng); return('+'); }
"-"							{ cn3(&yylval,yytext,yyleng); return('-'); }
"*"							{ cn3(&yylval,yytext,yyleng); return('*'); }
"/"							{ cn3(&yylval,yytext,yyleng); return('/'); }
"%" | {M}{O}{D}				{ cn2(&yylval,"%"); return('%'); }
"="							{ cn3(&yylval,yytext,yyleng); return('='); }
"<"							{ cn3(&yylval,yytext,yyleng); return('<'); }
">"							{ cn3(&yylval,yytext,yyleng); return('>'); }
"<="							{ cn3(&yylval,yytext,yyleng); return(LEQ); }
">="							{ cn3(&yylval,yytext,yyleng); return(GEQ); }
"=="							{ cn3(&yylval,yytext,yyleng); return(EQ); }
"==="						{ return(FEQ); }
"!="							{ cn3(&yylval,yytext,yyleng); return(NEQ); }
"!==="						{ return(NFEQ); }
"!" | {N}{O}{T}				{ cn2(&yylval,"!"); return('!'); }
"&&" | {A}{N}{D}				{ cn2(&yylval,"&&"); return(AND); }
"||" | {O}{R}					{ cn2(&yylval,"||"); return(OR); }
"++"							{ cn3(&yylval,yytext,yyleng); return(PLUSPLUS); }
"--"							{ cn3(&yylval,yytext,yyleng); return(MINUSMINUS); }
","							{ cn3(&yylval,yytext,yyleng); return(','); }
";"							{ cn3(&yylval,yytext,yyleng); return(';'); }
":"							{ cn3(&yylval,yytext,yyleng); return(':'); }
"("							{ cn3(&yylval,yytext,yyleng); return('('); }
")"							{ cn3(&yylval,yytext,yyleng); return(')'); }
"["							{ cn3(&yylval,yytext,yyleng); return('['); }
"]"							{ cn3(&yylval,yytext,yyleng); return(']'); }
"{"							{ cn3(&yylval,yytext,yyleng); return('{'); }
"}"							{ cn3(&yylval,yytext,yyleng); return('}'); }
"."							{ cn3(&yylval,yytext,yyleng); return('.'); }
\"[^\"]*?\"					{ cn3(&yylval,yytext,yyleng); return(STR); }
[a-zA-Z_]([a-zA-Z0-9_])*			{ cn3(&yylval,yytext,yyleng); return(IDENTIFIER); }
[0-9]+						{ cn3(&yylval,yytext,yyleng); return(INTEGER); }
[0-9]*\.[0-9]+					{ cn3(&yylval,yytext,yyleng); return(FLOAT); }
"//".*?\n						;
[\t\r\f\n ]+					;
.							fprintf(stderr, "unexpected character '%c' at line %d\n", yytext[0], yylineno);

%%

