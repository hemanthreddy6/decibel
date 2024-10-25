%{
    #include <stdio.h>
    #include <stdbool.h>
    #include "../Lex/lex.yy.c"
    int yyerror(const char*);

    YYSTYPE root = new Stype(NODE_ROOT);
%}
%define parse.error verbose

%start program
%token IMPORT CONST LOAD SAVE PLAY IF OR OTHERWISE LOOP OVER UNTIL LONG INT FLOAT STRING AUDIO BOOL TRUE FALSE CONTINUE BREAK RETURN HIGHPASS LOWPASS EQ SIN COS EXP_DECAY LIN_DECAY SQUARE SAW TRIANGLE PAN TO MAIN
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
    : statements main_block statements { 
        root->children.push_back($1);
        root->children.push_back($2); 
        root->children.push_back($3); };

main_block
    : MAIN '{' statements '}' {
        $$ = new Stype(NODE_MAIN_BLOCK); 
        $$->children.push_back($3); };

import_statement
    : IMPORT STRING_LITERAL;

statements
    : statements statement {
        $$ = $1; 
        $$->children.push_back($2); }
    | { $$ = new Stype(NODE_STATEMENTS); };

statement 
    : declaration_statement ';' {$$ = $1;}
    | assignment_statement ';' {$$ = $1;}
    | function_call ';' {$$ = $1;}
    | conditional_statement {$$ = $1;}
    | loop_statement {$$ = $1;}
    | load_statement ';' {$$ = $1;}
    | play_statement ';'  {$$ = $1;}
    | save_statement ';' {$$ = $1;}
    | read_statement ';' {$$ = $1;}
    | print_statement ';' {$$ = $1;}
    | import_statement ';' {yyerror("Import statements can only be at the start of the file before any other statements.");};

read_statement
    : READ assignable_value {
        $$ = new Stype(NODE_READ_STATEMENT); 
        $$->children.push_back($2); };

print_statement
    : PRINT expr {
        $$ = new Stype(NODE_PRINT_STATEMENT); 
        $$->children.push_back($2); };

declaration_statement
    : IDENTIFIER LEFT_ARROW expr {
        $$ = new Stype(NODE_DECLARATION_STATEMENT); 
        $$->children.push_back($1); 
        $$->children.push_back($3); }
    | IDENTIFIER ':' data_type LEFT_ARROW expr {
        $$ = new Stype(NODE_DECLARATION_STATEMENT_WITH_TYPE); 
        $$->children.push_back($1); 
        $$->children.push_back($5); 
        $$->children.push_back($3); }
    | CONST IDENTIFIER ':' data_type LEFT_ARROW expr {
        $$ = new Stype(NODE_CONST_DECLARATION_STATEMENT); 
        $$->children.push_back($2); 
        $$->children.push_back($6); 
        $$->children.push_back($4); };

data_type
    : primitive_data_type {$$ = $1;}
    | '(' data_type_list ')' {
        $$ = $2; 
        $$->data_type->return_type = NULL; }
    | '(' data_type_list ')' ':' data_type {
        $$ = $2; 
        $$->data_type->return_type = $5->data_type; }
    | error;

data_type_list
    : non_empty_data_type_list {$$ = $1;}
    | {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(); 
        $$->data_type->is_primitive = false; };

non_empty_data_type_list
    : non_empty_data_type_list ',' data_type {
        $$ = $1; 
        $$->data_type->parameters.push_back($3->data_type); }
    | data_type {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(); 
        $$->data_type->is_primitive = false; 
        $$->data_type->parameters.push_back($1->data_type); };

primitive_data_type
    : primitive_data_type '[' INT_LITERAL ']' { 
        $$ = $1; 
        $$->data_type = new DataType($$->data_type, 0);}
    | primitive_data_type '[' ']' { 
        $$ = $1; 
        $$->data_type = new DataType($$->data_type, 0);}
    | INT {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(INT);}
    | LONG {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(LONG);}
    | FLOAT {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(FLOAT);}
    | AUDIO {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(AUDIO);}
    | STRING {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(STRING);}
    | BOOL {
        $$ = new Stype(NODE_DATA_TYPE); 
        $$->data_type = new DataType(BOOL);};

