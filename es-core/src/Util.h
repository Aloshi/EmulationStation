#pragma once

#include <string>
#include <Eigen/Dense>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

// the original gamelist.xml used this format - it's something ISO, I forget what
#define LEGACY_TIME_STRING_FORMAT "%Y%m%dT%H%M%S%F%q"
// the SQLite database uses this format, as do MD_TIME/MD_DATE entries in the MetaDataMap
#define SQLITE_TIME_STRING_FORMAT "%Y-%m-%d %H:%M:%S.%F"

std::string strToUpper(const char* from);
std::string& strToUpper(std::string& str);
std::string strToUpper(const std::string& str);

Eigen::Affine3f& roundMatrix(Eigen::Affine3f& mat);
Eigen::Affine3f roundMatrix(const Eigen::Affine3f& mat);

Eigen::Vector3f roundVector(const Eigen::Vector3f& vec);
Eigen::Vector2f roundVector(const Eigen::Vector2f& vec);

#if defined(_WIN32) && _MSC_VER < 1800
float round(float num);
#endif

std::string getCanonicalPath(const std::string& str);

// example: removeCommonPath("/home/pi/roms/nes/foo/bar.nes", "/home/pi/roms/nes/") returns "foo/bar.nes"
boost::filesystem::path removeCommonPath(const boost::filesystem::path& path, const boost::filesystem::path& relativeTo, bool& contains);

// usage: makeRelativePath("/path/to/my/thing.sfc", "/path/to") -> "./my/thing.sfc"
// usage: makeRelativePath("/home/pi/my/thing.sfc", "/path/to", true) -> "~/my/thing.sfc"
boost::filesystem::path makeRelativePath(const boost::filesystem::path& path, const boost::filesystem::path& relativeTo, bool allowHome);

// expands "./my/path.sfc" to "[relativeTo]/my/path.sfc"
// if allowHome is true, also expands "~/my/path.sfc" to "/home/pi/my/path.sfc"
boost::filesystem::path resolvePath(const boost::filesystem::path& path, const boost::filesystem::path& relativeTo, bool allowHome);

boost::posix_time::ptime string_to_ptime(const std::string& str, const std::string& fmt);
std::string ptime_to_string(const boost::posix_time::ptime& str, const std::string& fmt);
