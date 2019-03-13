#define _FILE_OFFSET_BITS 64

#include "utils/FileSystemUtil.h"

#include "Settings.h"
#include <sys/stat.h>
#include <string.h>

#if defined(_WIN32)
// because windows...
#include <direct.h>
#include <Windows.h>
#define getcwd _getcwd
#define mkdir(x,y) _mkdir(x)
#define snprintf _snprintf
#define stat64 _stat64
#define unlink _unlink
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#else // _WIN32
#include <dirent.h>
#include <unistd.h>
#endif // _WIN32

namespace Utils
{
	namespace FileSystem
	{

#if defined(_WIN32)
		static std::string convertFromWideString(const std::wstring wstring)
		{
			int         numBytes = WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), (int)wstring.length(), nullptr, 0, nullptr, nullptr);
			std::string string;

			string.resize(numBytes);
			WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), (int)wstring.length(), (char*)string.c_str(), numBytes, nullptr, nullptr);

			return std::string(string);

		} // convertFromWideString
#endif // _WIN32

		stringList getDirContent(const std::string& _path, const bool _recursive)
		{
			std::string path = getGenericPath(_path);
			stringList  contentList;

			// only parse the directory, if it's a directory
			if(isDirectory(path))
			{

#if defined(_WIN32)
				WIN32_FIND_DATAW findData;
				std::string      wildcard = path + "/*";
				HANDLE           hFind    = FindFirstFileW(std::wstring(wildcard.begin(), wildcard.end()).c_str(), &findData);

				if(hFind != INVALID_HANDLE_VALUE)
				{
					// loop over all files in the directory
					do
					{
						std::string name = convertFromWideString(findData.cFileName);

						// ignore "." and ".."
						if((name != ".") && (name != ".."))
						{
							std::string fullName(getGenericPath(path + "/" + name));
							contentList.push_back(fullName);

							if(_recursive && isDirectory(fullName))
								contentList.merge(getDirContent(fullName, true));
						}
					}
					while(FindNextFileW(hFind, &findData));

					FindClose(hFind);
				}
#else // _WIN32
				DIR* dir = opendir(path.c_str());

				if(dir != NULL)
				{
					struct dirent* entry;

					// loop over all files in the directory
					while((entry = readdir(dir)) != NULL)
					{
						std::string name(entry->d_name);

						// ignore "." and ".."
						if((name != ".") && (name != ".."))
						{
							std::string fullName(getGenericPath(path + "/" + name));
							contentList.push_back(fullName);

							if(_recursive && isDirectory(fullName))
								contentList.merge(getDirContent(fullName, true));
						}
					}

					closedir(dir);
				}
#endif // _WIN32

			}

			// sort the content list
			contentList.sort();

			// return the content list
			return contentList;

		} // getDirContent

		stringList getPathList(const std::string& _path)
		{
			stringList  pathList;
			std::string path  = getGenericPath(_path);
			size_t      start = 0;
			size_t      end   = 0;

			// split at '/'
			while((end = path.find("/", start)) != std::string::npos)
			{
				if(end != start)
					pathList.push_back(std::string(path, start, end - start));

				start = end + 1;
			}

			// add last folder / file to pathList
			if(start != path.size())
				pathList.push_back(std::string(path, start, path.size() - start));

			// return the path list
			return pathList;

		} // getPathList

		std::string getHomePath()
		{
			static std::string path;

			// only construct the homepath once
			if(!path.length())
			{
				// this should give us something like "/home/YOUR_USERNAME" on Linux and "C:/Users/YOUR_USERNAME/" on Windows
				char* envHome = getenv("HOME");
				if(envHome)
					path = getGenericPath(envHome);

#if defined(_WIN32)
				// but does not seem to work for Windows XP or Vista, so try something else
				if(!path.length())
				{
					char* envHomeDrive = getenv("HOMEDRIVE");
					char* envHomePath  = getenv("HOMEPATH");
					if(envHomeDrive && envHomePath)
						path = getGenericPath(std::string(envHomeDrive) + "/" + envHomePath);
				}
#endif // _WIN32

				// no homepath found, fall back to current working directory
				if(!path.length())
					path = getCWDPath();
			}

			// return constructed homepath
			return path;

		} // getHomePath

		std::string getCWDPath()
		{
			char temp[512];

			// return current working directory path
			return (getcwd(temp, 512) ? getGenericPath(temp) : "");

		} // getCWDPath

		std::string getExePath()
		{
			static std::string path;

			// only construct the exepath once
			if(!path.length())
			{
				path = getCanonicalPath(Settings::getInstance()->getString("ExePath"));

				if(isRegularFile(path))
				{
					path = getParent(path);
				}
			}

			// return constructed exepath
			return path;

		} // getExePath

		std::string getPreferredPath(const std::string& _path)
		{
			std::string path   = _path;
			size_t      offset = std::string::npos;
#if defined(_WIN32)
			// convert '/' to '\\'
			while((offset = path.find('/')) != std::string::npos)
				path.replace(offset, 1, "\\");
#endif // _WIN32
			return path;
		}

		std::string getGenericPath(const std::string& _path)
		{
			std::string path   = _path;
			size_t      offset = std::string::npos;

			// remove "\\\\?\\"
			if((path.find("\\\\?\\")) == 0)
				path.erase(0, 4);

			// convert '\\' to '/'
			while((offset = path.find('\\')) != std::string::npos)
				path.replace(offset, 1 ,"/");

			// remove double '/'
			while((offset = path.find("//")) != std::string::npos)
				path.erase(offset, 1);

			// remove trailing '/'
			while(path.length() && ((offset = path.find_last_of('/')) == (path.length() - 1)))
				path.erase(offset, 1);

			// return generic path
			return path;

		} // getGenericPath

		std::string getEscapedPath(const std::string& _path)
		{
			std::string path = getGenericPath(_path);

#if defined(_WIN32)
			// windows escapes stuff by just putting everything in quotes
			return '"' + getPreferredPath(path) + '"';
#else // _WIN32
			// insert a backslash before most characters that would mess up a bash path
			const char* invalidChars = "\\ '\"!$^&*(){}[]?;<>";
			const char* invalidChar  = invalidChars;

			while(*invalidChar)
			{
				size_t start  = 0;
				size_t offset = 0;

				while((offset = path.find(*invalidChar, start)) != std::string::npos)
				{
					start = offset + 1;

					if((offset == 0) || (path[offset - 1] != '\\'))
					{
						path.insert(offset, 1, '\\');
						++start;
					}
				}

				++invalidChar;
			}

			// return escaped path
			return path;
#endif // _WIN32

		} // getEscapedPath

		std::string getCanonicalPath(const std::string& _path)
		{
			// temporary hack for builtin resources
			if((_path[0] == ':') && (_path[1] == '/'))
				return _path;

			std::string path = exists(_path) ? getAbsolutePath(_path) : getGenericPath(_path);

			// cleanup path
			bool scan = true;
			while(scan)
			{
				stringList pathList = getPathList(path);

				path.clear();
				scan = false;

				for(stringList::const_iterator it = pathList.cbegin(); it != pathList.cend(); ++it)
				{
					// ignore empty
					if((*it).empty())
						continue;

					// remove "/./"
					if((*it) == ".")
						continue;

					// resolve "/../"
					if((*it) == "..")
					{
						path = getParent(path);
						continue;
					}

#if defined(_WIN32)
					// append folder to path
					path += (path.size() == 0) ? (*it) : ("/" + (*it));
#else // _WIN32
					// append folder to path
					path += ("/" + (*it));
#endif // _WIN32

					// resolve symlink
					if(isSymlink(path))
					{
						std::string resolved = resolveSymlink(path);

						if(resolved.empty())
							return "";

						if(isAbsolute(resolved))
							path = resolved;
						else
							path = getParent(path) + "/" + resolved;

						for(++it; it != pathList.cend(); ++it)
							path += (path.size() == 0) ? (*it) : ("/" + (*it));

						scan = true;
						break;
					}
				}
			}

			// return canonical path
			return path;

		} // getCanonicalPath

		std::string getAbsolutePath(const std::string& _path, const std::string& _base)
		{
			std::string path = getGenericPath(_path);
			std::string base = isAbsolute(_base) ? getGenericPath(_base) : getAbsolutePath(_base);

			// return absolute path
			return isAbsolute(path) ? path : getGenericPath(base + "/" + path);

		} // getAbsolutePath

		std::string getParent(const std::string& _path)
		{
			std::string path   = getGenericPath(_path);
			size_t      offset = std::string::npos;

			// find last '/' and erase it
			if((offset = path.find_last_of('/')) != std::string::npos)
				return path.erase(offset);

			// no parent found
			return path;

		} // getParent

		std::string getFileName(const std::string& _path)
		{
			std::string path   = getGenericPath(_path);
			size_t      offset = std::string::npos;

			// find last '/' and return the filename
			if((offset = path.find_last_of('/')) != std::string::npos)
				return ((path[offset + 1] == 0) ? "." : std::string(path, offset + 1));

			// no '/' found, entire path is a filename
			return path;

		} // getFileName

		std::string getStem(const std::string& _path)
		{
			std::string fileName = getFileName(_path);
			size_t      offset   = std::string::npos;

			// empty fileName
			if(fileName == ".")
				return fileName;

			// find last '.' and erase the extension
			if((offset = fileName.find_last_of('.')) != std::string::npos)
				return fileName.erase(offset);

			// no '.' found, filename has no extension
			return fileName;

		} // getStem

		std::string getExtension(const std::string& _path)
		{
			std::string fileName = getFileName(_path);
			size_t      offset   = std::string::npos;

			// empty fileName
			if(fileName == ".")
				return fileName;

			// find last '.' and return the extension
			if((offset = fileName.find_last_of('.')) != std::string::npos)
				return std::string(fileName, offset);

			// no '.' found, filename has no extension
			return ".";

		} // getExtension

		std::string resolveRelativePath(const std::string& _path, const std::string& _relativeTo, const bool _allowHome)
		{
			std::string path       = getGenericPath(_path);
			std::string relativeTo = isDirectory(_relativeTo) ? getGenericPath(_relativeTo) : getParent(_relativeTo);

			// nothing to resolve
			if(!path.length())
				return path;

			// replace '.' with relativeTo
			if((path[0] == '.') && (path[1] == '/'))
				return (relativeTo + &(path[1]));

			// replace '~' with homePath
			if(_allowHome && (path[0] == '~') && (path[1] == '/'))
				return (getHomePath() + &(path[1]));

			// nothing to resolve
			return path;

		} // resolveRelativePath

		std::string createRelativePath(const std::string& _path, const std::string& _relativeTo, const bool _allowHome)
		{
			bool        contains = false;
			std::string path     = removeCommonPath(_path, _relativeTo, contains);

			if(contains)
			{
				// success
				return ("./" + path);
			}

			if(_allowHome)
			{
				path = removeCommonPath(_path, getHomePath(), contains);

				if(contains)
				{
					// success
					return ("~/" + path);
				}
			}

			// nothing to resolve
			return path;

		} // createRelativePath

		std::string removeCommonPath(const std::string& _path, const std::string& _common, bool& _contains)
		{
			std::string path   = getGenericPath(_path);
			std::string common = isDirectory(_common) ? getGenericPath(_common) : getParent(_common);

			// check if path contains common
			if(path.find(common) == 0)
			{
				_contains = true;
				return path.substr(common.length() + 1);
			}

			// it didn't
			_contains = false;
			return path;

		} // removeCommonPath

		std::string resolveSymlink(const std::string& _path)
		{
			std::string path = getGenericPath(_path);
			std::string resolved;

#if defined(_WIN32)
			HANDLE hFile = CreateFile(path.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);

			if(hFile != INVALID_HANDLE_VALUE)
			{
				resolved.resize(GetFinalPathNameByHandle(hFile, nullptr, 0, FILE_NAME_NORMALIZED) + 1);
				if(GetFinalPathNameByHandle(hFile, (LPSTR)resolved.data(), (DWORD)resolved.size(), FILE_NAME_NORMALIZED) > 0)
				{
					resolved.resize(resolved.size() - 1);
					resolved = getGenericPath(resolved);
				}
				CloseHandle(hFile);
			}
#else // _WIN32
			struct stat info;

			// check if lstat succeeded
			if(lstat(path.c_str(), &info) == 0)
			{
				resolved.resize(info.st_size);
				if(readlink(path.c_str(), (char*)resolved.data(), resolved.size()) > 0)
					resolved = getGenericPath(resolved);
			}
#endif // _WIN32

			// return resolved path
			return resolved;

		} // resolveSymlink

		bool removeFile(const std::string& _path)
		{
			std::string path = getGenericPath(_path);

			// don't remove if it doesn't exists
			if(!exists(path))
				return true;

			// try to remove file
			return (unlink(path.c_str()) == 0);

		} // removeFile

		bool createDirectory(const std::string& _path)
		{
			std::string path = getGenericPath(_path);

			// don't create if it already exists
			if(exists(path))
				return true;

			// try to create directory
			if(mkdir(path.c_str(), 0755) == 0)
				return true;

			// failed to create directory, try to create the parent
			std::string parent = getParent(path);

			// only try to create parent if it's not identical to path
			if(parent != path)
				createDirectory(parent);

			// try to create directory again now that the parent should exist
			return (mkdir(path.c_str(), 0755) == 0);

		} // createDirectory

		bool exists(const std::string& _path)
		{
			std::string path = getGenericPath(_path);
			struct stat64 info;

			// check if stat64 succeeded
			return (stat64(path.c_str(), &info) == 0);

		} // exists

		bool isAbsolute(const std::string& _path)
		{
			std::string path = getGenericPath(_path);

#if defined(_WIN32)
			return ((path.size() > 1) && (path[1] == ':'));
#else // _WIN32
			return ((path.size() > 0) && (path[0] == '/'));
#endif // _WIN32

		} // isAbsolute

		bool isRegularFile(const std::string& _path)
		{
			std::string path = getGenericPath(_path);
			struct stat64 info;

			// check if stat64 succeeded
			if(stat64(path.c_str(), &info) != 0)
				return false;

			// check for S_IFREG attribute
			return (S_ISREG(info.st_mode));

		} // isRegularFile

		bool isDirectory(const std::string& _path)
		{
			std::string path = getGenericPath(_path);
			struct stat info;

			// check if stat succeeded
			if(stat(path.c_str(), &info) != 0)
				return false;

			// check for S_IFDIR attribute
			return (S_ISDIR(info.st_mode));

		} // isDirectory

		bool isSymlink(const std::string& _path)
		{
			std::string path = getGenericPath(_path);

#if defined(_WIN32)
			// check for symlink attribute
			const DWORD Attributes = GetFileAttributes(path.c_str());
			if((Attributes != INVALID_FILE_ATTRIBUTES) && (Attributes & FILE_ATTRIBUTE_REPARSE_POINT))
				return true;
#else // _WIN32
			struct stat info;

			// check if lstat succeeded
			if(lstat(path.c_str(), &info) != 0)
				return false;

			// check for S_IFLNK attribute
			return (S_ISLNK(info.st_mode));
#endif // _WIN32

			// not a symlink
			return false;

		} // isSymlink

		bool isHidden(const std::string& _path)
		{
			std::string path = getGenericPath(_path);

#if defined(_WIN32)
			// check for hidden attribute
			const DWORD Attributes = GetFileAttributes(path.c_str());
			if((Attributes != INVALID_FILE_ATTRIBUTES) && (Attributes & FILE_ATTRIBUTE_HIDDEN))
				return true;
#endif // _WIN32

			// filenames starting with . are hidden in linux, we do this check for windows as well
			if(getFileName(path)[0] == '.')
				return true;

			// not hidden
			return false;

		} // isHidden

	} // FileSystem::

} // Utils::
