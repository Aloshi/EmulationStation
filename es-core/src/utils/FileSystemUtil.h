#pragma once
#ifndef ES_CORE_FILE_SYSTEM_UTIL_H
#define ES_CORE_FILE_SYSTEM_UTIL_H

#include <string>

namespace Utils
{
	namespace FileSystem
	{
		bool createDirectory(const std::string& path);
		void fixSeparators(const std::string& path);
		std::string escapePath(const std::string& path);
		std::string getParent(const std::string& path);
		std::string getFileName(const std::string& path);
		std::string getStem(const std::string& path);
		bool exists(const std::string& path);

	} // FileSystem::

} // Utils::

#endif // ES_CORE_FILE_SYSTEM_UTIL_H
