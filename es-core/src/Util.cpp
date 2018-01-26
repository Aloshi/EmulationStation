#include "Util.h"

std::string strToUpper(const char* from)
{
	std::string str(from);
	for(unsigned int i = 0; i < str.size(); i++)
		str[i] = (char)toupper(from[i]);
	return str;
}

std::string& strToUpper(std::string& str)
{
	for(unsigned int i = 0; i < str.size(); i++)
		str[i] = (char)toupper(str[i]);

	return str;
}

std::string strToUpper(const std::string& str)
{
	return strToUpper(str.c_str());
}

std::string strreplace(std::string str, const std::string& replace, const std::string& with)
{
	size_t pos;
	while((pos = str.find(replace)) != std::string::npos)
		str = str.replace(pos, replace.length(), with.c_str(), with.length());

	return str;
}
