// #include <cstddef>
#include <algorithm>
#include <type_traits>
#define SEMANTIC 1
int semantic();
#include "../Parser/y.tab.c"
#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

void traverse_ast(Stype *node);

// Use this function to check if two data types are the same type
bool are_data_types_equal(DataType *type1, DataType *type2) {
    if (!type1 || !type2)
        return type1 == type2;
    if (type1->is_primitive && type2->is_primitive) {
        if (type1->is_vector && type2->is_vector) {
            return are_data_types_equal(type1->vector_data_type,
                                        type2->vector_data_type);
        } else if (!type1->is_vector && !type2->is_vector) {
            return type1->basic_data_type == type2->basic_data_type &&
                   type1->basic_data_type != UNSET_DATA_TYPE;
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

bool are_data_types_equal_and_not_null(DataType *type1, DataType *type2) {
    if (!type1 || !type2)
        return false;
    return are_data_types_equal(type1, type2);
}

// this function is used to see if type1 can be implicitly converted to type2.
bool can_implicitly_convert(DataType *type1, DataType *type2) {
    if (!type1 || !type2)
        return false;
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

int final_return_type(DataType *type) {
    if (!type)
        return 0;
    if (type->is_primitive) {
        if (type->is_vector) {
            return final_return_type(type->vector_data_type);
        }
        return type->basic_data_type;
    } else {
        return final_return_type(type->return_type);
    }
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

DataType *current_return_type;

// function to do semantic checks for all declaration statements. has_dtype
// should be true if the data type was explicitly declared by the user. is_const
// is for const declarations.
int handle_declaration(Stype *node, bool has_dtype, bool is_const) {
    // if it's already in the current scope level, then it's a redeclaration.
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
int handle_parameter_declaration(Stype *identifier, Stype *data_type) {
    if (symbol_table[current_scope_table][current_scope].count(
            identifier->text)) {
        yylval = identifier;
        yyerror("Semantic error: Redeclaration of variable");
        return 1;
    }
    symbol_table[current_scope_table][current_scope].insert(
        {identifier->text, StEntry(data_type->data_type, false)});
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
// - Creates a new symbol table for the function and assigns appropriate return
// type
int handle_function_expression(Stype *node) {
    // Storing global variables
    int last_scope_table = current_scope_table;
    int last_scope = current_scope;
    DataType *last_return_type = current_return_type;
    unordered_map<string, StEntry> *last_global_scope = global_scope;

    // Setting global variables for traversal
    int least_unused_scope_table = symbol_table.size();
    current_scope_table = least_unused_scope_table;
    current_scope = 0;

    // Setting current_return_type to return type of the function
    if (node->children[1]->data_type != NULL) {
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
    if (node->node_type == NODE_INLINE_FUNCTION) {
        if (current_return_type == NULL) {
            if (node->children[2]->data_type != NULL) {
                yylval = node->children[2];
                yyerror("Semantic error: Incompatible return types");
                return 1;
            }
        } else {
            if (node->children[2]->data_type == NULL) {
                yylval = node->children[2];
                yyerror("Semantic error: Incompatible return types");
                return 1;
            } else if (!can_implicitly_convert(node->children[2]->data_type,
                                               current_return_type)) {
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

bool is_basic_type(DataType *type, int token) {
    return (type != NULL && type->is_primitive && !type->is_vector &&
            type->basic_data_type == token);
}

bool is_basic(DataType *type) {
    return (type != NULL && type->is_primitive && !type->is_vector);
}

bool convertible_to_float(DataType *type) {
    return (type != NULL && type->is_primitive && !type->is_vector &&
            (type->basic_data_type == INT || type->basic_data_type == LONG ||
             type->basic_data_type == FLOAT || type->basic_data_type == BOOL));
}

bool isFunction(DataType *type) {
    return (type != NULL && !type->is_primitive);
}

// Function to check if two data types are strictly same
bool equal_data_type(DataType *type1, DataType *type2) {
    if (type1->is_primitive != type2->is_primitive)
        return false;

    if (type1->is_primitive) {
        if (type1->is_vector != type2->is_vector)
            return false;

        if (type1->is_vector) {
            return equal_data_type(type1->vector_data_type,
                                   type2->vector_data_type);
        } else {
            return (type1->basic_data_type == type2->basic_data_type);
        }
    } else {
        if (!equal_data_type(type1->return_type, type2->return_type))
            return false;

        int n = type1->parameters.size();

        if (n != type2->parameters.size())
            return false;

        for (int i = 0; i < n; i++) {
            if (!equal_data_type(type1->parameters[i], type2->parameters[i]))
                return false;
        }
        return true;
    }

    // It should never this line
    return false;
}

int handle_unary_expression(Stype *node) {
    traverse_ast(node->children[0]);
    node->data_type = node->children[0]->data_type;
    if (!is_basic_type(node->data_type, INT) &&
        !is_basic_type(node->data_type, LONG) &&
        !is_basic_type(node->data_type, FLOAT) &&
        !is_basic_type(node->data_type, BOOL)) {
        yylval = node->children[0];
        yyerror("Semantic error: Unary operator invoked on incompatible data "
                "types");
        return 1;
    }
    return 0;
}

// Function to handle power expressions
// - Takes the node as parameter
// - Performs semantic checks on the operands
// - Assigns appropriate data type to node
int handle_power_expression(Stype *node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: power operator cannot be invoked on void data "
                "types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: power operator cannot be invoked on void data "
                "types");
        return 1;
    }

    // Checking if the right operand is compatible with power operator
    if (!is_basic_type(type2, INT) && !is_basic_type(type2, LONG) &&
        !is_basic_type(type2, FLOAT) && !is_basic_type(type2, BOOL)) {
        yylval = node->children[1];
        yyerror("Semantic error: Right operand is incompatible with power "
                "operator");
        return 1;
    }

    // Checking if the left operand is compatible with power operator
    if (type1 == NULL || !type1->is_primitive || type1->is_vector ||
        type1->basic_data_type == UNSET_DATA_TYPE) {
        yylval = node->children[0];
        yyerror(
            "Semantic error: Left operand is incompatible with power operator");
        return 1;
    }

    // Assigning correct data type to node
    if (convertible_to_float(type1)) {
        node->data_type = new DataType(FLOAT);
    } else {
        node->data_type = type1;
    }

    return 0;
}

// Function to handle distortion expressions
// - Takes the node as parameter
// - Performs semantic checks on the operands
// - Assigns appropriate data type to node
int handle_distortion_expression(Stype *node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: distortion operator cannot be invoked on void "
                "data types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: distortion operator cannot be invoked on void "
                "data types");
        return 1;
    }

    // Checking if the right operand is compatible with distortion operator
    if (type2->is_primitive || type2->parameters.size() != 1 ||
        !is_basic_type(type2->parameters[0], INT) ||
        !is_basic_type(type2->return_type, INT)) {
        yylval = node->children[1];
        yyerror("Semantic error: Right operand is incompatible with distortion "
                "operator, It should be a function which takes an int and "
                "returns an int");
        return 1;
    }

    // Checking if the left operand is compatible with distortion operator
    if (!is_basic_type(type1, AUDIO)) {
        yylval = node->children[0];
        yyerror("Semantic error: Left operand is incompatible with distortion "
                "operator, It should be audio");
        return 1;
    }

    // Assigning correct data type to node
    node->data_type = type1;

    return 0;
}

// Function to handle multiplication expressions
// - Takes the node as parameter
// - Performs semantic checks on the operands
// - Assigns appropriate data type to node
int handle_mult_expression(Stype *node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: multiplication operator cannot be invoked on "
                "void data types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: multiplication operator cannot be invoked on "
                "void data types");
        return 1;
    }

    // Checking the left operator
    if (is_basic_type(type1, STRING)) {
        yylval = node->children[0];
        yyerror("Semantic error: multiplication operator is not defined for "
                "strings");
        return 1;
    } else if (convertible_to_float(type1)) {
        if (!convertible_to_float(type2)) {
            if (isFunction(type2) && convertible_to_float(type2->return_type)) {
                node->data_type = type2;
                return 0;
            } else {
                yylval = node->children[1];
                yyerror("Semantic error: invalid operand for multiplication "
                        "operator");
                return 1;
            }
        }
        if (is_basic_type(type1, FLOAT) || is_basic_type(type2, FLOAT)) {
            node->data_type = new DataType(FLOAT);
        } else if (is_basic_type(type1, LONG) || is_basic_type(type2, LONG)) {
            node->data_type = new DataType(LONG);
        } else {
            node->data_type = new DataType(INT);
        }
        return 0;
    } else if (is_basic_type(type1, AUDIO)) {
        if (!convertible_to_float(type2)) {
            yylval = node->children[1];
            yyerror(
                "Semantic error: invalid operand for multiplication operator");
            return 1;
        }
        node->data_type = type1;
        return 0;
    } else if (isFunction(type1)) {
        if (convertible_to_float(type1->return_type) &&
            (equal_data_type(type1, type2) || convertible_to_float(type2))) {
            node->data_type = type1;
            return 0;
        } else {
            yylval = node->children[1];
            yyerror(
                "Semantic error: invalid operand for multiplication operator");
            return 1;
        }
    } else {
        yylval = node->children[0];
        yyerror("Semantic error: invalid operand for multiplication operator");
        return 1;
    }

    // It should never reach this line
    return 1;
}

// Function to handle division expressions
// - Takes the node as parameter
// - Performs semantic checks on the operands
// - Assigns appropriate data type to node
int handle_divide_expression(Stype *node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: division operator cannot be invoked on void "
                "data types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: division operator cannot be invoked on void "
                "data types");
        return 1;
    }

    // Checking the left operator
    if (is_basic_type(type1, STRING)) {
        yylval = node->children[0];
        yyerror("Semantic error: division operator is not defined for strings");
        return 1;
    } else if (convertible_to_float(type1)) {
        if (!convertible_to_float(type2)) {
            if (isFunction(type2) && convertible_to_float(type2->return_type)) {
                node->data_type = new DataType();
                node->data_type->is_primitive = false;
                node->data_type->parameters = type2->parameters;
                node->data_type->return_type = new DataType(FLOAT);
                return 0;
            } else {
                yylval = node->children[1];
                yyerror(
                    "Semantic error: invalid operand for division operator");
                return 1;
            }
        }
        node->data_type = new DataType(FLOAT);
        return 0;
    } else if (isFunction(type1)) {
        if (convertible_to_float(type1->return_type) &&
            (equal_data_type(type1, type2) || convertible_to_float(type2))) {
            node->data_type = new DataType();
            node->data_type->is_primitive = false;
            node->data_type->parameters = type1->parameters;
            node->data_type->return_type = new DataType(FLOAT);
            return 0;
        } else {
            yylval = node->children[1];
            yyerror("Semantic error: invalid operand for division operator");
            return 1;
        }
    } else {
        yylval = node->children[0];
        yyerror("Semantic error: invalid operand for division operator");
        return 1;
    }

    // It should never reach this line
    return 1;
}

int handle_modulo_expression(Stype *node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);

    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;
    int t1 = final_return_type(type1);
    int t2 = final_return_type(type2);
    if (!(is_basic(type1) && is_basic(type2))) {
        if (!are_data_types_equal_and_not_null(type1, type2)) {
            yylval = node->children[0];
            yyerror("Semantic error: invalid use of modulo operator");
            // yyerror
            return 1;
        }
    }

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: modulo operator cannot be invoked on void "
                "data types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: modulo operator cannot be invoked on void "
                "data types");
        return 1;
    }
    if (!type1->is_primitive) {
        if (!type2->is_primitive) {
            yylval = node->children[1];
            yyerror("Semantic error: modulo expression cannot be called on two "
                    "functions");
            return 1;
        }
        node->data_type = type1;
        type1 = type1->return_type;
    } else if (!type2->is_primitive) {
        node->data_type = type2;
        type2 = type2->return_type;
    }

    if (!type1->is_primitive || !type2->is_primitive) {
        yylval = node->children[0];
        yyerror("Semantic error: function used in modulo expression must "
                "return a basic data type");
    }

    if (type1->is_vector || type2->is_vector) {
        yylval = node->children[1];
        yyerror("Semantic error: modulo operation not supported for vectors");
    }

    if (type1->basic_data_type == STRING || type2->basic_data_type == STRING ||
        type2->basic_data_type == AUDIO) {
        yylval = node->children[1];
        yyerror("Semantic error: Types incompatible for modulo operator");
    }

    int curr_prim_data_type;

    if (type1->basic_data_type == AUDIO) {
        curr_prim_data_type = AUDIO;
    } else if (type1->basic_data_type == FLOAT ||
               type2->basic_data_type == FLOAT) {
        curr_prim_data_type = FLOAT;
    } else if (type1->basic_data_type == LONG ||
               type2->basic_data_type == LONG) {
        curr_prim_data_type = LONG;
    } else {
        curr_prim_data_type = INT;
    }
    if (!node->data_type->is_primitive)
        node->data_type->return_type = new DataType(curr_prim_data_type);
    else
        node->data_type->basic_data_type = curr_prim_data_type;

    return 0;
}

int handle_speedup_speeddown_expression(Stype *node, bool is_speedup) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: speedup/speeddown operator cannot be invoked "
                "on void "
                "data types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: speedup/speeddown operator cannot be invoked "
                "on void "
                "data types");
        return 1;
    } else if (!type2->is_primitive) {
        yylval = node->children[1];
        yyerror("Semantic error: cannot speedup/speeddown by a function");
        return 1;
    }
    if (!type1->is_primitive) {
        node->data_type = type1;
        type1 = type1->return_type;
    } else {
        node->data_type = type1;
    }
    if (!type1->is_primitive || type1->is_vector ||
        type1->basic_data_type != AUDIO) {
        yylval = node->children[0];
        yyerror("Semantic error: function used in speedup/speeddown expression "
                "must return audio");
    }

    if (type2->is_vector) {
        yylval = node->children[1];
        yyerror("Semantic error: speedup/speeddown operation not supported for "
                "vectors");
    }
    if (type2->basic_data_type == STRING || type2->basic_data_type == AUDIO) {
        yylval = node->children[1];
        yyerror("Semantic error: Types incompatible for speedup/speeddown "
                "operator");
    }

    return 0;
}

