#include "ArchiveManager.h"

#include <archive.h>
#include <archive_entry.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>


std::vector<std::string> ArchiveManager::list()
{
	std::vector<std::string> names;
	archive *a;
	archive_entry *entry;
	int r;

	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, mFilename.c_str(), 10240); // Note 1

	if (r != ARCHIVE_OK)
	{
		archive_read_close(a);
		archive_read_free(a);
		throw std::runtime_error("Can't open the archive");
	}

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

std::string ArchiveManager::extract()
{
	archive *a;
	archive *ext;
	archive_entry *entry;
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

	auto cleanup = [&] () {
		archive_read_close(a);
		archive_read_free(a);
		archive_write_close(ext);
		archive_write_free(ext);
	};

	if ((r = archive_read_open_filename(a, mFilename.c_str(), 10240)))
	{
		cleanup();
		throw std::runtime_error("Can't open the archive"); //TODO handle error
	}

	auto extractionFolder = tempPath.native() + "/" + boost::filesystem::unique_path().native();

	try
	{
		for (;;)
		{
			r = archive_read_next_header(a, &entry);

			if (r == ARCHIVE_EOF)
				break;

			if (r < ARCHIVE_OK)
				throw std::runtime_error(archive_error_string(a));

			const std::string tempstr = extractionFolder +
										"/" +
										archive_entry_pathname(entry);  // optional

			archive_entry_update_pathname_utf8(entry, tempstr.c_str());

			r = archive_write_header(ext, entry);

			if (r < ARCHIVE_OK)
				throw std::runtime_error(archive_error_string(ext));
			else if (archive_entry_size(entry) > 0)
			{
				r = copyData(a, ext);
				
				if (r < ARCHIVE_OK)
					throw std::runtime_error(archive_error_string(ext));
				
			}

			r = archive_write_finish_entry(ext);
			
			if (r < ARCHIVE_OK)
				throw std::runtime_error(archive_error_string(ext));
		}
	}
	catch(std::runtime_error& e)
	{
		cleanup();
		throw;
	}

	cleanup();

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
			throw std::runtime_error(archive_error_string(aw));
			return r;
		}
	}
}
