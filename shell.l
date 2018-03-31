/* 
 * shell.l: simple lexical analyzer for the shell. 
 */ 
%{ 
#include <string.h> 
#include "y.tab.h" 
%} 
%% 
\n      { 
 			return NEWLINE;   
} 
[ \t]   { 
               ;  /* Discard spaces and tabs */ 
        } 
">"     { 
return GREAT;
        } 
"<"     { 
return LESS; 
        } 
">>"    { 
return GREATGREAT;
        } 
">&"    { 
return GREATAMPERSAND;
        } 
"|"     { 
return PIPE; 
        } 
"&"     { 
return AMPERSAND; 
        } 
[^ \t\n][^ \t\n]*  { 
                /* Assume that file names have only alpha chars */ 
	yylval = strdup(yytext);
return WORD;
        } 
 
. { 
                /* Invalid character in input */ 
	return NOTOKEN;
	} 
%%

int yywrap()
{
	return 1;
}