parameter_list
    : non_empty_parameter_list { $$ = $1; }
    | { $$ = new Stype(NODE_PARAMETER_LIST); };

non_empty_parameter_list
    : non_empty_parameter_list ',' IDENTIFIER ':' data_type { 
        $$ = $1;
        $$->children.push_back(new Stype(NODE_PARAMETER));
        $$->children.back()->children.push_back($3);
        $$->children.back()->children.push_back($5); }
    | IDENTIFIER ':' data_type { 
        $$ = new Stype(NODE_PARAMETER); 
        $$->children.push_back($1); 
        $$->children.push_back($3); };

returnable_statements
    : returnable_statements returnable_statement { 
        $$ = $1;
        $$->children.push_back($2); }
    | { $$ = new Stype(NODE_RETURNABLE_STATEMENTS); };

returnable_statement
    : statement { $$ = $1; }
    | return_statement ';' { $$ = $1; }
    | error
    | error ';' ;

return_statement
    : RETURN expr { 
        $$ = new Stype(NODE_RETURN_STATEMENT); 
        $$->children.push_back($2); }
    | RETURN { $$ = new Stype(NODE_RETURN_STATEMENT); } ;
    
function_call
    : function_name function_arguments { 
        $$ = new Stype(NODE_FUNCTION_CALL);
        $$->children.push_back($1);
        $$->children.push_back($2); }
    | '(' expr ')' function_arguments { 
        $$ = new Stype(NODE_FUNCTION_CALL);
        $$->children.push_back($2);
        $$->children.push_back($4); };

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
    : function_arguments '(' argument_list ')' { 
        $$ = $1;
        $$->children.push_back($3); }
    | '(' argument_list ')' { 
        $$ = new Stype(NODE_FUNCTION_ARGUMENTS); 
        $$->children.push_back($2); }
    | error ;

argument_list
    : non_empty_argument_list { $$ = $1; }
    | { $$ = new Stype(NODE_ARGUMENT_LIST); };

non_empty_argument_list
    : non_empty_argument_list ',' expr { 
        $$ = $1; 
        $$->children.push_back($3); }
    | non_empty_argument_list ',' '_' { 
        $$ = $1; 
        $$->children.push_back(new Stype(NODE_OMITTED_PARAMETER)); }
    | expr { $$ = $1; }
    | '_' { $$ = new Stype(NODE_OMITTED_PARAMETER); };

loop_statement
    : LOOP expr '{' loopable_statements '}' {
        $$ = new Stype(NODE_LOOP_REPEAT_STATEMENT);
        $$->children.push_back($2);
        $$->children.push_back($4);
    }
    | LOOP UNTIL expr '{' loopable_statements '}' {
        $$ = new Stype(NODE_LOOP_UNTIL_STATEMENT);
        $$->children.push_back($3);
        $$->children.push_back($5);
    }
    | LOOP OVER assignable_value expr TO expr '@' expr '{' loopable_statements '}' {
        $$ = new Stype(NODE_LOOP_GENERAL_STATEMENT);
        $$->children.push_back($3);
        $$->children.push_back($4);
        $$->children.push_back($6);
        $$->children.push_back($8);
        $$->children.push_back($10);
    }
    | LOOP OVER assignable_value expr TO expr  '{' loopable_statements '}'; {
        $$ = new Stype(NODE_LOOP_GENERAL_STATEMENT);
        $$->children.push_back($3);
        $$->children.push_back($4);
        $$->children.push_back($6);
        $$->children.push_back(new(Stype(NODE_INT_LITERAL)));
        $$->children.push_back($8);
    }

loopable_statements
    : loopable_statements loopable_statement {
        $$ = $1;
        $$->children.push_back($2);
    }
    | { $$ = new Stype(NODE_STATEMENTS); };

