// #include <cstddef>
#define SEMANTIC 1
int semantic();
#include "../Parser/y.tab.c"
#include <iostream>
#include <string>
#include <unordered_map>
// #include <bits/stdc++.h>

using namespace std;

void traverse_ast(Stype *node);

// Use this function to check if two data types are the same type
bool are_data_types_equal(DataType *type1, DataType *type2) {
    if(type1->basic_data_type == UNSET_DATA_TYPE || type2->basic_data_type == UNSET_DATA_TYPE) {
        return false;
    }
    if (type1->is_primitive && type2->is_primitive) {
        if (type1->is_vector && type2->is_vector) {
            return are_data_types_equal(type1->vector_data_type,
                                        type2->vector_data_type);
        } else if (!type1->is_vector && !type2->is_vector) {
            return type1->basic_data_type == type2->basic_data_type;
        } else
            return false;
    } else if (!type1->is_primitive && !type2->is_primitive) {
        bool are_equal = true;
        if (type1->parameters.size() == type2->parameters.size()) {
            for (int i = 0; i < type1->parameters.size(); i++) {
                are_equal =
                    are_equal && are_data_types_equal(type1->parameters[i],
                                                      type2->parameters[i]);
            }
        } else {
            are_equal = false;
        }
        are_equal = are_equal && are_data_types_equal(type1->return_type,
                                                      type2->return_type);
        return are_equal;
    } else
        return false;
}

// this function is used to see if type1 can be implicitly converted to type2.
bool can_implicitly_convert(DataType *type1, DataType *type2) {
    // check if both data types are basic.
    if (type1->is_primitive && type2->is_primitive && !type1->is_vector &&
        !type2->is_vector) {
        // true if they're equal
        if (type1->basic_data_type == type2->basic_data_type)
            return true;
        // can't inter-convert audio
        if (type1->basic_data_type == AUDIO || type2->basic_data_type == AUDIO)
            return false;
        // other than the previously handled cases, the rest can be converted to
        // string
        if (type2->basic_data_type == STRING)
            return true;
        // string can't implicitly convert to anything
        if (type1->basic_data_type == STRING)
            return false;
        // int, long, bool and float can all inter-convert
        return true;
    } else
        // if both are not basic, they have to be equal
        return are_data_types_equal(type1, type2);
}

// Symbol table entry
struct StEntry {
    DataType *data_type;
    bool is_const;
    // more fields to be added here

    StEntry() {
        data_type = NULL;
        is_const = false;
    }

    StEntry(DataType *dtype, bool is_const) {
        data_type = dtype;
        this->is_const = is_const;
    }
} typedef StEntry;

vector<vector<unordered_map<string, StEntry>>> symbol_table;

// This is just a reference to easily access symbol_table[0][0]
unordered_map<string, StEntry> *global_scope = NULL;

// this is the first index into the symbol table. when it's 0, it is the scope
// table of global statements and statements in the main block.
// the other scope tables are for function declarations
int current_scope_table = 0;
// this is the scope level inside a specific scope table. This will increase
// when we go inside an if statement or something.
int current_scope = 0;

DataType* current_return_type;

// function to do semantic checks for all declaration statements. has_dtype
// should be true if the data type was explicitly declared by the user. is_const
// is for const declarations.
int handle_declaration(Stype *node, bool has_dtype, bool is_const) {
    // if it's already in the current scope level, then it's a redeclaration.
    cout << "hello: " << current_scope_table << " " << current_scope << endl;
    cout << "node-text: " << node->children[0]->text << endl;
    if (symbol_table[current_scope_table][current_scope].count(
            node->children[0]->text)) {
        // this is just for error handling and printing the error with the
        // correct line number
        yylval = node->children[0];
        yyerror("Semantic error: redeclaration of variable!");
        return 1;
    }
    // if data type was explicitly declared and the assigned type does not
    // match/cannot be implicitly converted
    if (has_dtype && !can_implicitly_convert(node->children[1]->data_type,
                                             node->children[2]->data_type)) {
        yyerror("Semantic error: cannot convert this data type to this "
                "other one");
        return 1;
    }
    DataType *dtype;
    if (has_dtype)
        // if data type was explicitly declared, set the variable's data type to
        // the declared type
        dtype = node->children[2]->data_type;
    else
        // otherwise set it to the data type of the expression
        dtype = node->children[1]->data_type;
    // insert to symbol table
    symbol_table[current_scope_table][current_scope].insert(
        {node->children[0]->text, StEntry(dtype, is_const)});
    return 0;
}

