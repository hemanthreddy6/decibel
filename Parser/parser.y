%{
    #include <stdio.h>
    #include "../Lex/lex.yy.c"
    int yyerror(const char*);
%}
%define parse.error verbose

%start program
%token IMPORT CONST LOAD SAVE PLAY FUNCTION IF OR OTHERWISE LOOP OVER LONG INT FLOAT STRING AUDIO BOOL TRUE FALSE CONTINUE BREAK RETURN HIGHPASS LOWPASS EQ SIN COS EXP_DECAY LIN_DECAY SQUARE SAW TRIANGLE PAN TO
%token SPEEDUP SPEEDDOWN LEQ GEQ EQUALS NOT_EQUALS LOGICAL_AND LOGICAL_OR POWER_EQUALS DISTORTION_EQUALS MULT_EQUALS DIVIDE_EQUALS MOD_EQUALS PLUS_EQUALS MINUS_EQUALS OR_EQUALS RIGHT_ARROW LEFT_ARROW IMPLIES
%token IDENTIFIER INT_LITERAL FLOAT_LITERAL STRING_LITERAL
%token INVALID_SYMBOL

%right LOAD
%right '~'
%right '!'
%left '^'
%left '&'
%left '*'
%left '/'
%left '%'
%left SPEEDUP
%left SPEEDDOWN
%left '+'
%left '-'
%left '|'
%left '<'
%left LEQ
%left '>'
%left GEQ
%left EQUALS
%left NOT_EQUALS
%left LOGICAL_AND
%left LOGICAL_OR

%%
program
    : import_statements statements {/* anything that does not have a rule or the rule is empty, please add stuff */};

import_statements
    : import_statements import_statement ';'
    | ;

returnable_statements
    : returnable_statements returnable_statement 
    | ;

returnable_statement
    : statement
    | return_statement ';' ;

loopable_statements
    : loopable_statements loopable_statement
    | ;

loopable_statement
    : statement
    | CONTINUE ';'
    | BREAK ';' ;

statement 
    : declaration_statement ';'
    | function_declaration
    | assignment_statement ';'
    | function_call ';'
    | conditional_statement
    | loop_statement
    | load_statement ';'
    | play_statement ';' 
    | save_statement ';' ;

statements
    : statements statement
    | ;

import_statement
    : IMPORT STRING_LITERAL;

declaration_statement
    : IDENTIFIER LEFT_ARROW expr 
    | IDENTIFIER ':' data_type LEFT_ARROW expr 
    | CONST IDENTIFIER ':' data_type LEFT_ARROW expr ;

data_type
    : INT
    | LONG
    | FLOAT
    | AUDIO
    | STRING
    | BOOL;

function_declaration
    : FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' ':' data_type '{' returnable_statements '}'
    | FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' ':' data_type IMPLIES expr ';'
    | FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' '{' returnable_statements '}'
    | FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' IMPLIES expr ';'
    | FUNCTION IDENTIFIER LEFT_ARROW expr ';';

parameter_list
    : non_empty_parameter_list
    | ;

non_empty_parameter_list
    : non_empty_parameter_list ',' IDENTIFIER ':' data_type
    | IDENTIFIER ':' data_type;

assignment_statement
    : IDENTIFIER '=' expr
    | IDENTIFIER PLUS_EQUALS expr
    | IDENTIFIER MINUS_EQUALS expr
    | IDENTIFIER MULT_EQUALS expr
    | IDENTIFIER DIVIDE_EQUALS expr
    | IDENTIFIER MOD_EQUALS expr
    | IDENTIFIER OR_EQUALS expr
    | IDENTIFIER POWER_EQUALS expr
    | IDENTIFIER DISTORTION_EQUALS expr ;

return_statement
    : RETURN expr
    | RETURN ;

conditional_statement
    : IF expr '{' loopable_statements '}' or_statements otherwise_statement;

or_statements
    : or_statement or_statements
    | ;

or_statement
    : OR expr '{' loopable_statements '}';

otherwise_statement
    : OTHERWISE '{' loopable_statements '}'
    | ;

loop_statement
    : LOOP expr '{' loopable_statements '}'
    | LOOP OVER IDENTIFIER expr TO expr '@' expr '{' loopable_statements '}'
    | LOOP OVER IDENTIFIER expr TO expr  '{' loopable_statements '}';

load_statement
    : LOAD expr;

play_statement
    : PLAY expr;

save_statement
    : SAVE IDENTIFIER RIGHT_ARROW expr ;

expr
    : '(' expr ')'
    | unary_expr
    | expr '^' expr
    | expr '&' expr
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | expr SPEEDUP expr
    | expr SPEEDDOWN expr
    | expr '+' expr
    | expr '-' expr
    | expr '|' expr
    | expr '<' expr
    | expr LEQ expr
    | expr '>' expr
    | expr GEQ expr
    | expr EQUALS expr
    | expr NOT_EQUALS expr
    | expr LOGICAL_AND expr
    | expr LOGICAL_OR expr;

unary_expr
    : value
    | '~' value
    | '!' value
    | '+' value
    | '-' value

value
    : INT_LITERAL
    | FLOAT_LITERAL
    | STRING_LITERAL
    | IDENTIFIER
    | load_statement
    | function_call
    | IDENTIFIER '[' expr ':' expr ']'

function_call
    : function_name function_arguments

function_name
    : IDENTIFIER
    | AUDIO
    | HIGHPASS
    | LOWPASS 
    | EQ 
    | SIN 
    | COS 
    | EXP_DECAY 
    | LIN_DECAY 
    | SQUARE 
    | SAW 
    | TRIANGLE 
    | PAN ;

function_arguments
    : function_arguments '(' argument_list ')'
    | '(' argument_list ')';

argument_list
    : non_empty_argument_list
    | ;

non_empty_argument_list
    : non_empty_argument_list ',' expr
    | non_empty_argument_list ',' '_'
    | expr 
    | '_';

%%

int yyerror(const char* s){
    printf("Line %lld: %s\n", yylval.line_no, s);
    return 1;
}
int main() {
    yyparse();
    return 1;
}
