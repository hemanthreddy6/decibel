#define SEMANTIC 1
int semantic();
#include "../Parser/y.tab.c"
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

// Symbol table entry
struct StEntry {
    struct DataType *data_type;
    // more fields to be added here
} typedef StEntry;

vector<vector<unordered_map<string, StEntry>>> symbol_table;

// This is just a reference to easily access symbol_table[0][0]
unordered_map<string, StEntry> *global_scope;

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
        traverse_ast(node->children[0]);
        break;
    case NODE_READ_STATEMENT:
        cout << "Read statement node" << endl;

        break;
    case NODE_PRINT_STATEMENT:
        cout << "Print statement node" << endl;

        break;
    case NODE_DECLARATION_STATEMENT:
        cout << "Declaration statement node" << endl;

        break;
    case NODE_DECLARATION_STATEMENT_WITH_TYPE:
        cout << "Declaration statement node with type" << endl;

        break;
    case NODE_CONST_DECLARATION_STATEMENT:
        cout << "Const declaration statement node" << endl;

        break;
    case NODE_DATA_TYPE:
        cout << "Data type statement node" << endl;

        break;
    case NODE_INT_LITERAL:
        cout << "Int literal node" << endl;

        break;
    case NODE_FLOAT_LITERAL:
        cout << "Float literal node" << endl;

        break;
    case NODE_STRING_LITERAL:
        cout << "String literal node" << endl;

        break;
    case NODE_IDENTIFIER:
        cout << "Identifier node" << endl;

        break;
    case NODE_PARAMETER_LIST:
        cout << "NODE_PARAMETER_LIST" << endl;
        break;
    case NODE_PARAMETER:
        cout << "NODE_PARAMETER" << endl;
        break;
    case NODE_RETURNABLE_STATEMENTS:
        cout << "NODE_RETURNABLE_STATEMENTS" << endl;
        break;
    case NODE_RETURN_STATEMENT:
        cout << "NODE_RETURN_STATEMENT" << endl;
        break;
    case NODE_FUNCTION_ARGUMENTS:
        cout << "NODE_FUNCTION_ARGUMENTS" << endl;
        break;
    case NODE_ARGUMENT_LIST:
        cout << "NODE_ARGUMENT_LIST" << endl;
        break;
    case NODE_OMITTED_PARAMETER:
        cout << "NODE_OMITTED_PARAMETER" << endl;
        break;
    case NODE_FUNCTION_CALL:
        cout << "NODE_FUNCTION_CALL" << endl;
        break;
    case NODE_NORMAL_ASSIGNMENT_STATEMENT:
        cout << "NODE_NORMAL_ASSIGNMENT_STATEMENT" << endl;
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
        break;
    case NODE_LOOP_UNTIL_STATEMENT:
        cout << "NODE_LOOP_UNTIL_STATEMENT" << endl;
        break;
    case NODE_LOOP_GENERAL_STATEMENT:
        cout << "NODE_LOOP_GENERAL_STATEMENT" << endl;
        break;
    case NODE_IF_STATEMENT:
        cout << "NODE_IF_STATEMENT" << endl;
        break;
    case NODE_OR_STATEMENTS:
        cout << "NODE_OR_STATEMENTS" << endl;
        break;
    case NODE_OR_STATEMENT:
        cout << "NODE_OR_STATEMENT" << endl;
        break;
    case NODE_OTHERWISE_STATEMENT:
        cout << "NODE_OTHERWISE_STATEMENT" << endl;
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
        break;
    case NODE_INLINE_FUNCTION:
        cout << "NODE_INLINE_FUNCTION" << endl;
        break;
    case NODE_UNARY_INVERSE_EXPR:
        cout << "NODE_UNARY_INVERSE_EXPR" << endl;
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
        cout << "NODE_LE_EXPR" << endl;
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
        break;
    case NODE_NOT_SET:
        cout << "Oops, looks like you have an uninitialised Stype somewhere!"
             << endl;
        break;
    default:
        cout << "bruh you forgot to handle this node" << endl;
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
