%{
    #include <stdio.h>
    #include <string.h>
    #include <math.h>
    #if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
    struct Stype{
        long long line_no;
        char* text;
    };
    typedef struct Stype  YYSTYPE;
    # define YYSTYPE_IS_TRIVIAL 1
    # define YYSTYPE_IS_DECLARED 1
    #endif
    #include "../Parser/y.tab.h"
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
    long long line_no = 1;
    void count_newlines(const char* s){
        for(int i = 0; i < strlen(s); i++){
            if (s[i] == '\n')
                line_no++;
        }
    }
    struct Stype token(char* str) {
        struct Stype lval;
        lval.line_no = line_no;
        lval.text = str;
        return lval;
    }
%}

alpha [a-zA-Z]
digit [0-9]
multiline_comment_start \/\*
multiline_comment_end \*+\/

%%
"import"        { yylval = token(yytext); return IMPORT; }
"const"         { yylval = token(yytext); return CONST; }
"load"          { yylval = token(yytext); return LOAD; }
"save"          { yylval = token(yytext); return SAVE; }
"play"          { yylval = token(yytext); return PLAY; }
"function"      { yylval = token(yytext); return FUNCTION; }
"if"            { yylval = token(yytext); return IF; }
"or"            { yylval = token(yytext); return OR; }
"otherwise"     { yylval = token(yytext); return OTHERWISE; }
"loop"          { yylval = token(yytext); return LOOP; }
"over"          { yylval = token(yytext); return OVER; }
"long"          { yylval = token(yytext); return LONG; }
"int"           { yylval = token(yytext); return INT; }
"float"         { yylval = token(yytext); return FLOAT; }
"string"        { yylval = token(yytext); return STRING; }
"audio"         { yylval = token(yytext); return AUDIO; }
"bool"          { yylval = token(yytext); return BOOL; }
"true"          { yylval = token(yytext); return TRUE; }
"false"         { yylval = token(yytext); return FALSE; }
"continue"      { yylval = token(yytext); return CONTINUE; }
"break"         { yylval = token(yytext); return BREAK; }
"return"        { yylval = token(yytext); return RETURN; }
"HIGHPASS"      { yylval = token(yytext); return HIGHPASS; }
"LOWPASS"       { yylval = token(yytext); return LOWPASS; }
"EQ"            { yylval = token(yytext); return EQ; }
"SIN"           { yylval = token(yytext); return SIN; }
"COS"           { yylval = token(yytext); return COS; }
"EXP_DECAY"     { yylval = token(yytext); return EXP_DECAY; }
"LIN_DECAY"     { yylval = token(yytext); return LIN_DECAY; }
"SQUARE"        { yylval = token(yytext); return SQUARE; }
"SAW"           { yylval = token(yytext); return SAW; }
"TRIANGLE"      { yylval = token(yytext); return TRIANGLE; }
"PAN"           { yylval = token(yytext); return PAN; }
"to"            { yylval = token(yytext); return TO; }

"~" { yylval = token(yytext); return '~'; }
"!" { yylval = token(yytext); return '!'; }

"^" { yylval = token(yytext); return '^'; }

"&" { yylval = token(yytext); return '&'; }

"*" { yylval = token(yytext); return '*'; }
"/" { yylval = token(yytext); return '/'; }
"%" { yylval = token(yytext); return '%'; }
">>" { yylval = token(yytext); return SPEEDUP; }
"<<" { yylval = token(yytext); return SPEEDDOWN; }

"+" { yylval = token(yytext); return '+'; }
"-" { yylval = token(yytext); return '-'; }
"|" { yylval = token(yytext); return '|'; }

"<" { yylval = token(yytext); return '<'; }
"<=" { yylval = token(yytext); return LEQ; }
">" { yylval = token(yytext); return '>'; }
">=" { yylval = token(yytext); return GEQ; }
"==" { yylval = token(yytext); return EQUALS; }
"!=" { yylval = token(yytext); return NOT_EQUALS; }

"&&" { yylval = token(yytext); return LOGICAL_AND; }
"||" { yylval = token(yytext); return LOGICAL_OR; }

"=" { yylval = token(yytext); return '='; }
"^=" { yylval = token(yytext); return POWER_EQUALS; }
"&=" { yylval = token(yytext); return DISTORTION_EQUALS; }
"*=" { yylval = token(yytext); return MULT_EQUALS; }
"/=" { yylval = token(yytext); return DIVIDE_EQUALS; }
"%=" { yylval = token(yytext); return MOD_EQUALS; }
"+=" { yylval = token(yytext); return PLUS_EQUALS; }
"-=" { yylval = token(yytext); return MINUS_EQUALS; }
"|=" { yylval = token(yytext); return OR_EQUALS; }
"->" { yylval = token(yytext); return RIGHT_ARROW; }
"<-" { yylval = token(yytext); return LEFT_ARROW; } 

";" { yylval = token(yytext); return ';'; }
"{" { yylval = token(yytext); return '{'; }
"}" { yylval = token(yytext); return '}'; }
"[" { yylval = token(yytext); return '['; }
"]" { yylval = token(yytext); return ']'; }
"(" { yylval = token(yytext); return '('; }
")" { yylval = token(yytext); return ')'; }
":" { yylval = token(yytext); return ':'; }
"@" { yylval = token(yytext); return '@'; }
"_" { yylval = token(yytext); return '_'; }
"," { yylval = token(yytext); return ','; }
"=>" { yylval = token(yytext); return IMPLIES; }

{alpha}({alpha}|{digit}|_)* { yylval = token(yytext); return IDENTIFIER; }
{digit}+ { yylval = token(yytext); return INT_LITERAL; } 
({digit}*"."{digit}+|{digit}+"."{digit}*) { yylval = token(yytext); return FLOAT_LITERAL; }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(hz|HZ|hZ|Hz) { yylval = token(yytext); return FLOAT_LITERAL; }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(ms|Ms|MS|mS) { yylval = token(yytext); return FLOAT_LITERAL; }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(s|S) { yylval = token(yytext); return FLOAT_LITERAL; }
\"[^"]*\" { count_newlines(yytext); yylval = token(yytext); return STRING_LITERAL; }
\'[^']*\' { count_newlines(yytext); yylval = token(yytext); return STRING_LITERAL; }

\/\/(.*) ;
{multiline_comment_start}([^\*]|\*+[^\/\*])*{multiline_comment_end} ;

\n { line_no++; }
[\t ] { ; }
. { /*printf("Invalid Symbol on line %ld: %c\n", line_no, *yytext);*/ yylval = token(yytext); return INVALID_SYMBOL; }
%%

int yywrap(void) {
    return 1;
}
