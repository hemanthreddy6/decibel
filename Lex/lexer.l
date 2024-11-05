%{
    #include <stdio.h>
    #include <string.h>
    #include <string>
    #include <math.h>
    #include <stdlib.h>
    #include <vector>

    enum NodeType{
        NODE_ROOT,
        NODE_STATEMENTS,
        NODE_MAIN_BLOCK,
        NODE_READ_STATEMENT,
        NODE_PRINT_STATEMENT,
        NODE_DECLARATION_STATEMENT,
        NODE_DECLARATION_STATEMENT_WITH_TYPE,
        NODE_CONST_DECLARATION_STATEMENT,
        NODE_DATA_TYPE,
        NODE_INT_LITERAL,
        NODE_FLOAT_LITERAL,
        NODE_STRING_LITERAL,
        NODE_IDENTIFIER,
        NODE_PARAMETER_LIST,
        NODE_PARAMETER,
        NODE_RETURNABLE_STATEMENTS,
        NODE_RETURN_STATEMENT,
        NODE_FUNCTION_ARGUMENTS,
        NODE_ARGUMENT_LIST,
        NODE_OMITTED_PARAMETER,
        NODE_FUNCTION_CALL,
        NODE_NORMAL_ASSIGNMENT_STATEMENT,
        NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_OR_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_DISTORTION_EQUALS_ASSIGNMENT_STATEMENT,
        NODE_CONTINUE_STATEMENT,
        NODE_BREAK_STATEMENT,
        NODE_LOOP_REPEAT_STATEMENT,
        NODE_LOOP_UNTIL_STATEMENT,
        NODE_LOOP_GENERAL_STATEMENT,
        NODE_IF_STATEMENT,
        NODE_OR_STATEMENTS,
        NODE_OR_STATEMENT,
        NODE_OTHERWISE_STATEMENT,
        NODE_LOAD_STATEMENT,
        NODE_PLAY_STATEMENT,
        NODE_SAVE_STATEMENT,
        NODE_NORMAL_FUNCTION,
        NODE_INLINE_FUNCTION,
        NODE_UNARY_INVERSE_EXPR,
        NODE_UNARY_LOGICAL_NOT_EXPR,
        NODE_UNARY_PLUS_EXPR,
        NODE_UNARY_MINUS_EXPR,
        NODE_BOOL_LITERAL,
        NODE_INDEX,
        NODE_SLICE,
        NODE_POWER_EXPR,
        NODE_DISTORTION_EXPR,
        NODE_MULT_EXPR,
        NODE_DIVIDE_EXPR,
        NODE_MOD_EXPR,
        NODE_SPEEDUP_EXPR,
        NODE_SPEEDDOWN_EXPR,
        NODE_PLUS_EXPR,
        NODE_MINUS_EXPR,
        NODE_SUPERPOSITION_EXPR,
        NODE_LE_EXPR,
        NODE_LEQ_EXPR,
        NODE_GE_EXPR,
        NODE_GEQ_EXPR,
        NODE_EQUALS_EXPR,
        NODE_NOT_EQUALS_EXPR,
        NODE_LOGICAL_AND_EXPR,
        NODE_LOGICAL_OR_EXPR,
        NODE_ASSIGNABLE_VALUE,
        NODE_NOT_SET
    };


    #if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
    struct Stype{
        long long line_no;
        long long col_no;
        std::string text;
        enum NodeType node_type;
        std::vector<struct Stype*> children;
        struct DataType* data_type;

        Stype(): Stype(NODE_NOT_SET) {}

        Stype(enum NodeType type){
            node_type = type;
        }
    };
    #define YYSTYPE struct Stype *
    # define YYSTYPE_IS_TRIVIAL 1
    # define YYSTYPE_IS_DECLARED 1
    #endif

    #include "../Parser/y.tab.h"

    struct DataType{
        bool is_primitive;
        // if it's primitive:
        bool is_vector;
        int vector_size;
        struct DataType* vector_data_type;
        int basic_data_type; // INT, LONG, FLOAT, AUDIO, STRING, BOOL, or UNSET_DATA_TYPE
        // if it's not primitive:
        std::vector<struct DataType*> parameters;
        struct DataType* return_type;
        DataType(){
            is_primitive = true;
            is_vector = false;
            basic_data_type = UNSET_DATA_TYPE;
        }
        DataType(int type){
            is_primitive = true;
            is_vector = false;
            basic_data_type = type;
        }
        DataType(struct DataType* vector_type, int size){
            is_primitive = true;
            is_vector = true;
            vector_data_type = vector_type;
            vector_size = size;
        }
    } typedef DataType;

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
    long long col_no = 1;
    char line_buffer[32768];
    char *input_file[32768];
    void newline(){
        input_file[line_no-1] = (char*)malloc(col_no);
        if (!input_file[line_no-1]){
            printf("allocation failed??\n");
        }
        line_buffer[col_no-1] = '\0';
        strcpy(input_file[line_no-1], line_buffer);
        col_no = 1;
        line_no++;
        input_file[line_no-1] = line_buffer;
    }
    void count_newlines(char* s){
        for(; *s != '\0'; s++){
            if (*s == '\n')
                newline();
            else{
                line_buffer[col_no-1] = *s;
                line_buffer[col_no] = '\0';
                input_file[line_no-1] = line_buffer;
                col_no++;
            }
        }
    }
    struct Stype* token(char* str) {
        struct Stype* lval = new Stype();
        lval->line_no = line_no;
        lval->text = str;
        lval->col_no = col_no;
        count_newlines(str);
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
"if"            { yylval = token(yytext); return IF; }
"or"            { yylval = token(yytext); return OR; }
"otherwise"     { yylval = token(yytext); return OTHERWISE; }
"loop"          { yylval = token(yytext); return LOOP; }
"over"          { yylval = token(yytext); return OVER; }
"until"         { yylval = token(yytext); return UNTIL; }
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
"main"          { yylval = token(yytext); return MAIN; }
"read"          { yylval = token(yytext); return READ; }
"print"         { yylval = token(yytext); return PRINT; }

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

{alpha}({alpha}|{digit}|_)* { yylval = token(yytext); yylval->node_type = NODE_IDENTIFIER; return IDENTIFIER; }
{digit}+ { yylval = token(yytext);  yylval->node_type = NODE_INT_LITERAL; return INT_LITERAL; } 
({digit}*"."{digit}+|{digit}+"."{digit}*) { yylval = token(yytext); yylval->node_type = NODE_FLOAT_LITERAL; return FLOAT_LITERAL; }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(hz|HZ|hZ|Hz) { yylval = token(yytext); yylval->node_type = NODE_FLOAT_LITERAL; return FLOAT_LITERAL; }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(ms|Ms|MS|mS) { yylval = token(yytext); yylval->node_type = NODE_FLOAT_LITERAL; return FLOAT_LITERAL; }
({digit}*"."{digit}+|{digit}+"."{digit}*|{digit}+)(s|S) { yylval = token(yytext); yylval->node_type = NODE_FLOAT_LITERAL; return FLOAT_LITERAL; }
\"[^"]*\" { yylval = token(yytext); yylval->node_type = NODE_STRING_LITERAL; return STRING_LITERAL; }
\'[^']*\' { yylval = token(yytext); yylval->node_type = NODE_STRING_LITERAL; return STRING_LITERAL; }

\/\/(.*) {count_newlines(yytext);};
{multiline_comment_start}([^\*]|\*+[^\/\*])*{multiline_comment_end} {count_newlines(yytext);};

\n { newline();}
[\t ] { count_newlines(yytext);}
. { /*printf("Invalid Symbol on line %ld: %c\n", line_no, *yytext);*/ yylval = token(yytext); return INVALID_SYMBOL; }
%%

int yywrap(void) {
    return 1;
}
