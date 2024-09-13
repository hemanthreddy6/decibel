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
    double milliseconds_to_float(char* str) {
        size_t len = strlen(str);
        str[len-2] = '\0';
        double ms;
        sscanf(str, "%lf", &ms);
        return ms/1000;
    }
    double seconds_to_float(char* str) {
        size_t len = strlen(str);
        str[len-1] = '\0';
        double ms;
        sscanf(str, "%lf", &ms);
        return ms;
    }
%}

alpha [a-zA-Z]
digit [0-9]
multiline_comment_start \/\*
multiline_comment_end \*+\/

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
"to"            { token("TO"); }

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
"{" { token("{"); }
"}" { token("}"); }
"[" { token("["); }
"]" { token("]"); }
"(" { token("("); }
")" { token(")"); }
":" { token(":"); }
"@" { token("@"); }
"_" { token("_"); }

{alpha}({alpha}|{digit}|_)* { token("IDENTIFIER");}
{digit}+ { token("INT");/* printf("%ld\n", atol(yytext));*/} 
({digit}*"."{digit}+|{digit}+"."{digit}*) { token("FLOAT");/* double var; sscanf(yytext, "%lf", &var); printf("%lf\n", var);*/}
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(hz|HZ|hZ|Hz) { token("FLOAT"); /*printf("%lf\n", frequency_to_semitones(yytext));*/}
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(ms|Ms|MS|mS) { token("FLOAT"); /*printf("%lf\n", milliseconds_to_float(yytext));*/ }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(s|S) { token("FLOAT"); /*printf("%lf\n", seconds_to_float(yytext));*/ }
\"[^"]*\" { token("STRING"); /*printf("%s\n", yytext);*/}
\'[^']*\' { token("STRING"); /*printf("%s\n", yytext);*/}

\/\/(.*)(\n)? ;
{multiline_comment_start}([^\*]|\*+[^\/\*])*{multiline_comment_end}(\n)? ;

\n { ; }
. { ; }
%%

int yywrap(void) {
    return 1;
}

int main(){
    yylex();
}

