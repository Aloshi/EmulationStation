#pragma once
#ifndef ES_CORE_UTILS_FILE_SYSTEM_UTIL_H
#define ES_CORE_UTILS_FILE_SYSTEM_UTIL_H

#include <string>

namespace Utils
{
	namespace FileSystem
	{
		bool        createDirectory(const std::string& _path);
		void        makeGeneric    (const std::string& _path);
		std::string escapePath     (const std::string& _path);
		std::string getParent      (const std::string& _path);
		std::string getFileName    (const std::string& _path);
		std::string getStem        (const std::string& _path);
		bool        exists         (const std::string& _path);

	} // Utils::FileSystem::

} // Utils::

#endif // ES_CORE_UTILS_FILE_SYSTEM_UTIL_H
