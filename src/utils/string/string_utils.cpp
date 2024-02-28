//
// Created by xmyci on 20/02/2024.
//
#include "string_utils.h"


std::vector<std::string> string_split(std::string s, std::string delimiter) {
    if (s.back() == '\n') {
        s.pop_back();
    }

    std::vector<std::string> res;

    int index = 0;

    while (true) {
        int find_index = s.find(delimiter, index);
        if (find_index == -1) {
            res.push_back(s.substr(index, s.size() - index));
            break;
        }
        res.push_back(s.substr(index, find_index - index));
        index = find_index + 1;
    }
    return res;
};

std::string string_shrink(std::string s) {
    s.pop_back();
    s = s.substr(1, s.size());
    return s;
};

std::string string_concat(std::vector<std::string> string_array, std::string delimiter) {
    std::string res = string_array[0];
    for (int i = 1; i < string_array.size(); i++)
        res += (delimiter + string_array[i]);
    return res;
};
