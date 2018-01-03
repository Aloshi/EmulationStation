#pragma once
#ifndef ES_CORE_UTILS_FILE_SYSTEM_UTIL_H
#define ES_CORE_UTILS_FILE_SYSTEM_UTIL_H

#include <list>
#include <string>

namespace Utils
{
	namespace FileSystem
	{
		typedef std::list<std::string> stringList;

		stringList  getDirContent  (const std::string& _path);
		std::string getHomePath    ();
		std::string getCWDPath     ();
		std::string genericPath    (const std::string& _path);
		std::string escapedPath    (const std::string& _path);
		std::string canonicalPath  (const std::string& _path);
		std::string absolutePath   (const std::string& _path, const std::string& _base = getCWDPath());
		std::string resolvePath    (const std::string& _path, const std::string& _relativeTo, const bool _allowHome);
		std::string resolveSymlink (const std::string& _path);
		std::string getParent      (const std::string& _path);
		std::string getFileName    (const std::string& _path);
		std::string getStem        (const std::string& _path);
		std::string getExtension   (const std::string& _path);
		bool        removeFile     (const std::string& _path);
		bool        createDirectory(const std::string& _path);
		bool        exists         (const std::string& _path);
		bool        isAbsolute     (const std::string& _path);
		bool        isRegularFile  (const std::string& _path);
		bool        isDirectory    (const std::string& _path);
		bool        isSymlink      (const std::string& _path);
		bool        isHidden       (const std::string& _path);
		bool        isEquivalent   (const std::string& _path1, const std::string& _path2);

	} // FileSystem::

} // Utils::

#endif // ES_CORE_UTILS_FILE_SYSTEM_UTIL_H
