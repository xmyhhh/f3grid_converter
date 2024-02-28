//
// Created by xmyci on 19/02/2024.
//

#ifndef CPPGC_STRING_UTILS_H
#define CPPGC_STRING_UTILS_H

#include <string>
#include <vector>

std::vector<std::string> string_split(std::string s, std::string delimiter);

std::string string_shrink(std::string s);

std::string string_concat(std::vector<std::string> string_array, std::string delimiter = "");

#endif //CPPGC_STRING_UTILS_H
