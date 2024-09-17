%{
    #include <stdio.h>
    #include <string.h>
    #include <math.h>
    #include "../Parser/y.tab.h"
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
"import"        { return IMPORT; }
"const"         { return CONST; }
"load"          { return LOAD; }
"save"          { return SAVE; }
"play"          { return PLAY; }
"function"      { return FUNCTION; }
"if"            { return IF; }
"or"            { return OR; }
"otherwise"     { return OTHERWISE; }
"loop"          { return LOOP; }
"over"          { return OVER; }
"long"          { return LONG; }
"int"           { return INT; }
"float"         { return FLOAT; }
"string"        { return STRING; }
"audio"         { return AUDIO; }
"bool"          { return BOOL; }
"true"          { return TRUE; }
"false"         { return FALSE; }
"continue"      { return CONTINUE; }
"break"         { return BREAK; }
"return"        { return RETURN; }
"HIGHPASS"      { return HIGHPASS; }
"LOWPASS"       { return LOWPASS; }
"EQ"            { return EQ; }
"SIN"           { return SIN; }
"COS"           { return COS; }
"EXP_DECAY"     { return EXP_DECAY; }
"LIN_DECAY"     { return LIN_DECAY; }
"SQUARE"        { return SQUARE; }
"SAW"           { return SAW; }
"TRIANGLE"      { return TRIANGLE; }
"PAN"           { return PAN; }
"to"            { return TO; }

"~" { return '~'; }
"!" { return '!'; }

"^" { return '^'; }

"&" { return '&'; }

"*" { return '*'; }
"/" { return '/'; }
"%" { return '%'; }
">>" { return SPEEDUP; }
"<<" { return SPEEDDOWN; }

"+" { return '+'; }
"-" { return '-'; }
"|" { return '|'; }

"<" { return '<'; }
"<=" { return LEQ; }
">" { return '>'; }
">=" { return GEQ; }
"==" { return EQUALS; }
"!=" { return NOT_EQUALS; }

"&&" { return LOGICAL_AND; }
"||" { return LOGICAL_OR; }

"=" { return '='; }
"^=" { return POWER_EQUALS; }
"&=" { return DISTORTION_EQUALS; }
"*=" { return MULT_EQUALS; }
"/=" { return DIVIDE_EQUALS; }
"%=" { return MOD_EQUALS; }
"+=" { return PLUS_EQUALS; }
"-=" { return MINUS_EQUALS; }
"|=" { return OR_EQUALS; }
"->" { return RIGHT_ARROW; }
"<-" { return LEFT_ARROW; } 

";" { return ';'; }
"{" { return '{'; }
"}" { return '}'; }
"[" { return '['; }
"]" { return ']'; }
"(" { return '('; }
")" { return ')'; }
":" { return ':'; }
"@" { return '@'; }
"_" { return '_'; }
"," { return ','; }
"=>" { return IMPLIES; }

{alpha}({alpha}|{digit}|_)* { return IDENTIFIER;}
{digit}+ { return INT_LITERAL;/* printf("%ld\n", atol(yytext));*/} 
({digit}*"."{digit}+|{digit}+"."{digit}*) { return FLOAT_LITERAL;/* double var; sscanf(yytext, "%lf", &var); printf("%lf\n", var);*/}
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(hz|HZ|hZ|Hz) { return FLOAT_LITERAL; /*printf("%lf\n", frequency_to_semitones(yytext));*/}
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(ms|Ms|MS|mS) { return FLOAT_LITERAL; /*printf("%lf\n", milliseconds_to_float(yytext));*/ }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(s|S) { return FLOAT_LITERAL; /*printf("%lf\n", seconds_to_float(yytext));*/ }
\"[^"]*\" { return STRING_LITERAL; /*printf("%s\n", yytext);*/}
\'[^']*\' { return STRING_LITERAL; /*printf("%s\n", yytext);*/}

\/\/(.*)(\n)? ;
{multiline_comment_start}([^\*]|\*+[^\/\*])*{multiline_comment_end}(\n)? ;

\n { ; }
[\t ] { ; }
. { return INVALID_SYMBOL; }
%%

int yywrap(void) {
    return 1;
}