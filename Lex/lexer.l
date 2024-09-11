%{
    #include <stdio.h>
    void token(char* str) {
        printf("%s\n",str);
    }
%}

%%
"const" { token("CONST"); }
"load" { token("LOAD"); }

\n { ;}
. { ;}
%%

int yywrap(void) {
    return 1;
}

int main(){
    yylex();
}

