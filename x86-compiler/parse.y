/* Parser */

%{

#include <iostream>
#include <cstdio>
#include <cstring>
#include "tree.h"
#include "main.h"
int yyerror(const char *s);
int yylex(void);

int linenum = 1;

%}

%union{
	tree * treeval;
	double dval;
	unsigned int uval;
	signed int ival;
	char * str;
}

%token <dval> DOUBLE
%token <ival> INT
%token <str> NAME
%token <str> STRING

%token ADDEQUAL     // +=
%token MINUSEQUAL   // -=
%token MULEQUAL     // *=
%token DIVEQUAL     // /=

%token INCREMENT    // ++
%token DECREMENT    // --

%token IS_EQUAL     // ==
%token NOT_EQUAL    // !=
%token GREATEROREQUAL // >=
%token LESSOREQUAL  // <=

%token U_PLUS
%token U_MINUS

%token POSTINC
%token POSTDEC

%token IF
%token ELSE
%token WHILE
%token FOR
%token BREAK
%token CONTINUE
%token RETURN
%token GLOBAL

%token LOG_AND
%token LOG_OR

%token T_INT
%token T_BOOL
%token T_STRING
%token T_FLOAT

%left ','
%right '=' ADDEQUAL MINUSEQUAL MULEQUAL DIVEQUAL
%left LOG_OR
%left LOG_AND
%left IS_EQUAL NOT_EQUAL
%left '>' '<' GREATEROREQUAL LESSOREQUAL
%left '+' '-'
%left '*' '/'
%right '!'
%left INCREMENT DECREMENT U_PLUS U_MINUS
%left '(' ')' '[' ']' POSTINC POSTDEC

%type <treeval> exp
%type <uval> type_spec;
%type <treeval> func_extern_spec_param
%type <treeval> func_spec_param
%type <treeval> func_call_param
%type <treeval> ifstmt
%type <treeval> whilestmt
%type <treeval> forexp
%type <treeval> forstmt
%type <treeval> line
%type <treeval> codelines

%start input 

%%
input: /* empty */
	| input func
	;

type_spec: T_INT { $$ = TYPE_SINT; }
	| T_BOOL     { $$ = TYPE_BOOL; }
	| T_STRING   { $$ = TYPE_STRING; }
	| T_FLOAT    { $$ = TYPE_FLOAT; }
	;

func_extern_spec_param: type_spec   { $$ = build_specparam(NULL, $1, NULL); }
	| type_spec ',' func_extern_spec_param { $$ = build_specparam(NULL, $1, $3); }
	;

func_spec_param: type_spec NAME          { $$ = build_specparam(build_var($2), $1, NULL); }
	| type_spec NAME ',' func_spec_param { $$ = build_specparam(build_var($2), $1, $4); }
	;

func:
	         type_spec NAME '(' ')'                 '{' codelines '}' { build_function($2, NULL, $6, false, false, $1); }
	|        type_spec NAME '(' func_spec_param ')' '{' codelines '}' { build_function($2, $4, $7, false, false, $1); }
	| GLOBAL type_spec NAME '(' ')'                 '{' codelines '}' { build_function($3, NULL, $7, true, false, $2); }
	| GLOBAL type_spec NAME '(' func_spec_param ')' '{' codelines '}' { build_function($3, $5, $8, true, false, $2); }
	| type_spec NAME '(' ')'                         lineend   { build_function($2, NULL, NULL, false, true, $1); }
	| type_spec NAME '(' func_extern_spec_param ')'  lineend   { build_function($2, $4, NULL, false, true, $1); }
	;

codelines: /* empty */ { $$ = NULL; }
	| codelines '{' codelines '}' { $$ = build_line($3, $1, linenum); }
	| codelines ifstmt { $$ = build_line($2, $1, linenum); }
    | codelines whilestmt { $$ = build_line($2, $1, linenum); }
	| codelines forstmt { $$ = build_line($2, $1, linenum); }
	| codelines line { $$ = build_line($2, $1, linenum); }
	| codelines lineend { $$ = $1; }
	;

ifstmt:	IF '(' exp ')' '{' codelines '}'             { $$ = build_if($3, $6, NULL); }
	| ifstmt ELSE IF '(' exp ')' '{' codelines '}'   { $$ = build_if($5, $8, $1); }
	| ifstmt ELSE '{' codelines '}'                  { $$ = build_else($4, $1); }
	;

