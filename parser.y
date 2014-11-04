%{

#include <stdio.h>
#include "mystr.h"

extern int yylineno;

typedef struct _NODE
{
	char *symbol;
	struct _NODE *next;
} NODE;

char *t1, *t2, *t3, *body;
NODE front;

inline void addSymbol(char *s)
{
	NODE *tail = &front;
	while (tail->next)
	{
		if (!strcmp(s, tail->symbol)) return;
		tail = tail->next;
	}
	int l = strlen(s);
	tail->symbol = (char*) malloc((l+1) * sizeof(char));
	strcpy(tail->symbol, s);
	tail->next = (NODE*) malloc(sizeof(NODE));
	tail->next->symbol = 0;
	tail->next->next = 0;
}

%}

%error-verbose
%glr-parser
%expect 22
%expect-rr 1

%token IDENTIFIER INTEGER FLOAT EQ FEQ NEQ NFEQ AND OR FOR WHILE DO IF VMINUS VPOSI
%token PLUSPLUS MINUSMINUS VFPLUSPLUS VFMINUSMINUS ELSE TRUE FALSE STR SUBACCESS
%token FOREACH ARROW AS

%right '='
%left OR
%left AND
%left EQ FEQ NEQ NFEQ
%left '<' '>' LEQ GEQ
%left '+' '-'
%left '*' '/' '%'
%right '!' VMINUS VPOSI VFPLUSPLUS VFMINUSMINUS
%right PLUSPLUS MINUSMINUS
%left '.' SUBACCESS

%%

file :
	contents				{ body = $1; }
;

contents :
						{ $$ = (char*) malloc(sizeof(char)), *$$ = 0; }
|	nonemp_contents
;

nonemp_contents :
	stmt
|	nonemp_contents stmt	{ cat2(&$$,&$1,&$2); }

for :
    FOR '(' expr ';' expr ';' expr ')' block_or_stmt
						{ cat3(&t1,&$2,&$3,&$4), cat3(&t2,&t1,&$5,&$6), cat3(&t1,&t2,&$7,&$8), cat3(&$$,&$1,&t1,&$9); }
;

foreach :
	FOREACH expr AS var block_or_stmt
						{ cn2(&t1,"foreach("), cn2(&t2,",[](const iter &"), cat3(&t3,&t1,&$2,&t2), cn2(&t1,"){"), cat3(&t2,&t3,&$4,&t1), cn2(&t1,"});\n"), cat3(&$$,&t2,&$5,&t1); }
|	FOREACH expr AS var ARROW var block_or_stmt
						{ cn2(&t1,"foreach("), cn2(&t2,",[](const iter &"), cat3(&t3,&t1,&$2,&t2), cn2(&t1,",const iter &"), cat3(&t2,&t3,&$4,&t1), cn2(&t1,"){"), cat3(&t3,&t2,&$6,&t1), cn2(&t1,"});\n"), cat3(&$$,&t3,&$7,&t1); }

while :
	WHILE condition block_or_stmt
						{ cat3(&$$,&$1,&$2,&$3); }
;

do_while :
	DO block_or_stmt WHILE condition
						{ cat2(&t1,&$1,&$2), cat3(&$$,&t1,&$3,&$4); }
;

if :
	IF condition block_or_stmt
						{ cat3(&$$,&$1,&$2,&$3); }
|	IF condition block_or_stmt ELSE block_or_stmt
						{ cat3(&t1,&$1,&$2,&$3), cat3(&$$,&t1,&$4,&$5); }
;

condition :
	'(' expr ')'			{ cat3(&$$,&$1,&$2,&$3); }
;

block_or_stmt :
	'{' contents '}'		{ cn2(&t1,"\n{\n"), cn2(&t2,"\n}\n"), cat3(&$$,&t1,&$2,&t2); }
|	stmt					{ cn2(&t1,"\n"), cat2(&$$,&t1,&$1); }
;

stmt :
	expr ';'				{ cn2(&t1,";\n"), cat2(&$$,&$1,&t1); }
|	if	
|	while
|	do_while
|	for
|	foreach
;

expr :
	const
|	var
|	func_call				{ cn2(&t1,"func::"), cat2(&$$,&t1,&$1); }
|	expr '[' ']' %prec SUBACCESS
						{ cn2(&t1,".add()[_I_(new v_int(-1))]"), cat2(&$$,&$1,&t1); }
|	expr '[' expr ']' %prec SUBACCESS
						{ cat3(&t1,&$1,&$2,&$3), cat2(&$$,&t1,&$4); }