int handle_plus_expression(Stype* node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: addition operator cannot be invoked on void data types");
        return 1;
    }
    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: addition operator cannot be invoked on void data types");
        return 1;
    }

    // Checking if both are not of the same type
    if (type1->is_primitive != type2->is_primitive) {
        yylval = node->children[0];
        yyerror("Semantic error: incompatiable operands for addition operator");
        return 1;
    }

    // When they are not functions
    if (type1->is_primitive) {
        // Addition is not supported for vectors
        if (type1->is_vector) {
            yylval = node->children[0];
            yyerror("Semantic error: incompatiable operands for addition operator");
            return 1;
        }

        if (type1->is_vector) {
            yylval = node->children[1];
            yyerror("Semantic error: incompatiable operands for addition operator");
            return 1;
        }

        if (type1->basic_data_type == UNSET_DATA_TYPE) {
            yylval = node->children[0];
            yyerror("Semantic error: data type cannot be unset");
            return 1;
        }

        if (type2->basic_data_type == UNSET_DATA_TYPE) {
            yylval = node->children[1];
            yyerror("Semantic error: data type cannot be unset");
            return 1;
        }

        // Audio addition(concatenation)
        if (type1->basic_data_type == AUDIO) {
            if(type2->basic_data_type == AUDIO) {
                node->data_type = type1;
                return 0;
            } else {
                yylval = node->children[1];
                yyerror("Semantic error: audio can only be added with audio");
                return 1;
            }
        } else if(type1->basic_data_type == STRING) {
            if(type2->basic_data_type == AUDIO) {
                yylval = node->children[1];
                yyerror("Semantic error: audio cannot be added to a string");
                return 1;
            } else {
                node->data_type = type1;
                return 0;
            }
        } else {
            if(is_basic_type(type1, FLOAT) || is_basic_type(type2, FLOAT)) {
                node->data_type = new DataType(FLOAT);
            } else if (is_basic_type(type1, LONG) || is_basic_type(type2, LONG)) {
                node->data_type = new DataType(LONG);
            } else {
                node->data_type = new DataType(INT);
            }
            return 0;
        }
    } else {
        // TODO: Handle functions additions
        // return
    }

    // It should never reach this line
    return 1;
}

