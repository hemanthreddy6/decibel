%{
    #include <stdio.h>
%}

%%
const { printf("CONST\n");}

\n { ;}
. { ;}
%%

int yywrap(void) {
    return 1;
}

int main(){
    yylex();
}

