%{
    #include <stdio.h>
    void token(char* str) {
        printf("%s\n",str);
    }
%}

alpha [a-zA-Z]
digit [0-9]

%%
"const" { token("CONST"); }
"load" { token("LOAD"); }


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

