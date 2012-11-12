%{
	#include <stdio.h>
	#include "y.tab.h"
%}

/******************************************************************************/

/* Definitions */
integer		[0-9]+
time		{integer}"."{integer}
word		[a-zA-Z]+
function 	(_|{word}|{integer})+

/******************************************************************************/

/* Rules */

%%
{time}		{ yylval.dval = atof (yytext); return REALNUMBER; }
{integer}	{ yylval.ival = atoi (yytext); return INTEGER; }
"| CALL   | FUNCTION NAME                            |"
" INC TIME (S) | ENTRY CYCLE        | EXIT CYCLE         |"
			{}
"Program total execution time (s):"
			{}
"LOAD"		{ return LOAD; }
"STORE"		{ return STORE; }
"N"			{ return UNKNOWN; }
{function}	{ yylval.str = strdup (yytext); return FUNCTIONNAME; }
"*"			{ return FACT; }
"\n"		{ return ENDLINE; }
[ \t\|\-\+]
%%

/******************************************************************************/

/* Functions */