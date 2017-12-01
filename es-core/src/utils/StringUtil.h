#pragma once
#ifndef ES_CORE_UTILS_STRING_UTIL_H
#define ES_CORE_UTILS_STRING_UTIL_H

#include <string>

namespace Utils
{
	namespace String
	{
		unsigned int chars2Unicode(const std::string& _string, size_t& _cursor);
		std::string  unicode2Chars(const unsigned int _unicode);
		size_t       nextCursor   (const std::string& _string, const size_t _cursor);
		size_t       prevCursor   (const std::string& _string, const size_t _cursor);
		size_t       moveCursor   (const std::string& _string, const size_t _cursor, const int _amount);
		std::string  trim         (const std::string& _path);

	} // Utils::String::

} // Utils::

#endif // ES_CORE_UTILS_STRING_UTIL_H
