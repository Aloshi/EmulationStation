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
#include "FileSystemSelectorComponent.h"
#include "Renderer.h"
#include "Log.h"

#define DEFAULT_FONT Font::get(FONT_SIZE_MEDIUM)
#define DEFAULT_COLOR 0x777777FF
#define DEFAULT_DISABLED_COLOR 0xBBBBBBFF

using namespace Eigen;
namespace fs = boost::filesystem;
typedef FileSystemSelectorComponent fssc;
typedef fssc::FileEntry fe;

struct ModePredicter : public std::unary_function<const fe*, bool>
{
	ModePredicter(fssc::Mode m) : mode(m) {
	}

	bool operator()(const fe* entry) {
		switch (mode) {
			case fssc::FilesShowFolders:
			case fssc::FoldersShowFiles:
				return (entry->type & fssc::FileInternal) || (entry->type & fssc::FolderInternal);

			case fssc::FilesHideFolders:
				return (entry->type & fssc::FileInternal);

			case fssc::FoldersHideFiles:
				return (entry->type & fssc::FolderInternal);
		}

		return false;
	}

	bool operator()(const fe& entry) {
		return operator()(&entry);
	}

private:
	fssc::Mode mode;
};

bool sortPredicter(const fe& lhs, const fe& rhs) {
	const std::string leftName = strToUpper(lhs.filePath.filename().string());
	const std::string rightName = strToUpper(rhs.filePath.filename().string());

	if (((lhs.type & fssc::FolderInternal) && (rhs.type & fssc::FolderInternal))
			|| ((lhs.type & fssc::FileInternal) && (rhs.type & fssc::FileInternal))) {
		return leftName.compare(rightName) < 0;
	}

	return lhs.type & fssc::FolderInternal;
}

FileSystemSelectorComponent::FileSystemSelectorComponent(Window* window, fssc::Mode mode)
	: MenuComponent(window, nullptr)
{
	init(mode, getHomePath());
}

FileSystemSelectorComponent::FileSystemSelectorComponent(Window* window, const fs::path& path, fssc::Mode mode)
	: MenuComponent(window, nullptr)
{
	init(mode, path);
}

void FileSystemSelectorComponent::init(const fssc::Mode mode, const fs::path& path)
{
	m_mode = mode;
	m_currentPathLabel = std::make_shared<TextComponent>(mWindow, std::string(), Font::get(FONT_SIZE_SMALL), 0xCC0000FF, ALIGN_CENTER);

	setTitle(titleForCurrentMode().c_str());
	if (!setCurrentPath(path)) {
		setCurrentPath(getHomePath());
	}
	addChild(m_currentPathLabel.get());
	onSizeChanged();
}

bool FileSystemSelectorComponent::input(InputConfig* config, Input input)
{
	const fs::path path = selectedFilePath();
	const fe::Type type = filePathType(path);

	if (config->isMappedTo("a", input) && input.value) { // cd in
		if (!path.empty()) {
			setCurrentPath(path);
		}

		return true;
	}
	else if (config->isMappedTo("b", input) && input.value) { // cd up
		switch (m_mode) {
			case fssc::FilesShowFolders:
			case fssc::FoldersShowFiles:
			case fssc::FoldersHideFiles:
				setCurrentPath(m_currentPath.parent_path());
				break;

			case fssc::FilesHideFolders:
				break;
		}

		return true;
	}
	else if (config->isMappedTo("x", input) && input.value) { // select
		if (m_acceptCallback) {
			switch (m_mode) {
				case fssc::FilesHideFolders:
				case fssc::FilesShowFolders: {
					if (type & fssc::FileInternal) {
						m_acceptCallback(path);
						delete this;
					}

					break;
				}

				case fssc::FoldersHideFiles:
				case fssc::FoldersShowFiles: {
					if (type & fssc::FolderInternal) {
						m_acceptCallback(path);
						delete this;
					}

					break;
				}
			}
		}

		return true;
	}
	else if (config->isMappedTo("y", input) && input.value) { // exit
		delete this;
		return true;
	}

	return MenuComponent::input(config, input);
}

std::vector<HelpPrompt> FileSystemSelectorComponent::getHelpPrompts()
{
	auto prompts = MenuComponent::getHelpPrompts();

	const fe::Type type = filePathType(selectedFilePath());

	if (m_mode != fssc::FilesHideFolders) {
		if (type & fssc::FolderInternal) {
			prompts.push_back(HelpPrompt("a", "cd in"));
		}

		if (m_currentPath != m_currentPath.root_path()) {
			prompts.push_back(HelpPrompt("b", "cd up"));
		}
	}

	prompts.push_back(HelpPrompt("y", "cancel"));

	switch (m_mode) {
		case fssc::FilesShowFolders: {
			if (type & fssc::FileInternal) {
				prompts.push_back(HelpPrompt("x", "select"));
			}

			break;
		}

		case fssc::FoldersShowFiles: {
			if (type & fssc::FolderInternal) {
				prompts.push_back(HelpPrompt("x", "select"));
			}

			break;
		}

		case fssc::FoldersHideFiles: {
			if (type & fssc::FolderInternal) {
				prompts.push_back(HelpPrompt("x", "select"));
			}

			break;
		}

		case fssc::FilesHideFolders: {
			if (type & fssc::FileInternal) {
				prompts.push_back(HelpPrompt("x", "select"));
			}

			break;
		}
	}

	return prompts;
}

void FileSystemSelectorComponent::onSizeChanged()
{
	MenuComponent::onSizeChanged();
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
	m_currentPathLabel->setSize(mSize.x(), 0);
	m_currentPathLabel->setPosition(0, mSize.y() - m_currentPathLabel->getSize().y());
}

