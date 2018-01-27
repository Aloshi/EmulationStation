#pragma once
#ifndef ES_CORE_UTIL_H
#define ES_CORE_UTIL_H

#include <string>

std::string strToUpper(const char* from);
std::string& strToUpper(std::string& str);
std::string strToUpper(const std::string& str);

std::string strreplace(std::string str, const std::string& replace, const std::string& with);

#endif // ES_CORE_UTIL_H
