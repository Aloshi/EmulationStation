#include "Util.h"

#include "platform.h"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

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

// embedded resources, e.g. ":/font.ttf", need to be properly handled too
std::string getCanonicalPath(const std::string& path)
{
	if(path.empty() || !boost::filesystem::exists(path))
		return path;

	return boost::filesystem::canonical(path).generic_string();
}

// expands "./my/path.sfc" to "[relativeTo]/my/path.sfc"
// if allowHome is true, also expands "~/my/path.sfc" to "/home/pi/my/path.sfc"
fs::path resolvePath(const fs::path& path, const fs::path& relativeTo, bool allowHome)
{
	// nothing here
	if(path.begin() == path.end())
		return path;

	if(*path.begin() == ".")
	{
		fs::path ret = relativeTo;
		for(auto it = ++path.begin(); it != path.end(); ++it)
			ret /= *it;
		return ret;
	}

	if(allowHome && *path.begin() == "~")
	{
		fs::path ret = getHomePath();
		for(auto it = ++path.begin(); it != path.end(); ++it)
			ret /= *it;
		return ret;
	}

	return path;
}

fs::path removeCommonPathUsingStrings(const fs::path& path, const fs::path& relativeTo, bool& contains)
{
#ifdef WIN32
	std::wstring pathStr = path.c_str();
	std::wstring relativeToStr = relativeTo.c_str();
#else
	std::string pathStr = path.c_str();
	std::string relativeToStr = relativeTo.c_str();
#endif
	if (pathStr.find_first_of(relativeToStr) == 0) {
		contains = true;
		return pathStr.substr(relativeToStr.size() + 1);
	}
	else {
		contains = false;
		return path;
	}
}

// example: removeCommonPath("/home/pi/roms/nes/foo/bar.nes", "/home/pi/roms/nes/") returns "foo/bar.nes"
fs::path removeCommonPath(const fs::path& path, const fs::path& relativeTo, bool& contains)
{
	// if either of these doesn't exist, fs::canonical() is going to throw an error
	if(!fs::exists(path) || !fs::exists(relativeTo))
	{
		contains = false;
		return path;
	}

	// if it's a symlink we don't want to apply fs::canonical on it, otherwise we'll lose the current parent_path
	fs::path p = (fs::is_symlink(path) ? fs::canonical(path.parent_path()) / path.filename() : fs::canonical(path));
	fs::path r = fs::canonical(relativeTo);

	if(p.root_path() != r.root_path())
	{
		contains = false;
		return p;
	}

	fs::path result;

	// find point of divergence
	auto itr_path = p.begin();
	auto itr_relative_to = r.begin();
	while(*itr_path == *itr_relative_to && itr_path != p.end() && itr_relative_to != r.end())
	{
		++itr_path;
		++itr_relative_to;
	}

	if(itr_relative_to != r.end())
	{
		contains = false;
		return p;
	}

	while(itr_path != p.end())
	{
		if(*itr_path != fs::path("."))
			result = result / *itr_path;

		++itr_path;
	}

	contains = true;
	return result;
}

// usage: makeRelativePath("/path/to/my/thing.sfc", "/path/to") -> "./my/thing.sfc"
// usage: makeRelativePath("/home/pi/my/thing.sfc", "/path/to", true) -> "~/my/thing.sfc"
fs::path makeRelativePath(const fs::path& path, const fs::path& relativeTo, bool allowHome)
{
	bool contains = false;

	fs::path ret = removeCommonPath(path, relativeTo, contains);
	if(contains)
	{
		// success
		ret = "." / ret;
		return ret;
	}

	if(allowHome)
	{
		contains = false;
		std::string homePath = getHomePath();
		ret = removeCommonPath(path, homePath, contains);
		if(contains)
		{
			// success
			ret = "~" / ret;
			return ret;
		}
	}

	// nothing could be resolved
	return path;
}

boost::posix_time::ptime string_to_ptime(const std::string& str, const std::string& fmt)
{
	std::istringstream ss(str);
	ss.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(fmt))); //std::locale handles deleting the facet
	boost::posix_time::ptime time;
	ss >> time;

	return time;
}

std::string strreplace(std::string str, const std::string& replace, const std::string& with)
{
	size_t pos;
	while((pos = str.find(replace)) != std::string::npos)
		str = str.replace(pos, replace.length(), with.c_str(), with.length());

	return str;
}

// plaform-specific escape path function
// on windows: just puts the path in quotes
// everything else: assume bash and escape special characters with backslashes
std::string escapePath(const boost::filesystem::path& path)
{
#ifdef WIN32
	// windows escapes stuff by just putting everything in quotes
	return '"' + fs::path(path).make_preferred().string() + '"';
#else
	// a quick and dirty way to insert a backslash before most characters that would mess up a bash path
	std::string pathStr = path.string();

	const char* invalidChars = " '\"\\!$^&*(){}[]?;<>";
	for(unsigned int i = 0; i < pathStr.length(); i++)
	{
		char c;
		unsigned int charNum = 0;
		do {
			c = invalidChars[charNum];
			if(pathStr[i] == c)
			{
				pathStr.insert(i, "\\");
				i++;
				break;
			}
			charNum++;
		} while(c != '\0');
	}

	return pathStr;
#endif
}

std::string removeParenthesis(const std::string& str)
{
	// remove anything in parenthesis or brackets
	// should be roughly equivalent to the regex replace "\((.*)\)|\[(.*)\]" with ""
	// I would love to just use regex, but it's not worth pulling in another boost lib for one function that is used once

	std::string ret = str;
	size_t start, end;

	static const int NUM_TO_REPLACE = 2;
	static const char toReplace[NUM_TO_REPLACE*2] = { '(', ')', '[', ']' };

	bool done = false;
	while(!done)
	{
		done = true;
		for(int i = 0; i < NUM_TO_REPLACE; i++)
		{
			end = ret.find_first_of(toReplace[i*2+1]);
			start = ret.find_last_of(toReplace[i*2], end);

			if(start != std::string::npos && end != std::string::npos)
			{
				ret.erase(start, end - start + 1);
				done = false;
			}
		}
	}

	// also strip whitespace
	end = ret.find_last_not_of(' ');
	if(end != std::string::npos)
		end++;

	ret = ret.substr(0, end);

	return ret;
}

std::vector<std::string> commaStringToVector(std::string commaString)
{
	// from a comma separated string, get a vector of strings
	std::vector<std::string> strs;
	boost::split(strs, commaString, boost::is_any_of(","));
	std::sort(strs.begin(), strs.end());
	return strs;
}

std::string vectorToCommaString(std::vector<std::string> stringVector)
{
	std::string out = "";
	std::sort(stringVector.begin(), stringVector.end());
	// from a vector of system names get comma separated string
	for(std::vector<std::string>::const_iterator it = stringVector.cbegin() ; it != stringVector.cend() ; it++ )
	{
		out = out + (out == "" ? "" : ",") + (*it);
	}
	return out;
}