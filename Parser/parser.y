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
program: statements {/* anything that does not have a rule or the rule is empty, please add stuff */};
statement: declaration_statement ';'
         | function_declaration 
         | assignment_statement ';'
         | return_statement ';'
         | conditional_statement
         | loop_statement
         | load_statement ';'
         | play_statement ';' 
         | save_statement ';' ;
statements: statements statement
          | statement;
declaration_statement: IDENTIFIER LEFT_ARROW expr 
                     | IDENTIFIER ':' data_type LEFT_ARROW expr 
                     | CONST IDENTIFIER ':' data_type LEFT_ARROW expr ;
data_type: INT
         | LONG
         | FLOAT
         | AUDIO
         | STRING
         | BOOL;
function_declaration: FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' ':' data_type '{' statements '}'
                    | FUNCTION IDENTIFIER LEFT_ARROW '(' parameter_list ')' ':' data_type IMPLIES expr ';'
                    | FUNCTION IDENTIFIER LEFT_ARROW expr ';';
parameter_list: non_empty_parameter_list
              | ;
non_empty_parameter_list: non_empty_parameter_list ',' IDENTIFIER ':' data_type
              | IDENTIFIER ':' data_type;
inbuilt_functions: highpass_function
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
highpass_function: HIGHPASS '(' expr ',' expr ')'
expr_that_returns_function:;
expr:;
assignment_statement:;
return_statement: RETURN expr
                | RETURN ;
conditional_statement: IF expr '{' statements '}'
                    | IF expr '{' statements '}' OR '{' statements '}'
                    | IF expr '{' statements '}' OR '{' statements '}' OTHERWISE '{'statements '}'
                    | IF expr '{' statements '}' OTHERWISE '{'statements '}' ;
loop_statement: LOOP expr '{' statements '}'
              | LOOP OVER IDENTIFIER expr TO expr @expr '{' statements '}'
              | LOOP OVER IDENTIFIER expr TO expr  '{' statements '}';
load_statement: LOAD expr;
play_statement: PLAY expr;
%%

int yyerror( char* s){
    printf("%s", s);
    return 1;
}
int main() {
    yyparse();
    return 1;
}
