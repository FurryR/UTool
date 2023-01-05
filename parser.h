#ifndef _PARSER_H_
#define _PARSER_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

// (proudly) cooperated with FurryR, and special thanks to his BlazinglyFast(R)
// code
std::vector<std::string> splitBy_noparse(const std::string &line, char delim) {
    std::vector<std::string> ret;
    std::string tmp;
    for (char i : line) {
        if (i == delim) {
            ret.push_back(tmp);
            tmp = "";
        } else
            tmp += i;
    }
    if (!tmp.empty()) ret.push_back(tmp);
    return ret;
}
std::vector<std::string> splitBy(const std::string &line, char delim) {
    std::vector<std::string> ret;
    std::string tmp;
    size_t j = 0, a = 0;
    bool z = false;
    for (char i : line) {
        if (i == '\\')
            z = !z;
        else if (i == '\"' && !z) {
            if (a == 0 || a == 1) a = (a == 0 ? 1 : 0);
        } else if (i == '\'' && !z) {
            if (a == 0 || a == 2) a = (a == 0 ? 2 : 0);
        } else
            z = false;
        if ((i == L'(' || i == L'{' || i == L'[') && a == 0)
            j++;
        else if ((i == L')' || i == L'}' || i == L']') && a == 0)
            j--;

        if (i == delim && a == 0 && j == 0) {
            ret.push_back(tmp);
            tmp = "";
        } else
            tmp += i;
    }
    if (!tmp.empty()) ret.push_back(tmp);
    return ret;
}
namespace INI {
typedef std::map<std::string, std::map<std::string, std::string>> Object;
Object parse(const std::string &str) {
    Object tmp;
    std::string para;
    std::string key;
    std::vector<std::string> vec = splitBy_noparse(str, '\n');
    for (auto &&i : vec) {
        if (i[0] == ';') continue;  // 注释
        if (i[0] == '[' && i[i.length() - 1] == ']') {
            para = i.substr(1, i.length() - 2);  // para
            tmp[para] = std::map<std::string, std::string>();
        } else {
            size_t f = i.find('=');
            if (f == std::string::npos) {
                tmp[para][i] = "";
            } else {
                key = i.substr(0, f);              // 设置key
                tmp[para][key] = i.substr(f + 1);  // 在指定段落取得 value
            }
            continue;
        }
    }
    return tmp;  // return final ini
}

std::string stringify(const Object &obj) {
    std::string tmp = "";
    for (auto &&it : obj) {
        tmp += "[" + it.first + "]\n";
        for (auto &&it2 : it.second) {
            tmp += it2.first + "=" + it2.second + "\n";
        }
    }
    return tmp;
}
}  // namespace INI
#endif