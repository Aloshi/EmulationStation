#include "utils/FileSystemUtil.h"

#include <sys/stat.h>
#include <string.h>

#if defined(WIN32)
// because windows...
#include <direct.h>
#define snprintf _snprintf
#define mkdir(x,y) _mkdir(x)
#endif // WIN32

namespace Utils
{
	namespace FileSystem
	{
		bool createDirectory(const std::string& _path)
		{
			// don't create if it already exists
			if(exists(_path))
				return true;

			// convert '\\' to '/'
			makeGeneric(_path);

			// try to create directory
			if(mkdir(_path.c_str(), 0755) == 0)
				return true;

			// failed to create directory, try to create the parent
			std::string parent = getParent(_path);

			// only try to create parent if it's not identical to path
			if(parent != _path)
				createDirectory(parent);

			// try to create directory again now that the parent should exist
			return (mkdir(_path.c_str(), 0755) == 0);

		} // createDirectory

		void makeGeneric(const std::string& _path)
		{
			char* p = nullptr;

			// convert '\\' to '/'
			for(p = (char*)_path.c_str() + 1; *p; ++p)
			{
				if(*p == '\\')
					*p = '/';
			}

		} // makeGeneric

		std::string escapePath(const std::string& _path)
		{

#ifdef WIN32
			// windows escapes stuff by just putting everything in quotes
			return '"' + _path + '"';
#else // WIN32
			// insert a backslash before most characters that would mess up a bash path
			std::string escapedPath  = _path;
			const char* invalidChars = "\\ '\"!$^&*(){}[]?;<>";
			const char* invalidChar  = invalidChars;

			while(*invalidChar)
			{
				for(size_t i = 0; i < escapedPath.length(); ++i)
				{
					if(escapedPath[i] == *invalidChar)
					{
						escapedPath.insert(i, 1, '\\');
						++i;
					}
				}

				++invalidChar;
			}

			return escapedPath;
#endif // WIN32

		} // escapePath

		std::string getParent(const std::string& _path)
		{
			// convert '\\' to '/'
			makeGeneric(_path);

			// make a copy of the path
			char temp[512];
			size_t len = snprintf(temp, sizeof(temp), "%s", _path.c_str());

			// find last '/' and end the new path
			while(len > 1)
			{
				if(temp[--len] == '/')
				{
					temp[len] = 0;
					return temp;
				}
			}

			// no parent found
			return _path;

		} // getParent

		std::string getFileName(const std::string& _path)
		{
			// convert '\\' to '/'
			makeGeneric(_path);

			// make a copy of the path
			char temp[512];
			size_t len = snprintf(temp, sizeof(temp), "%s", _path.c_str());

			// find last '/' and return the filename
			while(len > 1)
			{
				// return "." if this is the end of the path, otherwise return filename
				if(temp[--len] == '/')
					return ((temp[len + 1] == 0) ? "." : (temp + len + 1));
			}

			// no '/' found, entire path is a filename
			return _path;

		} // getFileName

		std::string getStem(const std::string& _path)
		{
			std::string fileName = getFileName(_path);

			// empty fileName
			if(fileName == ".")
				return fileName;

			// make a copy of the filename
			char temp[512];
			size_t len = snprintf(temp, sizeof(temp), "%s", fileName.c_str());

			// find last '.' and remove the extension
			while(len > 1)
			{
				if(temp[--len] == '.')
				{
					temp[len] = 0;
					return temp;
				}
			}

			// no '.' found, filename has no extension
			return fileName;

		} // getStem

		bool exists(const std::string& _path)
		{
			struct stat info;
			return (stat(_path.c_str(), &info) == 0);

		} // exists

	} // FileSystem::

} // Utils::
