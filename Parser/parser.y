%{
    #include <stdio.h>
    #include "../Lex/lex.yy.c"
    int yyerror(char*);
%}

%start program
%token IMPORT CONST LOAD SAVE PLAY FUNCTION IF OR OTHERWISE LOOP OVER LONG INT FLOAT STRING AUDIO BOOL TRUE FALSE CONTINUE BREAK RETURN HIGHPASS LOWPASS EQ SIN COS EXP_DECAY LIN_DECAY SQUARE SAW TRIANGLE PAN TO
%token SPEEDUP SPEEDDOWN LEQ GEQ EQUALS NOT_EQUALS LOGICAL_AND LOGICAL_OR POWER_EQUALS DISTORTION_EQUALS MULT_EQUALS DIVIDE_EQUALS MOD_EQUALS PLUS_EQUALS MINUS_EQUALS OR_EQUALS RIGHT_ARROW LEFT_ARROW IMPLIES
%token IDENTIFIER INT_LITERAL FLOAT_LITERAL STRING_LITERAL
%token INVALID_SYMBOL

%%
program 
    : statements {/* anything that does not have a rule or the rule is empty, please add stuff */};

statement
    : import_statement
    | declaration_statement
    | function_declaration
    | assignment_statement
    | return_statement
    | conditional_statement
    | loop_statement
    | load_statement
    | play_statement
    | save_statement;

statements
    : statements statement
    | statement;

import_statement
    : IMPORT STRING_LITERAL ';'

declaration_statement
    : IDENTIFIER LEFT_ARROW expr ';'
    | IDENTIFIER ':' data_type LEFT_ARROW expr ';'
    | CONST IDENTIFIER ':' data_type LEFT_ARROW expr ';';

data_type
    : INT
    | LONG
    | FLOAT
    | AUDIO
    | STRING
    | BOOL;

function_declaration
    : FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' ':' data_type '{' statements '}'
    | FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' ':' data_type IMPLIES expr ';'
    | FUNCTION IDENTIFIER LEFT_ARROW expr ';';

parameter_list
    : non_empty_parameter_list
    | ;

non_empty_parameter_list
    : non_empty_parameter_list ',' IDENTIFIER ':' data_type
    | IDENTIFIER ':' data_type;

inbuilt_functions
    : highpass_function
    | lowpass_function
    | eq_function
    | sin_function
    | cos_functino
    | exp_decay_function
    | lin_decay_function
    | square_function
    | saw_function
    | triangle_function
    | pan_function;

expr_that_returns_function:;

expr:;

assignment_statement
    : IDENTIFIER '=' expr ';'
    | IDENTIFIER PLUS_EQUALS expr ';'
    | IDENTIFIER MINUS_EQUALS expr ';'
    | IDENTIFIER MULT_EQUALS expr ';'
    | IDENTIFIER DIVIDE_EQUALS expr ';'
    | IDENTIFIER MOD_EQUALS expr ';'
    | IDENTIFIER OR_EQUALS expr ';'
    | IDENTIFIER POWER_EQUALS expr ';'
    | IDENTIFIER DISTORTION_EQUALS expr ';';

return_statement
    : RETURN expr ';' ;
    | expr ;

conditional_statement
    : IF expr '{' statements '}' or_statements otherwise_statement;

or_statements
    : or_statement or_statements
    | ;

or_statement
    : OR expr '{' statements '}';

otherwise_statement
    : OTHERWISE expr '{' statements '}'
    | ;

loop_statement
    : LOOP expr '{' statements '}'
    | LOOP OVER IDENTIFIER expr TO expr '@' expr '{' statements '}'
    | LOOP OVER IDENTIFIER expr TO expr  '{' statements '}';

load_statement
    : LOAD expr;

play_statement
    : PLAY IDENTIFIER ';';

save_statement
    : SAVE IDENTIFIER RIGHT_ARROW STRING_LITERAL ';';
%%

int yyerror( char* s){
    printf("%s", s);
    return 1;
}
int main() {
    yyparse();
    return 1;
}
