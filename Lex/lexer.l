%{
    #include <stdio.h>
    void token(char* str) {
        printf("%s\n",str);
    }
%}

%%
"import"        { token("IMPORT"); }
"const"         { token("CONST"); }
"load"          { token("LOAD"); }
"save"          { token("SAVE"); }
"play"          { token("PLAY"); }
"function"      { token("FUNCTION"); }
"if"            { token("IF"); }
"or"            { token("OR"); }
"otherwise"     { token("OTHERWISE"); }
"loop"          { token("LOOP"); }
"over"          { token("OVER"); }
"long"          { token("LONG"); }
"int"           { token("INT"); }
"float"         { token("FLOAT"); }
"string"        { token("STRING"); }
"audio"         { token("AUDIO"); }
"bool"          { token("BOOL"); }
"true"          { token("TRUE"); }
"false"         { token("FALSE"); }
"continue"      { token("CONTINUE"); }
"break"         { token("BREAK"); }
"return"        { token("RETURN"); }
"HIGHPASS"      { token("HIGHPASS"); }
"LOWPASS"       { token("LOWPASS"); }
"EQ"            { token("EQ"); }
"SIN"           { token("SIN"); }
"COS"           { token("COS"); }
"EXP_DECAY"     { token("EXP_DECAY"); }
"LIN_DECAY"     { token("LIN_DECAY"); }
"SQUARE"        { token("SQUARE"); }
"SAW"           { token("SAW"); }
"TRIANGLE"      { token("TRIANGLE"); }
"PAN"           { token("PAN"); }

"~" { token("~"); }
"!" { token("!"); }

"^" { token("^"); }

"&" { token("&"); }

"*" { token("*"); }
"/" { token("/"); }
"%" { token("\%"); }
">>" { token(">>"); }
"<<" { token("<<"); }

"+" { token("+"); }
"-" { token("-"); }
"|" { token("|"); }

"<" { token("<"); }
"<=" { token("<="); }
">" { token(">"); }
">=" { token(">="); }
"==" { token("=="); }
"!=" { token("!="); }

"&&" { token("&&"); }
"||" { token("||"); }

"=" { token("="); }
"^=" { token("^="); }
"&=" { token("&="); }
"*=" { token("*="); }
"/=" { token("/="); }
"%=" { token("\%="); }
"+=" { token("+="); }
"-=" { token("-="); }
"|=" { token("|="); }
"->" { token("->"); }
"<-" { token("<-"); } 

";" { token(";"); }
\n { ; }
. { ; }
%%

int yywrap(void) {
    return 1;
}

int main(){
    yylex();
}

