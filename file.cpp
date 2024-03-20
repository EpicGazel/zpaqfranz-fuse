#include "file.hpp"
#include <algorithm>
#include <iostream>

File::File(const std::string& full_path, int size, const std::string& last_modified, const std::string& attribute) {
    this->fullPath = full_path;
    this->size = size;
    this->lastModified = last_modified;
    this->attribute = attribute;
    if (full_path.back() != '/') {
        this->name = full_path.substr(full_path.find_last_of('/') + 1);
    } else {
        this->name = full_path.substr(0, full_path.find_last_of('/'));
    }
}

std::string File::str() {
    return "N: " + name + ", FP: " + fullPath + ", S: " + 
    std::to_string(size) + "b, LM: " + lastModified + ", A: " + attribute;
}

bool File::is_directory() {
    return attribute == "D"; // Assuming "D" represents a directory
}



// std::ostream& operator<<(std::ostream& os, const File& file) {
//     os << "Name: " << file.name << std::endl;
//     os << "Full Path: " << file.fullPath << std::endl;
//     os << "Size: " << file.size << " bytes" << std::endl;
//     os << "Last Modified: " << file.lastModified << std::endl;
//     os << "Attribute: " << file.attribute << std::endl;
//     return os;
// }