int handle_minus_expression(Stype* node)
{
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: minus operator cannot be invoked on "
                "void data types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: minus operator cannot be invoked on "
                "void data types");
        return 1;
    }

    // Checking the left operator
    if (is_basic_type(type1, STRING)) {
        yylval = node->children[0];
        yyerror("Semantic error: minus operator is not defined for "
                "strings");
        return 1;
    } else if (convertible_to_float(type1)) {
        if (!convertible_to_float(type2)) {
            yylval = node->children[1];
            yyerror("Semantic error: invalid operand for minus "
                    "operator");
            return 1;
        }
        if (is_basic_type(type1, FLOAT) || is_basic_type(type2, FLOAT)) {
            type1->basic_data_type = FLOAT;
            node->data_type = type1;
        } else if (is_basic_type(type1, LONG) || is_basic_type(type2, LONG)) {
            type1->basic_data_type = LONG;
            node->data_type = type1;
        } else {
            type1->basic_data_type = INT;
            node->data_type = type1;
        }
        return 0;
    } else if (is_basic_type(type1, AUDIO)) {
        yylval = node->children[1];
        yyerror(
            "Semantic error: invalid operand for minus "
            "operator");
        return 1;
    } else if (isFunction(type1)) {
        if (isFunction(type2)){
            if (are_data_types_equal_and_not_null(type1, type2)){
                if (final_return_type(type1) != STRING || final_return_type(type1) != AUDIO ){
                    // node->data_type = new DataType();
                    node->data_type->is_primitive = false;
                    node->data_type->parameters = type1->parameters;
                    node->data_type->return_type = type1->return_type;
                } 
                
            }
        }
    } else {
        yylval = node->children[0];
        yyerror("Semantic error: invalid operand for minus operator");
        return 1;
    }
    // It should never reach this line
    return 1;
}