whilestmt: WHILE '(' exp ')' '{' codelines '}' { $$ = build_while($3, $6); }
	;

forexp: exp       { $$ = $1; }
	| /* empty */ { $$ = 0; }
	;

forstmt: FOR '(' forexp lineend forexp lineend forexp ')' '{' codelines '}'  { $$ = build_for($3, $5, $7, $10, linenum); }
	;

lineend:
	';'
	;

line:
	RETURN exp lineend   { $$ = build_unary($2, TREE_RETURN); }
	| BREAK lineend      { $$ = build_unary(0, TREE_BREAK); }
	| CONTINUE lineend   { $$ = build_unary(0, TREE_CONTINUE); }
	| exp lineend        { $$ = $1; }
	;

func_call_param: /* empty */ { $$ = NULL; }
	| exp { $$ = $1; }
	;

exp:    '(' exp ')'           { $$ = build_unary($2, TREE_BRACKET); }
	| NAME '(' func_call_param ')' { $$ = build_func_call($1, $3); }
	| type_spec '(' exp ')'   { $$ = build_typeconv($3, $1); }
	| NAME '[' exp ']'        { $$ = build_dual(build_var($1), $3, TREE_SUBSCR); }
	| exp '+' exp             { $$ = build_dual($1, $3, TREE_ADD); }
	| exp '*' exp             { $$ = build_dual($1, $3, TREE_MULTIPLY); }
	| exp '-' exp             { $$ = build_dual($1, $3, TREE_SUBTRACT); }
	| exp '/' exp             { $$ = build_dual($1, $3, TREE_DIVIDE); }
	| INT                     { $$ = build_sint($1); }
	| DOUBLE                  { $$ = build_float($1); }
	| STRING                  { $$ = build_string($1); }
	| NAME                    { $$ = build_var($1); }
	| '+' exp %prec U_PLUS    { $$ = $2; }
	| '-' exp %prec U_MINUS   { $$ = build_unary($2, TREE_NEG); }
	| '!' exp                 { $$ = build_unary($2, TREE_NOT); }
	| exp IS_EQUAL exp        { $$ = build_dual($1, $3, TREE_ISEQUAL); }
	| exp NOT_EQUAL exp       { $$ = build_dual($1, $3, TREE_NOTEQUAL); }
	| exp '>' exp             { $$ = build_dual($1, $3, TREE_GT); }
	| exp '<' exp             { $$ = build_dual($1, $3, TREE_LT); }
	| exp GREATEROREQUAL exp  { $$ = build_dual($1, $3, TREE_GTOREQUAL); }
	| exp LESSOREQUAL exp     { $$ = build_dual($1, $3, TREE_LTOREQUAL); }
	| exp LOG_AND exp         { $$ = build_dual($1, $3, TREE_LOG_AND); }
	| exp LOG_OR exp          { $$ = build_dual($1, $3, TREE_LOG_OR); }
	| NAME '[' exp ']' '=' exp { $$ = build_subscr_set(build_var($1), $3, $6); }
	| NAME '=' exp            { $$ = build_dual(build_var($1), $3, TREE_EQUALS); }
	| NAME ADDEQUAL exp       { $$ = build_dual(build_var($1), $3, TREE_ADDEQUALS); }
	| NAME MINUSEQUAL exp     { $$ = build_dual(build_var($1), $3, TREE_SUBEQUALS); }
	| NAME MULEQUAL exp       { $$ = build_dual(build_var($1), $3, TREE_MULEQUALS); }
	| NAME DIVEQUAL exp       { $$ = build_dual(build_var($1), $3, TREE_DIVEQUALS); }
	| INCREMENT NAME          { $$ = build_unary(build_var($2), TREE_PREINC); }
	| DECREMENT NAME          { $$ = build_unary(build_var($2), TREE_PREDEC); }
	| NAME INCREMENT %prec POSTINC { $$ = build_unary(build_var($1), TREE_POSTINC); }
	| NAME DECREMENT %prec POSTDEC { $$ = build_unary(build_var($1), TREE_POSTDEC); }
	| exp ',' exp             { $$ = build_dual($1, $3, TREE_COMMA); }
	;
%%

int yyerror(const char * s){
	printf("Error: %s on line %i\n", s, linenum);
}

