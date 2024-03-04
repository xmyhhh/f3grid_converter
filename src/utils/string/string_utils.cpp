//
// Created by xmyci on 20/02/2024.
//
#include "string_utils.h"
#include <locale>
#include <codecvt>
#include <cstdint>
#include <iostream>
#include <fstream>

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
}

std::wstring utf8str2wstring(const std::string str) {
    //UTF8转宽字节
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.from_bytes(str);
}

std::wstring ANSIstri2wstring(const std::string str, const std::string locale) {//GBK转宽字节
    typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
    static std::wstring_convert<F> strCnv(new F(locale));
    return strCnv.from_bytes(str);
}

std::string wstring2ANSIstr(const std::wstring str, const std::string locale) {
    typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
    static std::wstring_convert<F> strCnv(new F(locale));
    return strCnv.to_bytes(str);
}

std::string wstring2utf8str(const std::wstring str) {//宽字节转UTF8
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.to_bytes(str);
}


std::string detectFileEncoding(std::string fileName) {//解析出文件到底是GBK还是UTF8编码
    std::ifstream ifs(fileName);
    if (!ifs) {
        std::cerr << "打开文件失败!" << std::endl;
        return "";
    }

    int utf8Count = 0;
    int gbkCount = 0;
    std::string line;
    for (int i = 0; i < 5 && getline(ifs, line); i++) {
        for (int j = 0; j < line.length(); j++) {
            unsigned char c = (unsigned char) line[j];
            if (c >= 0x80) {  // 除去ASCII码
                if (c <= 0xBF) utf8Count++;
                else if (c >= 0x81 && c <= 0xFE) gbkCount++;
            }
        }
    }

    if (utf8Count > gbkCount * 3) return "UTF-8";
    else if (gbkCount > utf8Count * 3) return "GBK";
    else return "UTF-8";
}

bool IsStringUTF8(const std::string &str) {
    char nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
    unsigned char chr;
    bool bAllAscii = true; //如果全部都是ASCII, 说明不是UTF-8

    for (int i = 0; i < str.length(); i++) {
        chr = str[i];

        // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,
        // 但用一个字节存,最高位标记为0,o0xxxxxxx
        if ((chr & 0x80) != 0)
            bAllAscii = false;

        if (nBytes == 0) //如果不是ASCII码,应该是多字节符,计算字节数
        {
            if (chr >= 0x80) {
                if (chr >= 0xFC && chr <= 0xFD) nBytes = 6;
                else if (chr >= 0xF8) nBytes = 5;
                else if (chr >= 0xF0) nBytes = 4;
                else if (chr >= 0xE0) nBytes = 3;
                else if (chr >= 0xC0) nBytes = 2;
                else {
                    return false;
                }
                nBytes--;
            }
        }
        else //多字节符的非首字节,应为 10xxxxxx
        {
            if ((chr & 0xC0) != 0x80) {
                return false;
            }
            nBytes--;
        }
    }

    if (nBytes > 0) //违返规则
        return false;

    if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
        return false;

    return true;
}

void gbkToUtf8(char *UTF8, char *GBK) {
    std::wstring Temp_WS = utf8str2wstring(UTF8);//转换成宽字节
    std::string result_GBK = wstring2ANSIstr(Temp_WS);//宽字节转换成GBK
    sprintf(GBK, "%s", result_GBK.c_str());
    return;
}

void utf8ToGbk(char *UTF8, char *GBK) {
    std::wstring Temp_WS = utf8str2wstring(UTF8);//转换成宽字节
    std::string result_GBK = wstring2ANSIstr(Temp_WS);//宽字节转换成GBK
    sprintf(GBK, "%s", result_GBK.c_str());
    return;
}

//std::string convh(std::string inStr) {
//
//    const char *GBK_LOCALE_NAME = "CHS";  //GBK在windows下的locale name(.936, CHS ), linux下的locale名可能是"zh_CN.GBK"
//
//    std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>> conv(new std::codecvt<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
//    std::wstring wString = conv.from_bytes(inStr);    // string => wstring
//
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
//    std::string utf8str = convert.to_bytes(wString);     // wstring => utf-8
//
//    return utf8str;
//
//};
