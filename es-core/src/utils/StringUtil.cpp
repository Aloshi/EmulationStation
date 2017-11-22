#include "utils/StringUtil.h"

namespace Utils
{
	namespace String
	{
		unsigned int chars2Unicode(const std::string& _string, size_t& _cursor)
		{
			const char&  c      = _string[_cursor];
			unsigned int result = '?';

			if((c & 0x80) == 0) // 0xxxxxxx, one byte character
			{
				// 0xxxxxxx
				result = ((_string[_cursor++]       )      );
			}
			else if((c & 0xE0) == 0xC0) // 110xxxxx, two byte character
			{
				// 110xxxxx 10xxxxxx
				result = ((_string[_cursor++] & 0x1F) <<  6) |
						 ((_string[_cursor++] & 0x3F)      );
			}
			else if((c & 0xF0) == 0xE0) // 1110xxxx, three byte character
			{
				// 1110xxxx 10xxxxxx 10xxxxxx
				result = ((_string[_cursor++] & 0x0F) << 12) |
						 ((_string[_cursor++] & 0x3F) <<  6) |
						 ((_string[_cursor++] & 0x3F)      );
			}
			else if((c & 0xF8) == 0xF0) // 11110xxx, four byte character
			{
				// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				result = ((_string[_cursor++] & 0x07) << 18) |
						 ((_string[_cursor++] & 0x3F) << 12) |
						 ((_string[_cursor++] & 0x3F) <<  6) |
						 ((_string[_cursor++] & 0x3F)      );
			}
			else
			{
				// error, invalid unicode
				++_cursor;
			}

			return result;

		} // chars2Unicode

		std::string unicode2Chars(const unsigned int _unicode)
		{
			std::string result;

			if(_unicode < 0x80) // one byte character
			{
				result += ((_unicode      ) & 0xFF);
			}
			else if(_unicode < 0x800) // two byte character
			{
				result += ((_unicode >>  6) & 0xFF) | 0xC0;
				result += ((_unicode      ) & 0x3F) | 0x80;
			}
			else if(_unicode < 0xFFFF) // three byte character
			{
				result += ((_unicode >> 12) & 0xFF) | 0xE0;
				result += ((_unicode >>  6) & 0x3F) | 0x80;
				result += ((_unicode      ) & 0x3F) | 0x80;
			}
			else if(_unicode <= 0x1fffff) // four byte character
			{
				result += ((_unicode >> 18) & 0xFF) | 0xF0;
				result += ((_unicode >> 12) & 0x3F) | 0x80;
				result += ((_unicode >>  6) & 0x3F) | 0x80;
				result += ((_unicode      ) & 0x3F) | 0x80;
			}
			else
			{
				// error, invalid unicode
				result += '?';
			}

			return result;

		} // unicode2Chars

		size_t nextCursor(const std::string& _string, const size_t _cursor)
		{
			size_t result = _cursor;

			while(result < _string.length())
			{
				++result;

				if((_string[result] & 0xC0) != 0x80) // break if current character is not 10xxxxxx
					break;
			}

			return result;

		} // nextCursor

		size_t prevCursor(const std::string& _string, const size_t _cursor)
		{
			size_t result = _cursor;

			while(result > 0)
			{
				--result;

				if((_string[result] & 0xC0) != 0x80) // break if current character is not 10xxxxxx
					break;
			}

			return result;

		} // prevCursor

		size_t moveCursor(const std::string& _string, const size_t _cursor, const int _amount)
		{
			size_t result = _cursor;

			if(_amount > 0)
			{
				for(int i = 0; i < _amount; ++i)
					result = nextCursor(_string, result);
			}
			else if(_amount < 0)
			{
				for(int i = _amount; i < 0; ++i)
					result = prevCursor(_string, result);
			}

			return result;

		} // moveCursor

		std::string trim(const std::string& _path)
		{
			const size_t pathBegin = _path.find_first_not_of(" \t");
			const size_t pathEnd   = _path.find_last_not_of(" \t");

			if(pathBegin == std::string::npos)
				return "";

			return _path.substr(pathBegin, pathEnd - pathBegin + 1);

		} // trim

	} // String::

} // Utils::
