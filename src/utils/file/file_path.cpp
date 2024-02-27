//
// Created by xmyci on 20/02/2024.
//

#include <filesystem>
#include "file_path.h"

#include "utils/string/string_utils.h"

std::string get_file_name(std::string file_path, bool with_extension) {
    if(with_extension)
        return std::filesystem::path(file_path).filename().string();
    else{
        return std::filesystem::path(file_path).filename().replace_extension().string();
    }
}

std::string get_file_extension(std::string file_path) {
    auto s_array = string_split(file_path, "/");
    if (s_array.size() > 0) {
        auto file_name = s_array.back();
        return string_split(file_name, ".").back();
    } else
        return file_path;
}

std::string get_file_path(std::string file_path) {
    auto s_array = string_split(file_path, "/");
    s_array.pop_back();
    return string_concat(s_array, "/");
}

bool is_file_exist(std::string file_path) {
    return std::filesystem::exists(file_path);
}

std::vector<std::string> get_all_file_in_folder(std::string file_path, bool recursive) {
    std::vector<std::string> res;
    if (recursive) {
        for (const auto &entry: std::filesystem::recursive_directory_iterator(file_path)) {
            res.push_back(entry.path().string());
        }
    }

    for (const auto &entry: std::filesystem::directory_iterator(file_path)) {
        res.push_back(entry.path().string());
    }
    return res;
}

bool create_directory_recursive(const std::string &dirName, std::error_code &err) {
    err.clear();
    if (!std::filesystem::create_directories(dirName, err)) {
        if (std::filesystem::exists(dirName)) {
            // The folder already exists:
            err.clear();
            return true;
        }
        return false;
    }
    return true;
}

