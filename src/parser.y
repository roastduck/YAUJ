%{

#include <stdio.h>
#include "mystr.h"

extern int yylineno;
char curFileName[64];

typedef struct _NODE
{
	char *symbol;
	struct _NODE *next;
} NODE;

char *body;
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
%expect 1
%expect-rr 1

%token IDENTIFIER INTEGER FLOAT EQ FEQ NEQ NFEQ AND OR FOR WHILE DO IF VMINUS VPOSI
%token PLUSPLUS MINUSMINUS VFPLUSPLUS VFMINUSMINUS ELSE TRUE FALSE STR SUBACCESS
%token FOREACH ARROW AS BREAK CONTINUE TRY CATCH THROW PLUSEQ MINUSEQ

%right '=' PLUSEQ MINUSEQ
%left OR
%left AND
%left EQ FEQ NEQ NFEQ
%left '<' '>' LEQ GEQ
%left '+' '-'
%left '*' '/' '%'
%right '!' VMINUS VPOSI VFPLUSPLUS VFMINUSMINUS
%right PLUSPLUS MINUSMINUS
%left '[' ']'

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
|	nonemp_contents stmt	{ cat("--",&$$,$1,$2); }

for :
    FOR '(' expr ';' expr ';' expr ')' block_or_stmt
						{ cat("---------",&$$,$1,$2,$3,$4,$5,$6,$7,$8,$9); }
;

foreach :
	FOREACH expr AS var block_or_stmt
						{ cat("+-+-+-+",&$$,"foreach(",$2,",[](const iter &",$4,"){",$5,"});\n"); }
|	FOREACH expr AS var ARROW var block_or_stmt
						{ cat("+-+-+-+-+",&$$,"foreach(",$2,",[](const iter &",$4,",const iter &",$6,"){",$7,"});\n"); }

while :
	WHILE condition block_or_stmt
						{ cat("---",&$$,$1,$2,$3); }
;

do_while :
	DO block_or_stmt WHILE condition
						{ cat("----",&$$,$1,$2,$3,$4); }
;

if :
	IF condition block_or_stmt
						{ cat("---",&$$,$1,$2,$3); }
|	IF condition block_or_stmt ELSE block_or_stmt
						{ cat("-----",&$$,$1,$2,$3,$4,$5); }
;

try :
    TRY block CATCH block	{ cat("----",&$$,$1,$2,$3,$4); }
;

condition :
	'(' expr ')'			{ cat("---",&$$,$1,$2,$3); }
;

block_or_stmt :
	block
|	stmt					{ cat("+-",&$$,"\n",$1); }
;

block :
	'{' contents '}'		{ cat("+-+",&$$,"\n{\n",$2,"\n}\n"); }
;	

stmt :
	expr ';'				{
							char s[32], t[32];
							sprintf(s,";}LINE_CAT(\"%d\")\n",yylineno);
							sprintf(t,"#line %d \"%s\" try{",yylineno,curFileName);
							cat("+-+",&$$,t,$1,s);
						}
|	BREAK ';'				{ cat("-+",&$$,$1,";\n"); }
|	CONTINUE ';'			{ cat("-+",&$$,$1,";\n"); }
|	if	
|	while
|	do_while
|	for
|	foreach
|	try
|	THROW ';'				{ cat("-+",&$$,$1,";\n"); }
;

expr :
	const
|	var
|	func_call				{ cat("+-",&$$,"func::",$1); }
|	expr '[' ']'
						{ cat("-+",&$$,$1,".add()[_I_(new v_int(-1))]"); }
|	expr '[' expr ']'
						{ cat("----",&$$,$1,$2,$3,$4); }
|	'(' expr ')'			{ cat("---",&$$,$1,$2,$3); }
|	expr '+' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr '-' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr '*' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr '/' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr '%' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr '=' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr PLUSEQ expr		{ cat("---",&$$,$1,$2,$3); }
|	expr MINUSEQ expr		{ cat("---",&$$,$1,$2,$3); }
|	expr '<' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr '>' expr			{ cat("---",&$$,$1,$2,$3); }
|	expr LEQ expr			{ cat("---",&$$,$1,$2,$3); }
|	expr GEQ expr			{ cat("---",&$$,$1,$2,$3); }
|	expr EQ expr			{ cat("---",&$$,$1,$2,$3); }
|	expr NEQ expr			{ cat("---",&$$,$1,$2,$3); }
|	expr FEQ expr			{ cat("+-+-+",&$$,"FEQ(",$1,",",$3,")"); }
|	expr NFEQ expr			{ cat("+-+-+",&$$,"NFEQ(",$1,",",$3,")"); }
|	'-' expr %prec VMINUS	{ cat("--",&$$,$1,$2); }
|	'+' expr %prec VPOSI	{ cat("--",&$$,$1,$2); }
|	expr PLUSPLUS			{ cat("--",&$$,$1,$2); }
|	expr MINUSMINUS		{ cat("--",&$$,$1,$2); }
|	PLUSPLUS expr	%prec VFPLUSPLUS
						{ cat("--",&$$,$1,$2); }
|	MINUSMINUS expr %prec VFMINUSMINUS
						{ cat("--",&$$,$1,$2); }
|	'!' expr				{ cat("--",&$$,$1,$2); }
|	expr AND expr			{ cat("---",&$$,$1,$2,$3); }
|	expr OR expr			{ cat("---",&$$,$1,$2,$3); }
;

var :
	IDENTIFIER			{ /*addSymbol($1),*/ cat("+-",&$$,"_v_",$1), addSymbol($$); }
;

const :
	INTEGER				{ cat("+-+",&$$,"_I_(new v_int(",$1,"))"); }
|	FLOAT				{ cat("+-+",&$$,"_I_(new v_float(",$1,"))"); }
|	TRUE					{ cn2(&$$,"_I_(new v_bool(true))"); }
|	FALSE				{ cn2(&$$,"_I_(new v_bool(false))"); }
|	STR					{ cat("+-+",&$$,"_I_(new v_str(",$1,"))"); }
|	list
|	dict
;

list :
	'[' list_ele ']'		{ cat("+-",&$$,"_I_(new v_list())",$2); }
;

list_ele :
						{ $$ = (char*) malloc(sizeof(char)), *$$ = 0; }
|	list_nonemp_ele
;

list_nonemp_ele :
	expr					{ cat("+-+",&$$,".add(",$1,")"); }
|	list_nonemp_ele ',' expr
						{ cat("-+-+",&$$,$1,".add(",$3,")"); }
;

dict :
	'{' dict_ele '}'		{ cat("+-",&$$,"_I_(new v_dict())",$2); }
;

dict_ele :
						{ $$ = (char*) malloc(sizeof(char)), *$$ = 0; }
|	dict_nonemp_ele
;

dict_nonemp_ele :
	dict_expr				{ cat("+-+",&$$,".add(",$1,")"); }
|	dict_nonemp_ele ',' dict_expr
						{ cat("-+-+",&$$,$1,".add(",$3,")"); }
;

dict_expr :
	expr ':' expr			{ cat("+-+-+",&$$,"std::pair<std::string,iter>((",$1,")->as_str(),",$3,")"); }
|	expr					{ cat("+-+",&$$,"std::pair<std::string,iter>((",$1,")->as_str(),_I_(new v_bool(true)))"); }

func_call :
	IDENTIFIER '(' params ')'
						{ cat("----",&$$,$1,$2,$3,$4); }
;

params :
						{ $$ = (char*) malloc(sizeof(char)), *$$ = 0; }
|	nonemp_params
;

nonemp_params :
	expr
|	nonemp_params ',' expr	{ cat("---",&$$,$1,$2,$3); }
;

%%


int yyerror(char *s)
{
	fprintf(stderr, "parser : %s, line %d: %s\n", curFileName, yylineno, s);
	return 1;
}

int main()
{
	FILE *decl_part, *init_part, *run_part;
	decl_part = fopen("decl_part","w");
	init_part = fopen("init_part","w");
	run_part = fopen("run_part","w");
	int stat;
	puts("parsing init.src");
	freopen("init.src","r",stdin);
	strcpy(curFileName,"Initialization Part");
	stat = yyparse();
	if (stat) return stat;
	fputs(body,init_part);
	fclose(stdin), fclose(init_part);
	puts("parsing run.src");
	yylineno=1;
	freopen("run.src","r",stdin);
	strcpy(curFileName,"Run Part");
	stat = yyparse();
	if (stat) return stat;
	fputs(body,run_part);
	fclose(stdin), fclose(run_part);
	addSymbol("_v_submission");
	addSymbol("_v_filemode");
	addSymbol("_v_result");
	if (front.symbol)
	{
		NODE *tail = &front;
		for (; tail->next; tail=tail->next)
		{
			fprintf(decl_part, tail == &front? "iter ": ", ");
			fprintf(decl_part, tail->symbol);
		}
		fputs(";",decl_part);
	}
	return 0;
}