|	'(' expr ')'			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '+' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '-' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '*' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '/' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '%' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '=' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '<' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr '>' expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr LEQ expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr GEQ expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr EQ expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr NEQ expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr FEQ expr			{ cn2(&t1,"FEQ("), cn2(&t2,","), cat3(&t3,&t1,&$1,&t2), cn2(&t1,")"), cat3(&$$,&t3,&$3,&t1); }
|	expr NFEQ expr			{ cn2(&t1,"NFEQ("), cn2(&t2,","), cat3(&t3,&t1,&$1,&t2), cn2(&t1,")"), cat3(&$$,&t3,&$3,&t1); }
|	'-' expr %prec VMINUS	{ cat2(&$$,&$1,&$2); }
|	'+' expr %prec VPOSI	{ cat2(&$$,&$1,&$2); }
|	expr PLUSPLUS			{ cat2(&$$,&$1,&$2); }
|	expr MINUSMINUS		{ cat2(&$$,&$1,&$2); }
|	PLUSPLUS expr	%prec VFPLUSPLUS
						{ cat2(&$$,&$1,&$2); }
|	MINUSMINUS expr %prec VFMINUSMINUS
						{ cat2(&$$,&$1,&$2); }
|	'!' expr				{ cat2(&$$,&$1,&$2); }
|	expr AND expr			{ cat3(&$$,&$1,&$2,&$3); }
|	expr OR expr			{ cat3(&$$,&$1,&$2,&$3); }
;

var :
	IDENTIFIER			{ /*addSymbol($1),*/ cn2(&t1,"_v_"), cat2(&$$,&t1,&$1), addSymbol($$); }
;

const :
	INTEGER				{ cn2(&t1,"_I_(new v_int("), cn2(&t2,"))"), cat3(&$$,&t1,&$1,&t2); }
|	FLOAT				{ cn2(&t1,"_I_(new v_float("), cn2(&t2,"))"), cat3(&$$,&t1,&$1,&t2); }
|	TRUE					{ cn2(&$$,"_I_(new v_bool(true))"); }
|	FALSE				{ cn2(&$$,"_I_(new v_bool(false))"); }
|	STR					{ cn2(&t1,"_I_(new v_str("), cn2(&t2,"))"), cat3(&$$,&t1,&$1,&t2); }
|	list
|	dict
;

list :
	'[' list_ele ']'		{ cn2(&t1,"_I_(new v_list())"), cat2(&$$,&t1,&$2); }
;

list_ele :
						{ $$ = (char*) malloc(sizeof(char)), *$$ = 0; }
|	list_nonemp_ele
;

list_nonemp_ele :
	expr					{ cn2(&t1,".add("), cn2(&t2,")"), cat3(&$$,&t1,&$1,&t2); }
|	list_nonemp_ele ',' expr
						{ cn2(&t1,".add("), cn2(&t2,")"), cat3(&t3,&t1,&$3,&t2), cat2(&$$,&$1,&t3); }
;

dict :
	'{' dict_ele '}'		{ cn2(&t1,"_I_(new v_dict())"), cat2(&$$,&t1,&$2); }
;

dict_ele :
						{ $$ = (char*) malloc(sizeof(char)), *$$ = 0; }
|	dict_nonemp_ele
;

dict_nonemp_ele :
	dict_expr				{ cn2(&t1,".add("), cn2(&t2,")"), cat3(&$$,&t1,&$1,&t2); }
|	dict_nonemp_ele ',' dict_expr
						{ cn2(&t1,".add("), cn2(&t2,")"), cat3(&t3,&t1,&$3,&t2), cat2(&$$,&$1,&t3); }
;

dict_expr :
	expr ':' expr			{ cn2(&t1,"std::pair<std::string,iter>(("), cn2(&t2,")->as_str(),"), cat3(&t3,&t1,&$1,&t2), cn2(&t1,")"), cat3(&$$,&t3,&$3,&t1); }
|	expr					{ cn2(&t1,"std::pair<std::string,iter>(("), cn2(&t2,")->as_str(),_I_(0))"), cat3(&$$,&t1,&$1,&t2); }

func_call :
	IDENTIFIER '(' params ')'
						{ cat3(&t2,&$2,&$3,&$4), cat2(&$$,&$1,&t2); }
;

params :
						{ $$ = (char*) malloc(sizeof(char)), *$$ = 0; }
|	nonemp_params
;

nonemp_params :
	expr
|	nonemp_params ',' expr	{ cat3(&$$,&$1,&$2,&$3); }
;

%%

int yyerror(char *s)
{
	fprintf(stderr, "line %d: %s\n", yylineno, s);
	return 1;
}

int main()
{
	int stat = yyparse();
	if (stat) return stat;
	puts("#include \"interpreter.h\"");
	puts("#include \"function.h\"");
	if (front.symbol)
	{
		//puts("namespace var {");
		NODE *tail = &front;
		for (; tail->next; tail=tail->next)
		{
			printf(tail == &front? "iter ": ", ");
			printf(tail->symbol);
		}
		puts(";");
		//puts("}");
	}
	puts("int main() {");
	puts(body);
	puts("return 0;");
	puts("}");
	return 0;
}

