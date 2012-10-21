%{
#include <stdio.h>
#include "y.tab.h"
%}

INTEGER [0-9]+
TIME {INTEGER}"."{INTEGER}
WORD [a-zA-Z]+
FUNCTION (_|{WORD}|{INTEGER})+

%%
.
'\n' { return ENDLINE; }
<<EOF>> {return ENDFILE;}

%%



