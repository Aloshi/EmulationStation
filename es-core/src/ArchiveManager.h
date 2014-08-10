#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include <iostream>

struct archive;
class ArchiveManager
{
	public:
		ArchiveManager(std::string filename): mFilename(filename) { }

		std::string extract();
		std::vector<std::string> list();

		static bool isAnArchive(const std::string& s)
		{
			std::string ext(s.substr(s.find_last_of('.') + 1));

			return  ext == "7z"  ||
					ext == "zip" ||
					ext == "rar";
		}

	private:
		int copyData(archive *ar, archive *aw);
		void checkError(int retcode, archive* arch);

		boost::filesystem::path tempPath{boost::filesystem::temp_directory_path()};
		std::string mFilename;
};

