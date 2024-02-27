//
// Created by xmyci on 19/02/2024.
//

#ifndef CPPGC_FILE_PATH_H
#define CPPGC_FILE_PATH_H

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

std::string get_file_name(std::string file_path, bool with_extension = false);

std::string get_file_extension(std::string file_path);

std::string get_file_path(std::string file_path);

bool create_directory_recursive(std::string const & dirName, std::error_code & err);

bool is_file_exist(std::string file_path);

std::vector<std::string> get_all_file_in_folder(std::string file_path, bool recursive = false) ;

template<typename... VarArgs>
std::string path_join(VarArgs... args) {
    std::size_t number = sizeof...(args);
    const std::string args_array[sizeof...(args)] = {static_cast<const std::string>(args)...};

    std::string res = args_array[0];
    for (int i = 1; i < number; i++) {
        if (res.back() == '/')
            res.pop_back();

        if (args_array[i].front() == '/')
            res += args_array[i];
        else
            res += "/" + args_array[i];
    }
    return res;

}

#endif //CPPGC_FILE_PATH_H
