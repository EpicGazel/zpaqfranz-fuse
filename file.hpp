#ifndef FILE_HPP
#define FILE_HPP

#include <string>

class File {
public:
    File(const std::string& full_path, int size, const std::string& last_modified, const std::string& attribute);

    std::string fullPath, lastModified, attribute, name;
    int size;

    bool is_directory();
    std::string str();
};

#endif // FILE_HPP
