%{
    #include <stdio.h>
    #include <stdbool.h>
    #include "../Lex/lex.yy.c"
    int yyerror(const char*);
%}
%define parse.error verbose

%start program
%token IMPORT CONST LOAD SAVE PLAY FUNCTION IF OR OTHERWISE LOOP OVER UNTIL LONG INT FLOAT STRING AUDIO BOOL TRUE FALSE CONTINUE BREAK RETURN HIGHPASS LOWPASS EQ SIN COS EXP_DECAY LIN_DECAY SQUARE SAW TRIANGLE PAN TO MAIN
%token READ PRINT
%token SPEEDUP SPEEDDOWN LEQ GEQ EQUALS NOT_EQUALS LOGICAL_AND LOGICAL_OR POWER_EQUALS DISTORTION_EQUALS MULT_EQUALS DIVIDE_EQUALS MOD_EQUALS PLUS_EQUALS MINUS_EQUALS OR_EQUALS RIGHT_ARROW LEFT_ARROW IMPLIES
%token IDENTIFIER INT_LITERAL FLOAT_LITERAL STRING_LITERAL
%token INVALID_SYMBOL

%left LOGICAL_OR
%left LOGICAL_AND
%left NOT_EQUALS
%left EQUALS
%left GEQ
%left '>'
%left LEQ
%left '<'
%left '|'
%left '-'
%left '+'
%left SPEEDDOWN
%left SPEEDUP
%left '%'
%left '/'
%left '*'
%left '&'
%left '^'
%right '!'
%right '~'
%right LOAD

%%
program
    : statements main_block statements;

main_block
    : MAIN '{' statements '}';

import_statement
    : IMPORT STRING_LITERAL;

statements
    : statements statement
    | ;

statement 
    : declaration_statement ';'
    | assignment_statement ';'
    | function_call ';'
    | conditional_statement
    | loop_statement
    | load_statement ';'
    | play_statement ';' 
    | save_statement ';'
    | read_statement ';'
    | print_statement ';'
    | import_statement ';' {yyerror("Import statements can only be at the start of the file before any other statements.");};

read_statement
    : READ assignable_value;

print_statement
    : PRINT expr;

declaration_statement
    : IDENTIFIER LEFT_ARROW expr 
    | IDENTIFIER ':' data_type LEFT_ARROW expr 
    | CONST IDENTIFIER ':' data_type LEFT_ARROW expr ;

data_type
    : primitive_data_type 
    | '(' data_type_list ')'
    | '(' data_type_list ')' ':' data_type
    | error;

data_type_list: non_empty_data_type_list
              |;

non_empty_data_type_list: non_empty_data_type_list ',' data_type
                        | data_type;

primitive_data_type
    : primitive_data_type '[' INT_LITERAL ']'
    | primitive_data_type '[' ']'
    | INT
    | LONG
    | FLOAT
    | AUDIO
    | STRING
    | BOOL;

parameter_list
    : non_empty_parameter_list
    | ;

non_empty_parameter_list
    : non_empty_parameter_list ',' IDENTIFIER ':' data_type
    | IDENTIFIER ':' data_type;

returnable_statements
    : returnable_statements returnable_statement 
    | ;

returnable_statement
    : statement
    | return_statement ';' 
    | error
    | error ';' ;

return_statement
    : RETURN expr
    | RETURN ;
    
function_call
    : function_name function_arguments
    | '(' expr ')' function_arguments;

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
    | '(' argument_list ')'
    | error ;

argument_list
    : non_empty_argument_list
    | ;

non_empty_argument_list
    : non_empty_argument_list ',' expr
    | non_empty_argument_list ',' '_'
    | expr 
    | '_';

loop_statement
    : LOOP expr '{' loopable_statements '}'
    | LOOP UNTIL expr '{' loopable_statements '}'
    | LOOP OVER assignable_value expr TO expr '@' expr '{' loopable_statements '}'
    | LOOP OVER assignable_value expr TO expr  '{' loopable_statements '}';

loopable_statements
    : loopable_statements loopable_statement
    | ;

loopable_statement
    : statement
    | return_statement ';'
    | CONTINUE ';'
    | BREAK ';' 
    | error
    | error ';';

assignment_statement
    : assignable_value '=' expr
    | assignable_value PLUS_EQUALS expr
    | assignable_value MINUS_EQUALS expr
    | assignable_value MULT_EQUALS expr
    | assignable_value DIVIDE_EQUALS expr
    | assignable_value MOD_EQUALS expr
    | assignable_value OR_EQUALS expr
    | assignable_value POWER_EQUALS expr
    | assignable_value DISTORTION_EQUALS expr ;

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

load_statement
    : LOAD expr;

play_statement
    : PLAY expr;

save_statement
    : SAVE assignable_value RIGHT_ARROW expr ;

expr
    : '(' expr ')'
    | '(' parameter_list ')' ':' data_type IMPLIES expr ';'
    | '(' parameter_list ')' ':' data_type '{' returnable_statements '}'
    | '(' parameter_list ')' IMPLIES expr ';'
    | '(' parameter_list ')' '{' returnable_statements '}'
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
    | expr LOGICAL_OR expr
    | error ;

unary_expr
    : value
    | '~' expr
    | '!' expr
    | '+' expr
    | '-' expr;

value
    : INT_LITERAL
    | FLOAT_LITERAL
    | STRING_LITERAL
    | TRUE
    | FALSE
    | assignable_value
    | load_statement
    | function_call;

assignable_value
    : assignable_value '[' expr ']'
    | assignable_value '[' expr ':' expr ']'
    | IDENTIFIER ;


%%

char error_statement[1024];
long long error_line_no;
long long error_col_no;
bool is_error = false;

int yyerror(const char* s){
    if (is_error){
        printf("Line %lld, Column %lld: %s\n", error_line_no, error_col_no, error_statement);
        printf("%6lld | %s\n       | ", error_line_no, input_file[error_line_no-1]);
        for(int i = 0; i < error_col_no-1;i++) printf(" ");
        printf("\x1b[31m^\x1b[39m\n\n");
    }
    is_error = true;
    strncpy(error_statement, s, 1023);
    error_line_no = yylval.line_no;
    error_col_no = yylval.col_no;
    return 1;
}
int main() {
    yyparse();
    yyerror("");
    return 1;
}
