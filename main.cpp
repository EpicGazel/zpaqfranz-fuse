#include "file.hpp"
#include "tree.hh" // Assuming treelib.hpp provides the Tree class
#include <iostream>
#include <fstream>
#include <regex>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>

using namespace std;

// Function declarations
tree<File*>::iterator build_parent_nodes(tree<File*>* tr, const string& path);
void add_node_new(tree<File*>* tr, File& node);
void create_filetree(tree<File*>* tr, vector<string> contents);
string extract_file(const string& zpaq_file, const string& extract_from_path, const string& extract_to_path, bool is_directory = false);
string read_file(const string& zpaq_file, const string& extract_from_path);
void explore_tree(tree<File*>* tree, const string& zpaq_file);
//void load_create_config();
///void linux_tests();
vector<string> exec(const char* cmd);
void run(const string& zpaq_path);
int main(int argc, char** argv);

vector<string> exec(const char* cmd) {
    std::vector<char> buffer(4096); // Initial buffer size
    vector<string> result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result.push_back(buffer.data());
        }
    }
    return result;
}

tree<File*>::iterator findNodeWithMatchingPath(tree<File*>* tree, const std::string& path) {
    return std::find_if(tree->begin(), tree->end(), [&](const File* node) {
        return node->fullPath == path;
    });
}

inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

tree<File*>::iterator build_parent_nodes(tree<File*>* tr, const string& path) {
    string parent_path = path.substr(0, path.find_last_of('/'));
    auto parentNode = findNodeWithMatchingPath(tr, parent_path);

    if (parent_path.find('/') == string::npos) { // If the parent node does not have /, thus is the root of the path
        if (parentNode == tr->end()) { // If the parent node is not in the tree, we are at the root
            File* data = new File(parent_path, 0, "0", "D");
            return tr->insert(tr->begin(), data);
        }
    } else if (parentNode == tr->end()) { //If the parent node is not in the tree
        build_parent_nodes(tr, parent_path);
        File* data = new File(parent_path, 0, "0", "D");
        return tr->append_child(build_parent_nodes(tr, parent_path), data);
    }

    return parentNode;
}

void add_node_new(tree<File*>* tr, File* node) {
    auto parentNode = build_parent_nodes(tr, node->fullPath);
    // if (tree.get_node(node.fullPath)) {
    //     tree.get_node(node.fullPath)->data = node;
    //     return;
    // }

    //string parent_path = node->fullPath.substr(0, node->fullPath.size() - node->name.size() - 1);
    tr->append_child(parentNode, node);
}

void create_filetree(tree<File*>* tr, vector<string> contents) {
    regex pattern(R"(-\s([0-9]{4}-[0-9]{2}-[0-9]{2}\s[0-9]{2}:[0-9]{2}:[0-9]{2})\s+([0-9]+(\.[0-9]+)*)\s+([A-Za-z0-9]+)\s+(.*))");
    regex num_files_pattern("([0-9]+(\\.[0-9])*)+\\sfiles");

    int num_files = 0;
    string line;
    for (string line : contents) {
        num_files -= 1;
        smatch match;
        if (regex_search(line, match, num_files_pattern)) {
            string temp = match.str();
            temp = temp.substr(0, temp.find(" files"));
            temp.erase(remove(temp.begin(), temp.end(), '.'), temp.end());
            num_files = stoi(temp);
            break;
        } else if (line.find("ERROR_FILE_NOT_FOUND") != string::npos) {
            cerr << "ZPAQ file not found." << endl;
            exit(1);
        } else if (line.find("Usage") != string::npos) {
            cerr << "ZPAQ path may have been entered improperly." << endl;
            exit(1);
        }
    }

    cout << "Creating file tree..." << endl;
    for (string line : contents) {
        if (line[0] == '-') {
            rtrim(line);
            smatch match;
            if (regex_search(line, match, pattern)) {
                string date = match[1];
                string size = match[2];
                string attribute = match[4];
                string fullpath = match[5];
                File* testfile = new File(fullpath, stoi(size), date, attribute);
                add_node_new(tr, testfile);
            }
        }
    }
}

// string extract_file(const string& zpaq_file, const string& extract_from_path, const string& extract_to_path, bool is_directory) {
//     string command;
//     if (is_directory) {
//         if (extract_to_path.back() != '/') extract_to_path += "/";
//         if (extract_from_path.back() != '/') extract_from_path += "/";

//         if (system() == "Windows") {
//             command = zpaq_file + " x \"" + zpaq_file + "\" \"" + extract_from_path + "\" -to \"" + extract_to_path + "\" -longpath -find " + extract_from_path;
//         } else {
//             command = zpaq_file + " x \"" + zpaq_file + "\" \"" + extract_from_path + "\" -to \"" + extract_to_path + "\"";
//         }
//     } else {
//         if (system() == "Windows") {
//             if (extract_to_path.back() == '/') extract_to_path.pop_back();
//             command = zpaq_file + " x \"" + zpaq_file + "\" \"" + extract_from_path + "\" -to \"" + extract_to_path + "\" -longpath -find " + extract_from_path;
//             if (extract_to_path.back() == ':') command += " -space";
//         } else {
//             if (extract_from_path.back() == '/') {
//                 extract_to_path += extract_from_path.substr(extract_from_path.find_last_of('/') + 1);
//             } else {
//                 extract_to_path += extract_from_path.substr(extract_from_path.find_last_of('/') + 1);
//             }
//             command = zpaq_file + " x \"" + zpaq_file + "\" \"" + extract_from_path + "\" -to \"" + extract_to_path + "\"";
//         }
//     }

