#include "ArchiveManager.h"

#include <archive.h>
#include <archive_entry.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>


std::vector<std::string> ArchiveManager::list()
{
	std::vector<std::string> names;
	struct archive *a;
	struct archive_entry *entry;
	int r;

	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, mFilename.c_str(), 10240); // Note 1

	if (r != ARCHIVE_OK)
		throw std::runtime_error("Can't open the archive");

	while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
	{
		names.push_back(archive_entry_pathname(entry));
		archive_read_data_skip(a);
	}

	r = archive_read_free(a);

	if (r != ARCHIVE_OK)
		throw std::runtime_error("Error in the archive");

	return names;
}

void ArchiveManager::checkError(int retcode, archive* arch)
{
	if (retcode < ARCHIVE_OK)
		std::cerr << "Archive error: " << archive_error_string(arch) << std::endl;
	if (retcode < ARCHIVE_WARN)
		throw std::runtime_error("Error in the archive");
}

std::string ArchiveManager::extract()
{
	struct archive *a;
	struct archive *ext;
	struct archive_entry *entry;
	int flags;
	int r;

	/* Select which attributes we want to restore. */
	flags = ARCHIVE_EXTRACT_TIME;
	flags |= ARCHIVE_EXTRACT_PERM;
	flags |= ARCHIVE_EXTRACT_ACL;
	flags |= ARCHIVE_EXTRACT_FFLAGS;

	a = archive_read_new();
	archive_read_support_format_all(a);
	archive_read_support_filter_all(a);

	ext = archive_write_disk_new();
	archive_write_disk_set_options(ext, flags);
	archive_write_disk_set_standard_lookup(ext);

	if ((r = archive_read_open_filename(a, mFilename.c_str(), 10240)))
		throw std::runtime_error("Can't open the archive"); //TODO handle error

	auto extractionFolder = tempPath.native() + "/" + boost::filesystem::unique_path().native();

	for (;;)
	{
		r = archive_read_next_header(a, &entry);

		if (r == ARCHIVE_EOF)
			break;

		checkError(r, a);

		const std::string tempstr = extractionFolder +
									"/" +
									archive_entry_pathname(entry);  // optional

		archive_entry_update_pathname_utf8(entry, tempstr.c_str());
//		printf("Extracting : %s\n", archive_entry_pathname(entry));

		r = archive_write_header(ext, entry);

		if (r < ARCHIVE_OK)
		{
			std::cerr << "Archive error: " << archive_error_string(ext) << std::endl;
		}
		else if (archive_entry_size(entry) > 0)
		{
			r = copyData(a, ext);
			checkError(r, ext);
		}

		r = archive_write_finish_entry(ext);
		checkError(r, ext);
	}

	archive_read_close(a);
	archive_read_free(a);

	archive_write_close(ext);
	archive_write_free(ext);

	return extractionFolder;
}


int ArchiveManager::copyData(archive* ar, archive* aw)
{
	int r;
	const void *buff;
	size_t size;
	off_t offset;

	for (;;)
	{
		r = archive_read_data_block(ar, &buff, &size, &offset);

		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);

		if (r < ARCHIVE_OK)
			return r;

		r = archive_write_data_block(aw, buff, size, offset);

		if (r < ARCHIVE_OK)
		{
			std::cerr << "Archive error: " << archive_error_string(aw) << std::endl;
			return r;
		}
	}
}