int handle_superposition_expression(Stype* node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: superposition operator cannot be invoked on void data types");
        return 1;
    }
    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: superposition operator cannot be invoked on void data types");
        return 1;
    }

    // Both data types must be AUDIO
    if (!is_basic_type(type1, AUDIO)) {
        yylval = node->children[0];
        yyerror("Semantic error: superposition operator is only defined for audio data types");
        return 1;
    }

    if (!is_basic_type(type2, AUDIO)) {
        yylval = node->children[1];
        yyerror("Semantic error: superposition operator is only defined for audio data types");
        return 1;
    }

    node->data_type = type1;
    return 0;
}

int handle_relational_expression(Stype* node, bool is_string_allowed) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: relational operators cannot be invoked on void data types");
        return 1;
    }
    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: relational operators cannot be invoked on void data types");
        return 1;
    }

    if (!convertible_to_float(type1) && !(is_string_allowed && is_basic_type(type1, STRING))) {
        yylval = node->children[0];
        yyerror("Semantic error: incompatiable operand for this relational operator");
        return 1;
    }

    if (!convertible_to_float(type2) && !(is_string_allowed && is_basic_type(type2, STRING))) {
        yylval = node->children[1];
        yyerror("Semantic error: incompatiable operand for this relational operator");
        return 1;
    }

    node->data_type = new DataType(BOOL);
    return 0;    
}

int handle_plus_expression(Stype* node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: addition operator cannot be invoked on void data types");
        return 1;
    }
    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: addition operator cannot be invoked on void data types");
        return 1;
    }

    // Checking if both are not of the same type
    if (type1->is_primitive != type2->is_primitive) {
        yylval = node->children[0];
        yyerror("Semantic error: incompatiable operands for addition operator");
        return 1;
    }

    // When they are not functions
    if (type1->is_primitive) {
        // Addition is not supported for vectors
        if (type1->is_vector) {
            yylval = node->children[0];
            yyerror("Semantic error: incompatiable operands for addition operator");
            return 1;
        }

        if (type1->is_vector) {
            yylval = node->children[1];
            yyerror("Semantic error: incompatiable operands for addition operator");
            return 1;
        }

        if (type1->basic_data_type == UNSET_DATA_TYPE) {
            yylval = node->children[0];
            yyerror("Semantic error: data type cannot be unset");
            return 1;
        }

        if (type2->basic_data_type == UNSET_DATA_TYPE) {
            yylval = node->children[1];
            yyerror("Semantic error: data type cannot be unset");
            return 1;
        }

        // Audio addition(concatenation)
        if (type1->basic_data_type == AUDIO) {
            if(type2->basic_data_type == AUDIO) {
                node->data_type = type1;
                return 0;
            } else {
                yylval = node->children[1];
                yyerror("Semantic error: audio can only be added with audio");
                return 1;
            }
        } else if(type1->basic_data_type == STRING) {
            if(type2->basic_data_type == AUDIO) {
                yylval = node->children[1];
                yyerror("Semantic error: audio cannot be added to a string");
                return 1;
            } else {
                node->data_type = type1;
                return 0;
            }
        } else {
            if(is_basic_type(type1, FLOAT) || is_basic_type(type2, FLOAT)) {
                node->data_type = new DataType(FLOAT);
            } else if (is_basic_type(type1, LONG) || is_basic_type(type2, LONG)) {
                node->data_type = new DataType(LONG);
            } else {
                node->data_type = new DataType(INT);
            }
            return 0;
        }
    } else {
        // TODO: Handle functions additions
        // return
    }

    // It should never reach this line
    return 1;
}


