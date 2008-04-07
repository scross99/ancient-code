%{
#include "tree.h"
#include "parse.tab.h"
#include <iostream>
#include <string.h>
std::string str_text = "";
int yyerror(char *s);
extern int linenum;
%}

%x STRING_STATE
%x COMMENT_STATE

%%

"\""                    { str_text = ""; BEGIN(STRING_STATE); }

<STRING_STATE>"\\\""          { str_text += "\""; }
<STRING_STATE>"\\\\"          { str_text += "\\"; }
<STRING_STATE>"\\n"           { str_text += "\n"; }
<STRING_STATE>"\\t"           { str_text += "\t"; }
<STRING_STATE>"\""            { yylval.str = new char[str_text.length()+1];
								yylval.str[str_text.length()] = 0;
								str_text.copy(yylval.str, str_text.length());
								BEGIN(INITIAL);
								return STRING; }
<STRING_STATE>.               { str_text += yytext; }

"/*"                    { BEGIN(COMMENT_STATE); }

<COMMENT_STATE>[^*\n]*        { }
<COMMENT_STATE>"*"+[^*/\n]*   { }
<COMMENT_STATE>\n             { linenum++; }
<COMMENT_STATE>"*"+"/"        { BEGIN(INITIAL); }

if                      { return IF; }
else                    { return ELSE; }
while                   { return WHILE; }
for                     { return FOR; }
break                   { return BREAK; }
continue                { return CONTINUE; }
return                  { return RETURN; }
global                  { return GLOBAL; }

int                     { return T_INT; }
bool                    { return T_BOOL; }
string                  { return T_STRING; }
float                   { return T_FLOAT; }

true                    { yylval.ival = 1; return INT; }
false                   { yylval.ival = 0; return INT; }
TRUE                    { yylval.ival = 1; return INT; }
FALSE                   { yylval.ival = 0; return INT; }

null                    { yylval.ival = 0; return INT; }
NULL                    { yylval.ival = 0; return INT; }

[0-9]+                  { yylval.ival = atoi(yytext); return INT; }
[0-9]+\.[0-9]+          { yylval.dval = atof(yytext); return DOUBLE; }
[A-Za-z_][0-9A-Za-z_]*   { yylval.str = strdup(yytext); return NAME; }
$[A-Za-z_][0-9A-Za-z_]*  { yylval.str = strdup(yytext); return NAME; }



"+="                    { return ADDEQUAL; }
"-="                    { return MINUSEQUAL; }
"*="                    { return MULEQUAL; }
"/="                    { return DIVEQUAL; }

"=="                    { return IS_EQUAL; }
"!="                    { return NOT_EQUAL; }
">="                    { return GREATEROREQUAL; }
"<="                    { return LESSOREQUAL; }

"++"                    { return INCREMENT; }
"--"                    { return DECREMENT; }
"&&"                    { return LOG_AND; }
"||"                    { return LOG_OR; }

\/\/(.*?)\n             { linenum++; } //do nothing for comments

;                       { return ';'; }
\n                      { linenum++; }
[ \t\r]+                { } //do nothing for whitespace
.                       { return yytext[0]; }

%%