// Function for parameter declarations in function definitions
// - Checks for re-declaration
// - Pushes to symbol table
int handle_parameter_declaration(Stype* identifier, Stype* data_type) {
    if (symbol_table[current_scope_table][current_scope].count(identifier->text)) {
        yylval = identifier;
        yyerror("Semantic error: Redeclaration of variable");
        return 1;
    }
    symbol_table[current_scope_table][current_scope].insert({identifier->text, StEntry(data_type->data_type, false)});
    return 0;
}

// This function checks if the referenced identifier exists in the symbol table
int handle_identifier_reference(Stype *node) {
    bool found = false;
    StEntry entry;
    // start at the highest scope and go higher and higher until it's found
    for (int scope = current_scope; scope >= 0; scope--) {
        if (symbol_table[current_scope_table][scope].count(node->text)) {
            found = true;
            entry = symbol_table[current_scope_table][scope][node->text];
            break;
        }
    }
    // also check global scope
    if (!found && global_scope->count(node->text)) {
        found = true;
        entry = (*global_scope)[node->text];
    }
    if (found)
        node->data_type = entry.data_type;
    else {
        yylval = node;
        yyerror(
            ("Semantic error: undefined reference to " + node->text).c_str());
        return 1;
    }
    return 0;
}

// Function to handle function definitions in expressions
// - Takes the function node as parameter
// - Creates a new symbol table for the function and assigns appropriate return type
int handle_function_expression(Stype* node) {
    // Storing global variables
    int last_scope_table = current_scope_table;
    int last_scope = current_scope;
    DataType* last_return_type = current_return_type;
    unordered_map<string, StEntry>* last_global_scope = global_scope;

    // Setting global variables for traversal
    int least_unused_scope_table = symbol_table.size();
    current_scope_table = least_unused_scope_table;
    current_scope = 0;

    // Setting current_return_type to return type of the function
    if(node->children[1]->data_type != NULL) {
        current_return_type = node->children[1]->data_type;
    } else {
        // When return type is "void"
        current_return_type = NULL;
    }

    // Creating a new table and creating a new scope
    symbol_table.push_back(vector<unordered_map<string, StEntry>>());
    symbol_table.back().push_back(unordered_map<string, StEntry>());

    // Traversing the children nodes
    traverse_ast(node->children[0]);
    traverse_ast(node->children[2]);
    
    // Return type check for inline functions
    if(node->node_type == NODE_INLINE_FUNCTION)
    {
        if(current_return_type == NULL)
        {
            if(node->children[2]->data_type != NULL)
            {
                yylval = node->children[2];
                yyerror("Semantic error: Incompatible return types");
                return 1;
            }
        }
        else
        {
            if(node->children[2]->data_type == NULL)
            {
                yylval = node->children[2];
                yyerror("Semantic error: Incompatible return types");
                return 1;
            }
            else if(!can_implicitly_convert(node->children[2]->data_type, current_return_type))
            {
                yylval = node->children[2];
                yyerror("Semantic error: Incompatible return types");
                return 1;
            }
        }
    }

    // Assigning correct data_type to function node
    node->data_type = new DataType();
    node->data_type->is_primitive = false;
    node->data_type->parameters = node->children[0]->data_type->parameters;
    node->data_type->return_type = current_return_type;

    // Restoring global variables
    current_scope_table = last_scope_table;
    current_scope = last_scope;
    current_return_type = last_return_type;
    return 0;
}

bool is_basic_type(DataType* type, int token) {
    return (type != NULL && type->is_primitive && !type->is_vector && type->basic_data_type == token);
}

