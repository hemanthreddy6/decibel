%{
    #include <stdio.h>
    #include <string.h>
    #include <math.h>
    void token(char* str) {
        printf("%s\n",str);
    }
    double frequency_to_semitones(char* str) {
        size_t len = strlen(str);
        str[len-2] = '\0';
        double freq;
        sscanf(str, "%lf", &freq);
        double semitones = 12 * log2(freq/440);
        return semitones;
    }
%}

alpha [a-zA-Z]
digit [0-9]

%%
"const" { token("CONST"); }
"load" { token("LOAD"); }


{alpha}({alpha}|{digit}|_)* { token("IDENTIFIER");}
{digit}+ { token("INT"); printf("%ld\n", atol(yytext));} 
({digit}*"."{digit}+|{digit}+"."{digit}*) { token("FLOAT"); printf("%f\n", atof(yytext));}
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(hz|HZ|hZ|Hz) { token("FLOAT"); printf("%lf\n", frequency_to_semitones(yytext));}
\"[^"]*\" { token("STRING"); printf("%s\n", yytext);}
\'[^']*\' { token("STRING"); printf("%s\n", yytext);}
\n { ;}
. { ;}
%%

int yywrap(void) {
    return 1;
}

int main(){
    yylex();
}