fs::path FileSystemSelectorComponent::currentPath() const
{
	return m_currentPath;
}

bool FileSystemSelectorComponent::setCurrentPath(const fs::path& currentPath)
{
	fs::path path = currentPath;

	if (!fs::is_directory(path)) {
		path = path.parent_path();
	}

	if (!fs::exists(path) || path.empty() || m_currentPath == path) {
		return false;
	}

	m_currentPath = path;

	clear();
	populate(m_currentPath);

	const std::vector<const fe*> entries = entriesForCurrentMode();

	for (auto ientry = entries.cbegin(), end = entries.cend(); ientry != end; ++ientry) {
		const fe* entry = (*ientry);
		ComponentListRow row(entry->filePath.string());
		unsigned int color = DEFAULT_DISABLED_COLOR;
		
		// A folder
		if (entry->type & fssc::FolderInternal) {
			// We want a folder
			if (m_mode & fssc::FolderInternal) {
				color = DEFAULT_COLOR;
			}
		}
		// A file
		else if (entry->type & fssc::FileInternal) {
			// We want a file
			if (m_mode & fssc::FileInternal) {
				color = DEFAULT_COLOR;
			}
		}
		
		row.addElement(std::make_shared<TextComponent>(mWindow, entry->filePath.filename().string(), DEFAULT_FONT, color), true);
		addRow(row, false, std::next(ientry) == entries.cend());
	}

	m_currentPathLabel->setText(m_currentPath.string());
	setSize(Renderer::getScreenWidth() * 0.95f, Renderer::getScreenHeight() * 0.747f);
	updateHelpPrompts();
	return true;
}

std::vector<std::string> FileSystemSelectorComponent::filters() const
{
	return m_filters;
}

void FileSystemSelectorComponent::setFilters(const std::vector<std::string>& filters)
{
	m_filters = filters;
}

fs::path FileSystemSelectorComponent::selectedFilePath() const
{
	ComponentList *list = getList();
	return list->isEmpty() ? fs::path() : list->getSelectedName();
}

void FileSystemSelectorComponent::setSelectedFilePath(const fs::path& filePath)
{
	getList()->setSelectedName(filePath.string());
}

void FileSystemSelectorComponent::setAcceptCallback(const std::function<void (const fs::path&)>& callback)
{
	m_acceptCallback = callback;
}

void FileSystemSelectorComponent::setCancelCallback(const std::function<void ()>& callback)
{
	m_cancelCallback = callback;
}

std::string FileSystemSelectorComponent::titleForCurrentMode() const
{
	if (m_mode & fssc::FileInternal) {
		return std::string("Select a file");
	}
	else if (m_mode & fssc::FolderInternal) {
		return std::string("Select a folder");
	}

	return std::string("Select something");
}

fe::Type FileSystemSelectorComponent::filePathType(const fs::path& filePath) const
{
	if (!filePath.empty()) {
		switch (fs::status(filePath).type()) {
			case fs::regular_file:
				return fe::File;

			case fs::directory_file:
				return fe::Folder;

			case fs::symlink_file:
				return fs::is_directory(filePath) ? fe::FolderSymLink : fe::FileSymLink;
		}
	}

	return fe::InvalidType;
}

void FileSystemSelectorComponent::populate(const fs::path& path)
{
	if (m_entries.find(path) != m_entries.cend()) {
		return;
	}

	if (!fs::is_directory(path)) {
		LOG(LogWarning) << "Error - folder with path \"" << path.string() << "\" is not a directory!";
		return;
	}

	//make sure that this isn't a symlink to a thing we already have
	/*if (fs::is_symlink(path)) {
		//if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
		if(folderStr.find(fs::canonical(folderPath).generic_string()) == 0)
		{
			LOG(LogWarning) << "Skipping infinitely recursive symlink \"" << folderPath << "\"";
			return;
		}
	}*/

	try {
		for (fs::directory_iterator dir(path), end; dir != end; ++dir) {
			const fs::path filePath = (*dir).path();

			// Don't show entries without extensions
			/*if (!filePath.has_stem()) {
				continue;
			}*/

			// don't show hidden entries
			if (filePath.filename().string().compare(0, 1, ".") == 0) {
				continue;
			}

			if (!m_filters.empty()) {
				const std::string extension = filePath.extension().string();

				if (std::find(m_filters.begin(), m_filters.end(), extension) == m_filters.end()) {
					continue;
				}
			}

			FileEntry entry;
			entry.type = filePathType(filePath);
			entry.filePath = filePath;
			m_entries[path].push_back(entry);

			/*if (entry.type && fssc::FolderInternal) {
				populate(filePath);
			}*/
		}

		std::vector<fe>& entries = m_entries[path];
		std::stable_sort(entries.begin(), entries.end(), sortPredicter);
	}
	catch (...) {
	}
}

std::vector<const fe*> FileSystemSelectorComponent::entriesForCurrentMode() const
{
	auto it = m_entries.find(m_currentPath);

	if (it == m_entries.cend()) {
		return std::vector<const fe*>();
	}

	const std::vector<fe>& entries = (*it).second;
	std::vector<const fe*> matches;
	ModePredicter modePredicter(m_mode);

	matches.reserve(entries.size());

	for (auto pit = entries.cbegin(), end = entries.cend(); pit != end; ++pit) {
		const fe* entry = &(*pit);

		if (modePredicter(entry)) {
			matches.push_back(entry);
		}
	}

	return matches;
}