int handle_superposition_expression(Stype* node) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: superposition operator cannot be invoked on void data types");
        return 1;
    }
    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: superposition operator cannot be invoked on void data types");
        return 1;
    }

    // Both data types must be AUDIO
    if (!is_basic_type(type1, AUDIO)) {
        yylval = node->children[0];
        yyerror("Semantic error: superposition operator is only defined for audio data types");
        return 1;
    }

    if (!is_basic_type(type2, AUDIO)) {
        yylval = node->children[1];
        yyerror("Semantic error: superposition operator is only defined for audio data types");
        return 1;
    }

    node->data_type = type1;
    return 0;
}

int handle_relational_expression(Stype* node, bool is_string_allowed) {
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: relational operators cannot be invoked on void data types");
        return 1;
    }
    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: relational operators cannot be invoked on void data types");
        return 1;
    }

    if (!convertible_to_float(type1) && !(is_string_allowed && is_basic_type(type1, STRING))) {
        yylval = node->children[0];
        yyerror("Semantic error: incompatiable operand for this relational operator");
        return 1;
    }

    if (!convertible_to_float(type2) && !(is_string_allowed && is_basic_type(type2, STRING))) {
        yylval = node->children[1];
        yyerror("Semantic error: incompatiable operand for this relational operator");
        return 1;
    }

    node->data_type = new DataType(BOOL);
    return 0;    
}

