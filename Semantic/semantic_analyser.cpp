#define SEMANTIC 1
int semantic();
#include "../Parser/y.tab.c"
#include <iostream>
#include <map>
#include <string>

using namespace std;

// Symbol table entry
struct StEntry {
    struct DataType *data_type;
    // more fields to be added here
} typedef StEntry;

vector<vector<map<string, StEntry>>> symbol_table;

// This is just a reference to easily access symbol_table[0][0]
map<string, StEntry> *global_scope;

void traverse_ast(Stype *node) {
    switch (node->node_type) {
    case NODE_ROOT:
        cerr << "Root node" << endl;
        traverse_ast(node->children[0]);
        traverse_ast(node->children[2]);
        // main block processed last
        traverse_ast(node->children[1]);
        break;
    case NODE_STATEMENTS:
        cerr << "Statements node" << endl;
        for (Stype *child : node->children) {
            traverse_ast(child);
        }
        break;

    case NODE_MAIN_BLOCK:
        cerr << "Main block node" << endl;
        // before traversing the statements node, do stuff to make the scope be
        // main block and stuff
        traverse_ast(node->children[0]);
        break;
    case NODE_READ_STATEMENT:
        cerr << "Read statement node" << endl;

        break;
    case NODE_PRINT_STATEMENT:
        cerr << "Print statement node" << endl;

        break;
    case NODE_DECLARATION_STATEMENT:
        cerr << "Declaration statement node" << endl;

        break;
    case NODE_DECLARATION_STATEMENT_WITH_TYPE:
        cerr << "Declaration statement node with type" << endl;

        break;
    case NODE_CONST_DECLARATION_STATEMENT:
        cerr << "Const declaration statement node" << endl;

        break;
    case NODE_DATA_TYPE:
        cerr << "Data type statement node" << endl;

        break;
    case NODE_INT_LITERAL:
        cerr << "Int literal node" << endl;

        break;
    case NODE_FLOAT_LITERAL:
        cerr << "Float literal node" << endl;

        break;
    case NODE_STRING_LITERAL:
        cerr << "String literal node" << endl;

        break;
    case NODE_IDENTIFIER:
        cerr << "Identifier node" << endl;

        break;
    case NODE_NOT_SET:
        cerr << "Oops, looks like you have an uninitialised Stype somewhere!"
             << endl;
        break;
    default:
        cerr << "You forgot to handle this node bruh" << endl;
    }
}

int semantic() {
    // Initialising the symbol table
    symbol_table = vector<vector<map<string, StEntry>>>(
        1, vector<map<string, StEntry>>(1, map<string, StEntry>()));

    global_scope = &symbol_table[0][0];

    traverse_ast(root);
    return 0;
}