void traverse_ast(Stype *node) {
    switch (node->node_type) {
    case NODE_ROOT:
        cout << "Root node" << endl;
        traverse_ast(node->children[0]);
        traverse_ast(node->children[2]);
        // main block processed last
        traverse_ast(node->children[1]);
        break;
    case NODE_STATEMENTS:
        cout << "Statements node" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;

    case NODE_MAIN_BLOCK:
        cout << "Main block node" << endl;
        // before traversing the statements node, do stuff to make the scope be
        // main block and stuff
        symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        current_scope = 1;
        traverse_ast(node->children[0]);
        current_scope = 0;
        break;
    case NODE_READ_STATEMENT:
        cout << "Read statement node" << endl;
        traverse_ast(node->children[0]);
        if (node->data_type->is_primitive && !node->data_type->is_vector) {
            if (node->data_type->basic_data_type == AUDIO) {
                yyerror("Semantic error: Cannot read audio. Use load statement instead");
            }
        } else {
            yyerror("Semantic error: Cannot read non-primitive data types");
        }
        break;
    case NODE_PRINT_STATEMENT:
        cout << "Print statement node" << endl;
        traverse_ast(node->children[0]);
        if (node->children[0]->data_type->is_primitive) {
            if (node->children[0]->data_type->basic_data_type == AUDIO) {
                yyerror("Semantic error: Cannot print audio. Use play statement instead");
            }
        } else {
            yyerror("Semantic error: Cannot print non-primitive data types");
        }

        break;
    case NODE_DECLARATION_STATEMENT:
        cout << "Declaration statement node" << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);

        if (handle_declaration(node, false, false))
            break;
        break;
    case NODE_DECLARATION_STATEMENT_WITH_TYPE:
        cout << "Declaration statement node with type" << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);

        if (handle_declaration(node, true, false))
            break;
        break;
    case NODE_CONST_DECLARATION_STATEMENT:
        cout << "Const declaration statement node" << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);

        if (handle_declaration(node, true, true))
            break;
        break;
    case NODE_DATA_TYPE:
        cout << "Data type statement node" << endl;
        break;
    case NODE_INT_LITERAL:
        cout << "Int literal node" << endl;
        node->data_type = new DataType(INT);
        break;
    case NODE_FLOAT_LITERAL:
        cout << "Float literal node" << endl;
        node->data_type = new DataType(FLOAT);
        break;
    case NODE_STRING_LITERAL:
        cout << "String literal node" << endl;
        node->data_type = new DataType(STRING);
        break;
    case NODE_IDENTIFIER:
        cout << "Identifier node" << endl;
        if (handle_identifier_reference(node))
            break;
        break;
    case NODE_PARAMETER_LIST:
        cout << "NODE_PARAMETER_LIST" << endl;
        node->data_type = new DataType();
        node->data_type->is_primitive = false;
        for(Stype* parameter: node->children) {
            traverse_ast(parameter);
            node->data_type->parameters.push_back(parameter->data_type);
        }
        break;
    case NODE_PARAMETER:
        cout << "NODE_PARAMETER" << endl;
        traverse_ast(node->children[1]);
        if(handle_parameter_declaration(node->children[0], node->children[1])) {
            break;
        }
        node->data_type = node->children[1]->data_type;
        break;
    case NODE_RETURNABLE_STATEMENTS:
        cout << "NODE_RETURNABLE_STATEMENTS" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }   
        break;
    case NODE_RETURN_STATEMENT:
        cout << "NODE_RETURN_STATEMENT" << endl;
        if (!node->children.size()){
            if (current_return_type != NULL){
                // error
                yylval = node;
                yyerror("Semantic error: Expecting Return Value got no return value");
                break;
            } else {
                node->data_type = NULL;
            }
        } else {
            if(current_return_type == NULL) {
                yylval = node;
                yyerror("Semantic error: void function cannot return non-void types");
                break;
            } else {
                traverse_ast(node->children[0]);
                node->data_type = node->children[0]->data_type;
                if (node->data_type && !can_implicitly_convert(node->data_type, current_return_type)){
                    yylval = node;
                    yyerror("Semantic error: Incompatible return type");
                    break;
                }
            }
        }
        break;
    case NODE_FUNCTION_ARGUMENTS:
        cout << "NODE_FUNCTION_ARGUMENTS" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;
    case NODE_ARGUMENT_LIST:
        cout << "NODE_ARGUMENT_LIST" << endl;
        for (Stype* child: node->children) {
            traverse_ast(child);
        }
        break;
    case NODE_OMITTED_PARAMETER:
        cout << "NODE_OMITTED_PARAMETER" << endl;
        break;
    case NODE_FUNCTION_CALL:
    {
        cout << "NODE_FUNCTION_CALL" << endl;
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        if (node->data_type->is_primitive){
            yyerror("Semantic error: Cannot call a non-function  ");
        }
        std::vector<DataType*> parameters = node->data_type->parameters;
        std::vector<DataType*> new_parameters;
        for (int i = 0; i < node->children[1]->children.size(); i++){
            
            Stype* argument_list = node->children[1]->children[i];
            if (argument_list->children.size() != parameters.size()){
                yyerror("Semantic error: Incorrect number of parameters passed");
            }
            for (int j = 0 ; j < argument_list->children.size(); j++){
            
                if (argument_list->children[j]->node_type == NODE_OMITTED_PARAMETER){
                    new_parameters.push_back(parameters[j]);
                }
                else if (!can_implicitly_convert(argument_list->children[j]->data_type, parameters[j])){
                    yyerror("Semantic error: Incompatible argument type");
                }
            }
            parameters = new_parameters;
            new_parameters.clear();
            if (!can_implicitly_convert(node->children[1]->children[i]->data_type, node->data_type->parameters[i])){
                yyerror("Semantic error: Incompatible argument type");
            }
        }
        break;
    }
    case NODE_NORMAL_ASSIGNMENT_STATEMENT:
        cout << "NODE_NORMAL_ASSIGNMENT_STATEMENT" << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);
        // then, traverse the assignable value to check if it's a valid
        // reference to a variable
        traverse_ast(node->children[0]);
        // TODO: check if data types match/can convert using the function
        break;
    case NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_OR_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_OR_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_DISTORTION_EQUALS_ASSIGNMENT_STATEMENT:
        cout << "NODE_DISTORTION_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_CONTINUE_STATEMENT:
        cout << "NODE_CONTINUE_STATEMENT" << endl;
        break;
    case NODE_BREAK_STATEMENT:
        cout << "NODE_BREAK_STATEMENT" << endl;
        break;
    case NODE_LOOP_REPEAT_STATEMENT:
        cout << "NODE_LOOP_REPEAT_STATEMENT" << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    case NODE_LOOP_UNTIL_STATEMENT:
        cout << "NODE_LOOP_UNTIL_STATEMENT" << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    case NODE_LOOP_GENERAL_STATEMENT:
        cout << "NODE_LOOP_GENERAL_STATEMENT" << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        traverse_ast(node->children[2]);
        traverse_ast(node->children[3]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    case NODE_IF_STATEMENT:
        cout << "NODE_IF_STATEMENT" << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        traverse_ast(node->children[2]);
        traverse_ast(node->children[3]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    case NODE_OR_STATEMENTS:
        cout << "NODE_OR_STATEMENTS" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;
    case NODE_OR_STATEMENT:
        cout << "NODE_OR_STATEMENT" << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    case NODE_OTHERWISE_STATEMENT:
        cout << "NODE_OTHERWISE_STATEMENT" << endl;
        if (node->children.size() == 0) {
            // do nothing
        } else {
            current_scope++;
            symbol_table[current_scope_table].push_back(unordered_map<string, StEntry>());
            traverse_ast(node->children[0]);
            symbol_table[current_scope_table].pop_back();
            current_scope--;
        }
        break;
    case NODE_LOAD_STATEMENT:
        cout << "NODE_LOAD_STATEMENT" << endl;
        break;
    case NODE_PLAY_STATEMENT:
        cout << "NODE_PLAY_STATEMENT" << endl;
        break;
    case NODE_SAVE_STATEMENT:
        cout << "NODE_SAVE_STATEMENT" << endl;
        break;
    case NODE_NORMAL_FUNCTION:
        cout << "NODE_NORMAL_FUNCTION" << endl;
        handle_function_expression(node);
        break;
    case NODE_INLINE_FUNCTION:
        cout << "NODE_INLINE_FUNCTION" << endl;
        handle_function_expression(node);
        break;
    case NODE_UNARY_INVERSE_EXPR:
        cout << "NODE_UNARY_INVERSE_EXPR" << endl;
        traverse_ast(node->children[0]);
        node->data_type = node->children[0]->data_type;
        if(!is_basic_type(node->data_type, AUDIO))
        {
            yylval = node->children[0];
            yyerror("Semantic error: Inverse operator invoked on non-audio data types");
            break;
        }
        break;
    case NODE_UNARY_LOGICAL_NOT_EXPR:
        cout << "NODE_UNARY_LOGICAL_NOT_EXPR" << endl;

        break;
    case NODE_UNARY_PLUS_EXPR:
        cout << "NODE_UNARY_PLUS_EXPR" << endl;
        break;
    case NODE_UNARY_MINUS_EXPR:
        cout << "NODE_UNARY_MINUS_EXPR" << endl;
        break;
    case NODE_BOOL_LITERAL:
        cout << "NODE_BOOL_LITERAL" << endl;
        break;
    case NODE_INDEX:
        cout << "NODE_INDEX" << endl;
        break;
    case NODE_SLICE:
        cout << "NODE_SLICE" << endl;
        break;
    case NODE_POWER_EXPR:
        cout << "NODE_POWER_EXPR" << endl;
        break;
    case NODE_DISTORTION_EXPR:
        cout << "NODE_DISTORTION_EXPR" << endl;
        break;
    case NODE_MULT_EXPR:
        cout << "NODE_MULT_EXPR" << endl;
        break;
    case NODE_DIVIDE_EXPR:
        cout << "NODE_DIVIDE_EXPR" << endl;
        break;
    case NODE_MOD_EXPR:
        cout << "NODE_MOD_EXPR" << endl;
        break;
    case NODE_SPEEDUP_EXPR:
        cout << "NODE_SPEEDUP_EXPR" << endl;
        break;
    case NODE_SPEEDDOWN_EXPR:
        cout << "NODE_SPEEDDOWN_EXPR" << endl;
        break;
    case NODE_PLUS_EXPR:
        cout << "NODE_PLUS_EXPR" << endl;
        break;
    case NODE_MINUS_EXPR:
        cout << "NODE_MINUS_EXPR" << endl;
        break;
    case NODE_SUPERPOSITION_EXPR:
        cout << "NODE_SUPERPOSITION_EXPR" << endl;
        break;
    case NODE_LE_EXPR:
        cout << "nœud le expression" << endl;
        break;
    case NODE_LEQ_EXPR:
        cout << "NODE_LEQ_EXPR" << endl;
        break;
    case NODE_GE_EXPR:
        cout << "NODE_GE_EXPR" << endl;
        break;
    case NODE_GEQ_EXPR:
        cout << "NODE_GEQ_EXPR" << endl;
        break;
    case NODE_EQUALS_EXPR:
        cout << "NODE_EQUALS_EXPR" << endl;
        break;
    case NODE_NOT_EQUALS_EXPR:
        cout << "NODE_NOT_EQUALS_EXPR" << endl;
        break;
    case NODE_LOGICAL_AND_EXPR:
        cout << "NODE_LOGICAL_AND_EXPR" << endl;
        break;
    case NODE_LOGICAL_OR_EXPR:
        cout << "NODE_LOGICAL_OR_EXPR" << endl;
        break;
    case NODE_ASSIGNABLE_VALUE:
        cout << "NODE_ASSIGNABLE_VALUE" << endl;
        traverse_ast(node->children[0]);
        node->data_type = node->children[0]->data_type;
        // TODO: handle vector index and slice
        break;
    case NODE_NOT_SET:
        cout << "Big bad error: Oops, looks like you have an uninitialised Stype somewhere!"
             << endl;
        break;
    default:
        cout << "Big bad error: bruh you forgot to handle this node" << endl;
    }
}

int semantic() {
    // Initialising the symbol table
    symbol_table = vector<vector<unordered_map<string, StEntry>>>(
        1, vector<unordered_map<string, StEntry>>(
               1, unordered_map<string, StEntry>()));

    global_scope = &symbol_table[0][0];

    traverse_ast(root);
    return 0;
}