loopable_statement
    : statement { $$ = $1; }
    | return_statement ';' { $$ = $1; }
    | CONTINUE ';' 
    | BREAK ';' 
    | error
    | error ';';

assignment_statement
    : assignable_value '=' expr { 
        $$ = new Stype(NODE_NORMAL_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value PLUS_EQUALS expr { 
        $$ = new Stype(NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value MINUS_EQUALS expr { 
        $$ = new Stype(NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value MULT_EQUALS expr { 
        $$ = new Stype(NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value DIVIDE_EQUALS expr { 
        $$ = new Stype(NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value MOD_EQUALS expr { 
        $$ = new Stype(NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value OR_EQUALS expr { 
        $$ = new Stype(NODE_OR_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value POWER_EQUALS expr { 
        $$ = new Stype(NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); }
    | assignable_value DISTORTION_EQUALS expr { 
        $$ = new Stype(NODE_DISTORTION_EQUALS_ASSIGNMENT_STATEMENT);
        $$->children.push_back($1);
        $$->children.push_back($3); } ;

conditional_statement
    : IF expr '{' loopable_statements '}' or_statements otherwise_statement {
        $$ = new Stype(NODE_IF_STATEMENT);
        $$->children.push_back($2);
        $$->children.push_back($4);
        $$->children.push_back($6);
        $$->children.push_back($7);
    };

or_statements
    : or_statement or_statements { 
        $$ = $2; 
        $$->children.push_back($1); }
    | { $$ = new Stype(NODE_OR_STATEMENT); }; ;

or_statement
    : OR expr '{' loopable_statements '}' {
        $$ = new Stype(NODE_OR_STATEMENT);
        $$->children.push_back($2);
        $$->children.push_back($4);
    };

otherwise_statement
    : OTHERWISE '{' loopable_statements '}' {
        $$ = new Stype(NODE_OTHERWISE_STATEMENT);
        $$->children.push_back($3);
    }
    | { $$ = new Stype(NODE_OTHERWISE_STATEMENT); };

load_statement
    : LOAD expr{
        $$ = new Stype(NODE_LOAD_STATEMENT);
        $$->children.push_back($2);
    };

play_statement
    : PLAY expr{
        $$ = new Stype(NODE_PLAY_STATEMENT);
        $$.children.push_back($2);
    };

save_statement
    : SAVE assignable_value RIGHT_ARROW expr {
        $$ = new Stype(NODE_SAVE_STATEMENT);
        $$.children.push_back($2);
        $$.children.push_back($4);
    };

expr
    : '(' expr ')' {$$ = new Stype(NODE_EXPRESSION_STATEMENT); $$->children.push_back($2);}
    | '(' parameter_list ')' ':' data_type IMPLIES expr ';' {
        $$ = new Stype(NODE_EXPRESSION_STATEMENT);
        $$->children.push_back($2);
        $$->children.push_back($5);
        $$->children.push_back($7);
    }
    | '(' parameter_list ')' ':' data_type '{' returnable_statements '}' {
        $$ = new Stype(NODE_EXPRESSION_STATEMENT);
        $$->children.push_back($2);
        $$->children.push_back($5);
        $$->children.push_back($7);
    }
    | '(' parameter_list ')' IMPLIES expr ';' {
        $$ = new Stype(NODE_EXPRESSION_STATEMENT);
        $$->children.push_back($2);
        $$->children.push_back(new Stype(NODE_DATA_TYPE));
        $$->children.push_back($4);
    }
    | '(' parameter_list ')' '{' returnable_statements '}' {
        $$ = new Stype(NODE_EXPRESSION_STATEMENT);
        $$->children.push_back($2);
        $$->children.push_back(new Stype(NODE_DATA_TYPE));
        $$->children.push_back($4);
    }
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
    error_line_no = yylval->line_no;
    error_col_no = yylval->col_no;
    return 1;
}
int main() {
    yyparse();
    yyerror("");

    #ifdef SEMANTIC
    semantic();
    #endif

    return 1;
}
