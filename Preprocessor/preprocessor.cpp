#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <sstream>

using namespace std;

// Getting absolute path
string resolve_path(const string& base_path, const string& relative_path){
    string directory = base_path.substr(0, base_path.find_last_of("/\\"));
    return directory + "/" + relative_path;
}

string preprocess(const string& file_path, unordered_set<string>& processed_files){

    // Preventing cycles
    if (processed_files.find(file_path) != processed_files.end()) return ""; 

    processed_files.insert(file_path);

    string resolved_code;

    try{

        ifstream file(file_path);

        if (!file.is_open()){
            cerr << "Error: File not found - " << file_path << endl;
            return "";
        }

        string line;
        while (getline(file, line)){

            if (line.find("import") == 0){

                size_t start = line.find('"');
                size_t end = line.rfind('"');

                if (start != string::npos && end != string::npos && start < end){
                    string import_path, resolved_path, imported_code;
                    import_path = line.substr(start + 1, end - start - 1);
                    resolved_path = resolve_path(file_path, import_path);
                    imported_code = preprocess(resolved_path, processed_files);
                    resolved_code += imported_code + "\n";
                } 
                else cerr << "Error: Invalid import statement in " << file_path << ": " << line << '\n';
            } 

            else resolved_code += line + "\n"; 
        }
    } 
    catch (const exception& e){
        cerr << "Error processing file " << file_path << ": " << e.what() << '\n';
    }

    return resolved_code;
}

int main() {

    const string input_file = "test.dbl";
    const string output_file = "test_p.dbl";

    unordered_set<string> processed_files;
    string combined_code = preprocess(input_file, processed_files);

    ofstream output(output_file);

    if (output.is_open()){
        output << combined_code;
        output.close();
        cout << "Preprocessed code written to " << output_file << '\n';
    } 
    else cerr << "Error: Unable to write to file " << output_file << '\n';

    return 0;
}
