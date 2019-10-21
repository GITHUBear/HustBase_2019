%{
#include<string.h>
#include<stdio.h>
#include "yacc_sql.tab.h"
extern int atoi();
extern double atof();
char * position = "";

%}

/* Prevent the need for linking with -lfl */
%option noyywrap

WS      [\ \t\b\f]
DIGIT   [0-9]+

ID      [A-Za-z0-9]*[A-Za-z]+[A-Za-z0-9]*
DOT     \.
QUOTE   [\'\"]
%x STR

%%

{WS}                    /* ignore whitespace */;
\n																						 ;			

[\-]?{DIGIT}+					 { yylval.number=atoi(yytext); position=strdup(yytext); return NUMBER;}
[\-]?{DIGIT}+{DOT}{DIGIT}+					 { yylval.floats=(float)(atof(yytext)); position=strdup(yytext); return FLOAT;}

";"                 	 				 return SEMICOLON;
{DOT}                 						 return DOT;
"*"                   					 position=strdup(yytext);return STAR;
[Ee][Xx][Ii][Tt]						 position=strdup(yytext); return EXIT;
[Hh][Ee][Ll][Pp]                    	 position=strdup(yytext); return HELP;
[cC][Rr][Ee][Aa][Tt][Ee]                 position=strdup(yytext);return CREATE;
[Dd][Rr][Oo][Pp]                    	 position=strdup(yytext);return DROP;
[Tt][Aa][Bb][Ll][Ee]					 position=strdup(yytext);return TABLE;
[Ii][Nn][Dd][Ee][Xx]                  	 position=strdup(yytext);return INDEX;
[Oo][Nn]											position=strdup(yytext);return ON;
[Ss][Ee][Ll][Ee][Cc][Tt]                 position=strdup(yytext);return SELECT;
[Ff][Rr][Oo][Mm]                    	 position=strdup(yytext);return FROM;
[Ww][Hh][eE][rR][Ee]                  	 position=strdup(yytext);return WHERE;
[Aa][Nn][Dd]                     		 position=strdup(yytext);return AND;
[Ii][Nn][Ss][Ee][rR][tT]                 position=strdup(yytext);return INSERT;
[Ii][Nn][Tt][To]					 	 position=strdup(yytext);return INTO;
[Vv][Aa][Ll][Uu][Ee][Ss]                 position=strdup(yytext);return VALUES;
[Dd][Ee][Ll][Ee][Tt][Ee]                 position=strdup(yytext);return DELETE;
[Uu][Pp][Dd][Aa][Tt][Ee]                 position=strdup(yytext);return UPDATE;
[Ss][Ee][Tt]							 position=strdup(yytext);return SET;
[Ii][Nn][Tt]					{ position=strdup(yytext);return INT_T;}
[Cc][Hh][Aa][Rr]				{ position=strdup(yytext);return STRING_T;}
[Ff][Ll][Oo][Aa][Tt]			{ position=strdup(yytext);return FLOAT_T;}
{ID}							{ yylval.string=strdup(yytext);position=strdup(yytext);	return ID;}
"("								{ position=strdup(yytext);return LBRACE;}
")"								position=strdup(yytext);return RBRACE;

","                     position=strdup(yytext);return COMMA;
"="                     position=strdup(yytext);return EQ;
"<="                    position=strdup(yytext);return LE;
"<>"                    position=strdup(yytext);return NE;
"<"                     position=strdup(yytext);return LT;
">="                    position=strdup(yytext);return GE;
">"                     position=strdup(yytext);return GT;
{QUOTE}[\40\42\47a-zA-Z0-9]*{QUOTE}			{yylval.string=strdup(yytext);position = strdup(yytext);return SSS;}

.						 { printf("Unknown character [%c]\n",yytext[0]);  return yytext[0];    }
%%
