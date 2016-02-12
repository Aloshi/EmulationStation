/*
 * Copyright (c) 2015 Filipe Azevedo <pasnox@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
*/
#pragma once

#include "components/MenuComponent.h"

#include <boost/filesystem/path.hpp>

/*
 * Lazy loaded file system selector component
 * */
class FileSystemSelectorComponent : public MenuComponent
{
private:
	enum InternalType {
		InvalidInternal = 0x0,
		FileInternal = 0x1,
		FolderInternal = 0x2,
		SymLinkInternal = 0x4,
		ShowInternal = 0x8,
		HideInternal = 0x10
	};

public:
	enum Mode {
		FilesShowFolders = FileInternal | ShowInternal,
		FilesHideFolders = FileInternal | HideInternal,
		FoldersShowFiles = FolderInternal | ShowInternal,
		FoldersHideFiles = FolderInternal | HideInternal
	};

	struct FileEntry {
		enum Type {
			InvalidType = InvalidInternal,
			File = FileInternal,
			FileSymLink = FileInternal | SymLinkInternal,
			Folder = FolderInternal,
			FolderSymLink = FolderInternal | SymLinkInternal
		};

		Type type;
		boost::filesystem::path filePath;
	};

	FileSystemSelectorComponent(Window* window, const FileSystemSelectorComponent::Mode mode = FileSystemSelectorComponent::FilesShowFolders);
	FileSystemSelectorComponent(Window* window, const boost::filesystem::path& path, const FileSystemSelectorComponent::Mode mode = FileSystemSelectorComponent::FilesShowFolders);

	bool input(InputConfig* config, Input input) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	void onSizeChanged() override;

	boost::filesystem::path currentPath() const;
	bool setCurrentPath(const boost::filesystem::path& currentPath);

	std::vector<std::string> filters() const;
	void setFilters(const std::vector<std::string>& filters);

	boost::filesystem::path selectedFilePath() const;
	void setSelectedFilePath(const boost::filesystem::path& filePath);

	void setAcceptCallback(const std::function<void(const boost::filesystem::path&)>& callback);
	void setCancelCallback(const std::function<void()>& callback);

private:
	FileSystemSelectorComponent::Mode m_mode;
	std::vector<std::string> m_filters;
	boost::filesystem::path m_currentPath;
	std::map<boost::filesystem::path, std::vector<FileSystemSelectorComponent::FileEntry>> m_entries;
	std::shared_ptr<TextComponent> m_currentPathLabel;
	std::function<void(const boost::filesystem::path&)> m_acceptCallback;
	std::function<void()> m_cancelCallback;

	void init(const FileSystemSelectorComponent::Mode mode, const boost::filesystem::path& path);
	std::string titleForCurrentMode() const;
	FileSystemSelectorComponent::FileEntry::Type filePathType(const boost::filesystem::path& filePath) const;
	void populate(const boost::filesystem::path& path);
	std::vector<const FileSystemSelectorComponent::FileEntry*> entriesForCurrentMode() const;

	friend class ModePredicter;
	friend bool sortPredicter(const FileSystemSelectorComponent::FileEntry&, const FileSystemSelectorComponent::FileEntry&);
};