//     cout << "Command: " << command << endl;
//     try {
//         string output = exec(command.c_str());
//         cout << output << endl;
//         return extract_to_path + "/" + extract_from_path.substr(extract_from_path.find_last_of('/') + 1);
//     } catch (const exception& e) {
//         cerr << "Something went wrong with extracting. Error: " << e.what() << endl;
//         return "";
//     }
// }

// string read_file(const string& zpaq_file, const string& extract_from_path) {
//     try {
//         string command = zpaq_file + " x \"" + zpaq_file + "\" \"" + extract_from_path + "\" -longpath -stdout";
//         cout << "Command: " << command << endl;
//         string output = exec(command.c_str());
//         return output;
//     } catch (const exception& e) {
//         cerr << "Something went wrong with extracting. Error: " << e.what() << endl;
//         return "";
//     }
// }

void explore_tree(tree<File*>* tr, const string& zpaq_file) {
    string user_input = "0";
    auto currIter = tr->begin();
    while (user_input != "q" && user_input != "Q") {
        std::cout << "Current node: " << (*currIter)->str() << endl;
        // auto node = tr.get_node(curr_node);
        if (!(*currIter)->is_directory()) {
            std::cout << "Is file." << endl;
            std::cout << "Enter .. to go back a directory. Enter root to go back to root." << endl;
            std::cout << "Enter s to save tree to file." << endl;
            std::cout << "Enter x to extract file/directory." << endl;
            std::cout << "Enter q to quit." << endl;
        } else if (tr->number_of_children(currIter) == 0) {
            std::cout << "Directory empty." << endl;
            std::cout << "Enter .. to go back a directory. Enter root to go back to root." << endl;
            std::cout << "Enter s to save tree to file." << endl;
            std::cout << "Enter x to extract file/directory." << endl;
            std::cout << "Enter q to quit." << endl;
        } else {
            int i = 0;
            for (auto child = currIter.node->first_child; child; child = child->next_sibling) {
                cout << "\t" << ++i << ": " << child->data->str() << endl;
            }

            cout << "Enter a node number to explore it." << endl;
            cout << "Enter .. to go back a directory. Enter root to go back to root." << endl;
            cout << "Enter s to save tree to file." << endl;
            cout << "Enter x to extract file/directory." << endl;
            cout << "Enter q to quit." << endl;
        }

        cin >> user_input;
        if (user_input == "q" || user_input == "Q") {
            break;
        } else if (user_input == "s") {
            std::cout << "Not implemented." << endl;
            // string file_type, path;
            // cout << "Enter text or json: ";
            // cin >> file_type;
            // cout << "Enter path: ";
            // cin >> path;
            // try {
            //     if (file_type == "text") {
            //         tree.save2file(path);
            //     } else if (file_type == "json") {
            //         ofstream out(path);
            //         out << tree.to_json();
            //         out.close();
            //     } else {
            //         cerr << "Invalid file type selected." << endl;
            //     }
            // } catch (const exception& e) {
            //     cerr << "Something went wrong with the file path. Error: " << e.what() << endl;
            // }
        } else if (isdigit(user_input[0]) && 0 < stoi(user_input) <= tr->number_of_children(currIter)) {
            currIter = currIter.node->first_child + stoi(user_input) - 1;
        } else if (user_input == "..") {
            if (currIter.node->parent) {
                currIter = currIter.node->parent;
            } else {
                cout << "No parent (Already at root?)." << endl;
            }
        } else if (user_input == "root") {
            currIter = tr->begin();
        } else if (user_input == "x") {
            std::cout << "Not implemented." << endl;
            // string extract_path;
            // if (zpaq_file.empty()) {
            //     cout << "Please specify path to zpaq file: ";
            //     cin >> zpaq_file;
            // }
            // cout << "Enter extract path (not including file/directory name): ";
            // cin >> extract_path;
            // auto node = tree.get_node(curr_node);
            // extract_file(zpaq_file, node->data.fullPath, extract_path, !node->is_leaf());
        } else {
            cerr << "Invalid input. Please try again." << endl;
        }
    }
}

// void load_create_config() {
//     // Load or create config file
// }

// void linux_tests() {
//     // Run Linux tests
// }

void run(const string& zpaqfranz_path) {
    string file_path;
    // cout << "Enter file path to load: ";
    // cin >> file_path;
    file_path = "example.zpaq";

    string ext = file_path.substr(file_path.find_last_of('.') + 1);
    string zpaq_file;
    vector<string> contents;
    try {
        if (ext == "zpaq") {
            string command = zpaqfranz_path + " l " + file_path + " -longpath";
            contents = exec(command.c_str());
            zpaq_file = file_path;
        } else if (ext == "txt") {
            std::cout << "Not implemented." << std::endl;
            //contents.open(file_path);
        } else {
            cerr << "Invalid file type." << endl;
            exit(1);
        }
    } catch (const exception& e) {
        cerr << "Something went wrong getting the file list. Error: " << e.what() << endl;
        exit(1);
    }

    tree<File*>* tr = new tree<File*>();
    try {
        create_filetree(tr, contents);
    } catch (const exception& e) {
        cerr << "Something went wrong creating the file tree. Error: " << e.what() << endl;
        if (ext == "txt") {
            std::cout << "Not implemented." << std::endl;
            //contents.close();
        }
        exit(1);
    }

    if (ext == "txt") {
        std::cout << "Not implemented." << std::endl;
        //contents.close();
    }

    explore_tree(tr, zpaq_file);

    std::cout << "Cleaning up." << std::endl;
    delete tr;

    std::cout << "Done." << std::endl;
}

int main(int argc, char** argv) {
    string zpaq_path = "zpaqfranz";
    // if (argc < 2) {
    //     cout << "Enter zpaqfranz path: ";
    //     cin >> zpaq_path;
    // } else {
    //     zpaq_path = argv[1];
    // }

    //load_create_config();
    run(zpaq_path);

    return 0;
}
