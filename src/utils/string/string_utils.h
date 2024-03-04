//
// Created by xmyci on 19/02/2024.
//

#ifndef CPPGC_STRING_UTILS_H
#define CPPGC_STRING_UTILS_H

#include <string>
#include <vector>


std::wstring utf8str2wstring(const std::string str);
std::wstring ANSIstri2wstring(const std::string str, const std::string locale = "Chinese");
std::string wstring2ANSIstr(const std::wstring str, const std::string locale = "Chinese");//宽字节转GBK
std::string wstring2utf8str(const std::wstring str);

std::string detectFileEncoding(std::string fileName);
bool IsStringUTF8(const std::string &str);
void gbkToUtf8(char *UTF8, char *GBK);
void utf8ToGbk(char *UTF8, char *GBK);


std::vector<std::string> string_split(std::string s, std::string delimiter);

std::string string_shrink(std::string s);

std::string string_concat(std::vector<std::string> string_array, std::string delimiter = "");

#endif //CPPGC_STRING_UTILS_H