int handle_minus_expression(Stype* node)
{
    traverse_ast(node->children[0]);
    traverse_ast(node->children[1]);
    DataType *type1 = node->children[0]->data_type;
    DataType *type2 = node->children[1]->data_type;

    // Checking if any data types are NULL(void in DSL)
    if (type1 == NULL) {
        yylval = node->children[0];
        yyerror("Semantic error: minus operator cannot be invoked on "
                "void data types");
        return 1;
    }

    if (type2 == NULL) {
        yylval = node->children[1];
        yyerror("Semantic error: minus operator cannot be invoked on "
                "void data types");
        return 1;
    }

    // Checking the left operator
    if (is_basic_type(type1, STRING)) {
        yylval = node->children[0];
        yyerror("Semantic error: minus operator is not defined for "
                "strings");
        return 1;
    } else if (convertible_to_float(type1)) {
        if (!convertible_to_float(type2)) {
            yylval = node->children[1];
            yyerror("Semantic error: invalid operand for minus "
                    "operator");
            return 1;
        }
        if (is_basic_type(type1, FLOAT) || is_basic_type(type2, FLOAT)) {
            type1->basic_data_type = FLOAT;
            node->data_type = type1;
        } else if (is_basic_type(type1, LONG) || is_basic_type(type2, LONG)) {
            type1->basic_data_type = LONG;
            node->data_type = type1;
        } else {
            type1->basic_data_type = INT;
            node->data_type = type1;
        }
        return 0;
    } else if (is_basic_type(type1, AUDIO)) {
        yylval = node->children[1];
        yyerror(
            "Semantic error: invalid operand for minus "
            "operator");
        return 1;
    } else if (isFunction(type1)) {
        if (isFunction(type2)){
            if (are_data_types_equal_and_not_null(type1, type2)){
                if (final_return_type(type1) != STRING || final_return_type(type1) != AUDIO ){
                    // node->data_type = new DataType();
                    node->data_type->is_primitive = false;
                    node->data_type->parameters = type1->parameters;
                    node->data_type->return_type = type1->return_type;
                } 
                
            }
        }
    } else {
        yylval = node->children[0];
        yyerror("Semantic error: invalid operand for minus operator");
        return 1;
    }
    // It should never reach this line
    return 1;
}
void traverse_ast(Stype *node) {
    switch (node->node_type) {
    case NODE_ROOT:
        cout << string(current_scope, '\t') << "Root node" << endl;
        traverse_ast(node->children[0]);
        traverse_ast(node->children[2]);
        // main block processed last
        traverse_ast(node->children[1]);
        break;
    case NODE_STATEMENTS:
        cout << string(current_scope, '\t') << "Statements node" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;

    case NODE_MAIN_BLOCK:
        cout << string(current_scope, '\t') << "Main block node" << endl;
        // before traversing the statements node, do stuff to make the scope be
        // main block and stuff
        symbol_table[current_scope_table].push_back(
            unordered_map<string, StEntry>());
        current_scope = 1;
        traverse_ast(node->children[0]);
        current_scope = 0;
        break;
    case NODE_READ_STATEMENT:
        cout << string(current_scope, '\t') << "Read statement node" << endl;
        traverse_ast(node->children[0]);
        if (node->data_type->is_primitive && !node->data_type->is_vector) {
            if (node->data_type->basic_data_type == AUDIO) {
                yyerror("Semantic error: Cannot read audio. Use load statement "
                        "instead");
            }
        } else {
            yyerror("Semantic error: Cannot read non-primitive data types");
        }
        break;
    case NODE_PRINT_STATEMENT:
        cout << string(current_scope, '\t') << "Print statement node" << endl;
        traverse_ast(node->children[0]);
        if (node->children[0]->data_type->is_primitive) {
            if (node->children[0]->data_type->basic_data_type == AUDIO) {
                yyerror("Semantic error: Cannot print audio. Use play "
                        "statement instead");
            }
        } else {
            yyerror("Semantic error: Cannot print non-primitive data types");
        }

        break;
    case NODE_DECLARATION_STATEMENT:
        cout << string(current_scope, '\t') << "Declaration statement node"
             << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);

        if (handle_declaration(node, false, false))
            break;
        break;
    case NODE_DECLARATION_STATEMENT_WITH_TYPE:
        cout << string(current_scope, '\t')
             << "Declaration statement node with type" << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);

        if (handle_declaration(node, true, false))
            break;
        break;
    case NODE_CONST_DECLARATION_STATEMENT:
        cout << string(current_scope, '\t')
             << "Const declaration statement node" << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);

        if (handle_declaration(node, true, true))
            break;
        break;
    case NODE_DATA_TYPE:
        cout << string(current_scope, '\t') << "Data type statement node"
             << endl;
        break;
    case NODE_INT_LITERAL:
        cout << string(current_scope, '\t') << "Int literal node" << endl;
        node->data_type = new DataType(INT);
        break;
    case NODE_FLOAT_LITERAL:
        cout << string(current_scope, '\t') << "Float literal node" << endl;
        node->data_type = new DataType(FLOAT);
        break;
    case NODE_STRING_LITERAL:
        cout << string(current_scope, '\t') << "String literal node" << endl;
        node->data_type = new DataType(STRING);
        break;
    case NODE_BOOL_LITERAL:
        cout << string(current_scope, '\t') << "NODE_BOOL_LITERAL" << endl;
        node->data_type = new DataType(BOOL);
        break;
    case NODE_IDENTIFIER:
        cout << string(current_scope, '\t') << "Identifier node" << endl;
        if (handle_identifier_reference(node))
            break;
        break;
    case NODE_PARAMETER_LIST:
        cout << string(current_scope, '\t') << "NODE_PARAMETER_LIST" << endl;
        node->data_type = new DataType();
        node->data_type->is_primitive = false;
        for (Stype *parameter : node->children) {
            traverse_ast(parameter);
            node->data_type->parameters.push_back(parameter->data_type);
        }
        break;
    case NODE_PARAMETER:
        cout << string(current_scope, '\t') << "NODE_PARAMETER" << endl;
        traverse_ast(node->children[1]);
        if (handle_parameter_declaration(node->children[0],
                                         node->children[1])) {
            break;
        }
        node->data_type = node->children[1]->data_type;
        break;
    case NODE_RETURNABLE_STATEMENTS:
        cout << string(current_scope, '\t') << "NODE_RETURNABLE_STATEMENTS"
             << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;
    case NODE_RETURN_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_RETURN_STATEMENT" << endl;
        if (!node->children.size()) {
            if (current_return_type != NULL) {
                // error
                yylval = node;
                yyerror("Semantic error: Expecting Return Value got no return "
                        "value");
                break;
            } else {
                node->data_type = NULL;
            }
        } else {
            if (current_return_type == NULL) {
                yylval = node;
                yyerror("Semantic error: void function cannot return non-void "
                        "types");
                break;
            } else {
                traverse_ast(node->children[0]);
                node->data_type = node->children[0]->data_type;
                if (node->data_type &&
                    !can_implicitly_convert(node->data_type,
                                            current_return_type)) {
                    yylval = node;
                    yyerror("Semantic error: Incompatible return type");
                    break;
                }
            }
        }
        break;
    case NODE_FUNCTION_ARGUMENTS:
        cout << string(current_scope, '\t') << "NODE_FUNCTION_ARGUMENTS"
             << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;
    case NODE_ARGUMENT_LIST:
        cout << string(current_scope, '\t') << "NODE_ARGUMENT_LIST" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;
    case NODE_OMITTED_PARAMETER:
        cout << string(current_scope, '\t') << "NODE_OMITTED_PARAMETER" << endl;
        break;
    case NODE_FUNCTION_CALL: {
        cout << string(current_scope, '\t') << "NODE_FUNCTION_CALL" << endl;
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        if (node->data_type->is_primitive) {
            yyerror("Semantic error: Cannot call a non-function  ");
        }
        std::vector<DataType *> parameters = node->data_type->parameters;
        std::vector<DataType *> new_parameters;
        for (int i = 0; i < node->children[1]->children.size(); i++) {

            Stype *argument_list = node->children[1]->children[i];
            if (argument_list->children.size() != parameters.size()) {
                yyerror(
                    "Semantic error: Incorrect number of parameters passed");
            }
            for (int j = 0; j < argument_list->children.size(); j++) {

                if (argument_list->children[j]->node_type ==
                    NODE_OMITTED_PARAMETER) {
                    new_parameters.push_back(parameters[j]);
                } else if (!can_implicitly_convert(
                               argument_list->children[j]->data_type,
                               parameters[j])) {
                    yyerror("Semantic error: Incompatible argument type");
                }
            }
            parameters = new_parameters;
            new_parameters.clear();
            if (!can_implicitly_convert(
                    node->children[1]->children[i]->data_type,
                    node->data_type->parameters[i])) {
                yyerror("Semantic error: Incompatible argument type");
            }
        }
        break;
    }
    case NODE_NORMAL_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_NORMAL_ASSIGNMENT_STATEMENT" << endl;
        // traverse the expression first
        traverse_ast(node->children[1]);
        // then, traverse the assignable value to check if it's a valid
        // reference to a variable
        traverse_ast(node->children[0]);
        // TODO: check if data types match/can convert using the function
        break;
    case NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_PLUS_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_MINUS_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_MULT_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_DIVIDE_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_MOD_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_OR_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_OR_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_POWER_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_DISTORTION_EQUALS_ASSIGNMENT_STATEMENT:
        cout << string(current_scope, '\t')
             << "NODE_DISTORTION_EQUALS_ASSIGNMENT_STATEMENT" << endl;
        break;
    case NODE_CONTINUE_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_CONTINUE_STATEMENT"
             << endl;
        break;
    case NODE_BREAK_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_BREAK_STATEMENT" << endl;
        break;
    case NODE_LOOP_REPEAT_STATEMENT: {
        cout << string(current_scope, '\t') << "NODE_LOOP_REPEAT_STATEMENT"
             << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(
            unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        DataType *type2 = new DataType(INT);
        if (node->children[0]->data_type == NULL) {
            yyerror("Semantic error: Repeat statement expects an integer, "
                    "recieved void");
        }
        if (!can_implicitly_convert(node->children[0]->data_type, type2)) {
            yyerror("Semantic error: Repeat statement expects an integer");
        }
        traverse_ast(node->children[1]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    }
    case NODE_LOOP_UNTIL_STATEMENT: {
        cout << string(current_scope, '\t') << "NODE_LOOP_UNTIL_STATEMENT"
             << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(
            unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        DataType *type2 = new DataType(LONG);
        if (node->children[0]->data_type == NULL) {
            yyerror("Semantic error: Repeat statement expects an integer, "
                    "recieved void");
        }
        if (!can_implicitly_convert(node->children[0]->data_type, type2)) {
            yyerror("Semantic error: Repeat statement expects an integer");
        }
        traverse_ast(node->children[1]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    }
    case NODE_LOOP_GENERAL_STATEMENT: {
        cout << string(current_scope, '\t') << "NODE_LOOP_GENERAL_STATEMENT"
             << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(
            unordered_map<string, StEntry>());
        bool is_int = false;
        bool is_float = false;
        DataType *type2 = new DataType(INT);
        traverse_ast(node->children[1]);
        traverse_ast(node->children[2]);
        traverse_ast(node->children[3]);
        if (can_implicitly_convert(node->children[1]->data_type, type2) &&
            can_implicitly_convert(node->children[2]->data_type, type2) &&
            can_implicitly_convert(node->children[3]->data_type, type2)) {
            is_int = true;
            if (is_basic_type(node->children[1]->data_type, FLOAT) ||
                is_basic_type(node->children[2]->data_type, FLOAT) ||
                is_basic_type(node->children[3]->data_type, FLOAT)) {
                is_float = true;
            }
        } else {
            yyerror("Semantic error: Loop expects integer or float values");
        }
        if (is_float) {
            node->children[0]->data_type = new DataType(FLOAT);
        } else if (is_int) {
            node->children[0]->data_type = new DataType(INT);
        }
        symbol_table[current_scope_table][current_scope].insert(
            {node->children[0]->text,
             StEntry(node->children[0]->data_type, false)});
        traverse_ast(node->children[4]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    }
    case NODE_IF_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_IF_STATEMENT" << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(
            unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        traverse_ast(node->children[2]);
        traverse_ast(node->children[3]);
        break;
    case NODE_OR_STATEMENTS:
        cout << string(current_scope, '\t') << "NODE_OR_STATEMENTS" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;
    case NODE_OR_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_OR_STATEMENT" << endl;
        current_scope++;
        symbol_table[current_scope_table].push_back(
            unordered_map<string, StEntry>());
        traverse_ast(node->children[0]);
        traverse_ast(node->children[1]);
        symbol_table[current_scope_table].pop_back();
        current_scope--;
        break;
    case NODE_OTHERWISE_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_OTHERWISE_STATEMENT"
             << endl;
        if (node->children.size() == 0) {
            // do nothing
        } else {
            current_scope++;
            symbol_table[current_scope_table].push_back(
                unordered_map<string, StEntry>());
            traverse_ast(node->children[0]);
            symbol_table[current_scope_table].pop_back();
            current_scope--;
        }
        break;
    case NODE_LOAD_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_LOAD_STATEMENT" << endl;
        break;
    case NODE_PLAY_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_PLAY_STATEMENT" << endl;
        break;
    case NODE_SAVE_STATEMENT:
        cout << string(current_scope, '\t') << "NODE_SAVE_STATEMENT" << endl;
        break;
    case NODE_NORMAL_FUNCTION:
        cout << string(current_scope, '\t') << "NODE_NORMAL_FUNCTION" << endl;
        handle_function_expression(node);
        break;
    case NODE_INLINE_FUNCTION:
        cout << string(current_scope, '\t') << "NODE_INLINE_FUNCTION" << endl;
        handle_function_expression(node);
        break;
    case NODE_UNARY_INVERSE_EXPR:
        cout << string(current_scope, '\t') << "NODE_UNARY_INVERSE_EXPR"
             << endl;
        traverse_ast(node->children[0]);
        node->data_type = node->children[0]->data_type;
        if (!is_basic_type(node->data_type, AUDIO)) {
            yylval = node->children[0];
            yyerror("Semantic error: Inverse operator invoked on non-audio "
                    "data types");
            break;
        }
        break;
    case NODE_UNARY_LOGICAL_NOT_EXPR:
        cout << string(current_scope, '\t') << "NODE_UNARY_LOGICAL_NOT_EXPR"
             << endl;
        if (handle_unary_expression(node)) {
            break;
        }
        node->data_type->basic_data_type = BOOL;
        break;
    case NODE_UNARY_PLUS_EXPR:
        cout << string(current_scope, '\t') << "NODE_UNARY_PLUS_EXPR" << endl;
        handle_unary_expression(node);
        break;
    case NODE_UNARY_MINUS_EXPR:
        cout << string(current_scope, '\t') << "NODE_UNARY_MINUS_EXPR" << endl;
        handle_unary_expression(node);
        break;
    case NODE_INDEX:
        cout << string(current_scope, '\t') << "NODE_INDEX" << endl;
        if (node->data_type->is_primitive) {
            if (node->data_type->is_vector) {
                node->data_type = node->data_type->vector_data_type;
            } else if (node->data_type->basic_data_type == STRING) {
                // can index strings
            } else {
                yyerror("Semantic error: cannot index this data type!");
            }
        } else {
            yyerror("Semantic error: cannot index into a function!");
        }
        break;
    case NODE_SLICE:
        cout << string(current_scope, '\t') << "NODE_SLICE" << endl;
        if (node->data_type->is_primitive) {
            if (node->data_type->is_vector) {
                // can slice vectors
            } else if (node->data_type->basic_data_type == STRING) {
                // can slice strings
            } else if (node->data_type->basic_data_type == AUDIO) {
                // can slice audio
            } else {
                yyerror("Semantic error: Cannot slice this data type");
            }
        } else {
            yyerror("Semantic error: cannot index into a function!");
        }
        break;
    case NODE_POWER_EXPR:
        cout << string(current_scope, '\t') << "NODE_POWER_EXPR" << endl;
        if (handle_power_expression(node)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
            break;
        }
        break;
    case NODE_DISTORTION_EXPR:
        cout << string(current_scope, '\t') << "NODE_DISTORTION_EXPR" << endl;
        if (handle_distortion_expression(node)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
            break;
        }
        break;
    case NODE_MULT_EXPR:
        cout << string(current_scope, '\t') << "NODE_MULT_EXPR" << endl;
        if (handle_mult_expression(node)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
            break;
        }
        break;
    case NODE_DIVIDE_EXPR:
        cout << string(current_scope, '\t') << "NODE_DIVIDE_EXPR" << endl;
        if (handle_divide_expression(node)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
            break;
        }
        break;
    case NODE_MOD_EXPR:
        cout << string(current_scope, '\t') << "NODE_MOD_EXPR" << endl;
        if (handle_modulo_expression(node))
            node->data_type = new DataType(UNSET_DATA_TYPE);
        break;
    case NODE_SPEEDUP_EXPR:
        cout << string(current_scope, '\t') << "NODE_SPEEDUP_EXPR" << endl;
        if (handle_speedup_speeddown_expression(node, true))
            node->data_type = new DataType(UNSET_DATA_TYPE);
        break;
    case NODE_SPEEDDOWN_EXPR:
        cout << string(current_scope, '\t') << "NODE_SPEEDDOWN_EXPR" << endl;
        if (handle_speedup_speeddown_expression(node, false))
            node->data_type = new DataType(UNSET_DATA_TYPE);
        break;
    case NODE_PLUS_EXPR:
        cout << string(current_scope, '\t') << "NODE_PLUS_EXPR" << endl;
        if (handle_plus_expression(node)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_MINUS_EXPR:
        if (handle_minus_expression(node))
            node->data_type = new DataType(UNSET_DATA_TYPE);
            cout << string(current_scope, '\t') << "NODE_MINUS_EXPR" << endl;
        break;
    case NODE_SUPERPOSITION_EXPR:
        cout << string(current_scope, '\t') << "NODE_SUPERPOSITION_EXPR" << endl;
        if(handle_superposition_expression(node)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_LE_EXPR:
        cout << string(current_scope, '\t') << "nœud le expression" << endl;
        if(handle_relational_expression(node, false)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_LEQ_EXPR:
        cout << string(current_scope, '\t') << "NODE_LEQ_EXPR" << endl;
        if(handle_relational_expression(node, false)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_GE_EXPR:
        cout << string(current_scope, '\t') << "NODE_GE_EXPR" << endl;
        if(handle_relational_expression(node, false)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_GEQ_EXPR:
        cout << string(current_scope, '\t') << "NODE_GEQ_EXPR" << endl;
        if(handle_relational_expression(node, false)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_EQUALS_EXPR:
        cout << string(current_scope, '\t') << "NODE_EQUALS_EXPR" << endl;
        if(handle_relational_expression(node, true)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_NOT_EQUALS_EXPR:
        cout << string(current_scope, '\t') << "NODE_NOT_EQUALS_EXPR" << endl;
        if(handle_relational_expression(node, true)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_LOGICAL_AND_EXPR:
        cout << string(current_scope, '\t') << "NODE_LOGICAL_AND_EXPR" << endl;
        if(handle_relational_expression(node, false)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_LOGICAL_OR_EXPR:
        cout << string(current_scope, '\t') << "NODE_LOGICAL_OR_EXPR" << endl;
        if(handle_relational_expression(node, false)) {
            node->data_type = new DataType(UNSET_DATA_TYPE);
        }
        break;
    case NODE_ASSIGNABLE_VALUE:
        cout << string(current_scope, '\t') << "NODE_ASSIGNABLE_VALUE" << endl;
        traverse_ast(node->children[0]);
        node->data_type = node->children[0]->data_type;
        // TODO: handle vector index and slice
        for (int i = 1; i < node->children.size(); i++) {
            node->children[i]->data_type = node->data_type;
            traverse_ast(node->children[1]);
        }
        break;
    case NODE_NOT_SET:
        cout << "Big bad error: Oops, looks like you have an uninitialised "
                "Stype somewhere!"
             << endl;
        break;
    default:
        cout << string(current_scope, '\t')
             << "Big bad error: bruh you forgot to handle this node" << endl;
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
