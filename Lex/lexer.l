%{
    #include <stdio.h>
%}

alpha [a-zA-Z]
digit [0-9]

%%
const { printf("CONST\n");}


{alpha}({alpha}|{digit}|_)* { printf("IDENTIFIER\n");}
\n { ;}
. { ;}
%%

int yywrap(void) {
    return 1;
}

int main(){
    yylex();
}

