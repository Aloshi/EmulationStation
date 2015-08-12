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
#include "GuiRomsManager.h"
#include "views/gamelist/BasicGameListView.h"
#include "SystemData.h"
#include "Settings.h"
#include "Util.h"
#include "Window.h"
#include "guis/GuiTextEditPopup.h"

#include "components/FileSystemSelectorComponent.h"

#include <boost/format.hpp>
#include <boost/system/error_code.hpp>

namespace fs = boost::filesystem;

#define SMALL_FONT_LIGHT Font::get(FONT_SIZE_SMALL, FONT_PATH_LIGHT)
#define DEFAULT_FONT_COLOR 0x777777FF
#define DEFAULT_ROMS_PATH_KEY "DefaultRomsPath"
#define DEFAULT_EXTERNAL_ROMS_PATH_KEY "DefaultExternalRomsPath"
#define PLATFORM_EXTERNAL_ROMS_PATH_KEY_FORMAT "ExternalRomsPath_%1%"
#define STRING_SETTING(KEY) Settings::getInstance()->getString(KEY)

namespace {
	bool fileDataLessThan(const FileData* lhs, const FileData* rhs) {
		const std::string leftName = strToUpper(lhs->getName());
		const std::string rightName = strToUpper(rhs->getName());

		if (lhs->getType() == rhs->getType()) {
			return leftName.compare(rightName) < 0;
		}

		return lhs->getType() == FOLDER;
	}

	bool createDirectories(const fs::path& target) {
		boost::system::error_code ec;
		fs::create_directories(target, ec);
		return ec.value() == boost::system::errc::success;
	}

	bool removeDirectories(const fs::path& target) {
		boost::system::error_code ec;
		fs::remove_all(target);
		return ec.value() == boost::system::errc::success;
	}

	bool createDirectorySymLink(const fs::path& source, const fs::path& target) {
		boost::system::error_code ec;
		fs::create_directory_symlink(source, target, ec);
		return ec.value() == boost::system::errc::success;
	}

	bool removeDirectorySymLink(const fs::path& source) {
		boost::system::error_code ec;
		fs::remove(source, ec);
		return ec.value() == boost::system::errc::success;
	}

	bool createFileSymLink(const fs::path& source, const fs::path& target) {
		boost::system::error_code ec;
		fs::create_symlink(source, target, ec);
		return ec.value() == boost::system::errc::success;
	}

	bool removeFileSymLink(const fs::path& source) {
		boost::system::error_code ec;
		fs::remove(source, ec);
		return ec.value() == boost::system::errc::success;
	}
}

class RomsListView : public BasicGameListView
{
private:
	GuiRomsManager::PlatformData m_data;

public:
	RomsListView(Window* window, FileData* fileData, const GuiRomsManager::PlatformData& data)
		: BasicGameListView(window, fileData)
		, m_data(data)
	{
		const fs::path themePath = ThemeData::getThemeFromCurrentSet(m_data.name);
		auto theme = std::make_shared<ThemeData>();

		try {
			theme->loadFile(themePath.string());
		}
		catch(...) {
		}

		setTheme(theme);

		mList.setAlignment(mList.ALIGN_LEFT);

		fileData->changePath(m_data.externalRomsPath);
		fileData->lazyPopulate();
		fileData->sort(fileDataLessThan, true);
		populateList(fileData->getChildren());
	}

	virtual std::vector<HelpPrompt> getHelpPrompts() override {
		std::vector<HelpPrompt> prompts;
		prompts.push_back(HelpPrompt("left/right", "link-switch"));
		prompts.push_back(HelpPrompt("up/down", "move"));

		if (fs::is_directory(getCursor()->getPath())) {
			prompts.push_back(HelpPrompt("a", "cd in"));
		}

		prompts.push_back(HelpPrompt("b", "cd up / back"));
		return prompts;
	}

	virtual bool input(InputConfig* config, Input input) override {
		if (config->isMappedTo("up", input) || config->isMappedTo("down", input) || config->isMappedTo("pageup", input) || config->isMappedTo("pagedown", input)) {
			return BasicGameListView::input(config, input);
		}
		else if (input.value != 0) {
			if (config->isMappedTo("a", input)) {
				FileData* cursor = getCursor();

				if (cursor->getType() == FOLDER) {
					if(cursor->getChildren().empty()) {
						cursor->lazyPopulate();
						cursor->sort(fileDataLessThan, true);
					}

					mCursorStack.push(cursor);
					populateList(cursor->getChildren());
				}
			}
			else if (config->isMappedTo("b", input)) {
				if (mCursorStack.size()) {
					populateList(mCursorStack.top()->getParent()->getChildren());
					setCursor(mCursorStack.top());
					mCursorStack.pop();
				}
				else {
					onFocusLost();
					delete this;
				}
			}
			else if (config->isMappedTo("right", input) || config->isMappedTo("left", input)) {
				// Create / Delete symlink
				FileData* cursor = getCursor();
				const int cursorId = mList.getCursor();
				const fs::path fileName = cursor->getPath().filename();
				const fs::path platformRomsPath = m_data.romsPath;
				const fs::path platformRomFilePath = fs::absolute(fileName, platformRomsPath);
				const bool isSymLink = fs::is_symlink(platformRomFilePath);

				// Don't handle existing non symlink files.
				if (fs::exists(platformRomFilePath) && !isSymLink) {
					return true;
				}

				if (cursor->getType() == GAME) {
					if (isSymLink) {
						if (removeFileSymLink(platformRomFilePath)) {
							updateCursor(cursorId, cursor);
						}
					}
					else {
						if (createFileSymLink(cursor->getPath(), platformRomFilePath)) {
							updateCursor(cursorId, cursor);
						}
					}
				}
				else if (cursor->getType() == FOLDER) {
					if (isSymLink) {
						if (removeDirectorySymLink(platformRomFilePath)) {
							updateCursor(cursorId, cursor);
						}
					}
					else {
						if (createDirectorySymLink(cursor->getPath(), platformRomFilePath)) {
							updateCursor(cursorId, cursor);
						}
					}
				}
			}
		}

		return true;
	}

	virtual void populateList(const std::vector<FileData*>& files) override {
		mList.clear();

		const FileData* root = getRoot();
		const SystemData* systemData = root->getSystem();
		const fs::path platformRomsPath = m_data.romsPath;

		mHeaderText.setText(systemData ? systemData->getFullName() : root->getCleanName());

		for (auto it = files.begin(); it != files.end(); it++) {
			const fs::path fileName = (*it)->getPath().filename();
			const fs::path platformRomFilePath = fs::absolute(fileName, platformRomsPath);
			const bool isSymLink = fs::is_symlink(platformRomFilePath);
			mList.add(formatName((*it)->getName(), isSymLink), *it, ((*it)->getType() == FOLDER));
		}
	}

protected:
	virtual void launch(FileData*) override {
	}

	std::string formatName(const std::string& name, bool isLinked) const {
		return str(boost::format("%1% %2%") %(isLinked ? "\u00A4" : "  ") %name);
	}

	void updateCursor(const int cursorId, FileData* fileData) {
		const fs::path fileName = fileData->getPath().filename();
		const fs::path platformRomsPath = m_data.romsPath;
		const fs::path platformRomFilePath = fs::absolute(fileName, platformRomsPath);
		const bool isSymLink = fs::is_symlink(platformRomFilePath);
		mList.changeCursorName(cursorId, formatName(fileData->getName(), isSymLink));
	}
};

GuiRomsManager::GuiRomsManager(Window *window)
	: MenuComponent(window, "ROMS MANAGER")
	, m_fileData(std::make_shared< FileData >(FOLDER, fs::path(), nullptr))
	, m_defaultRomsPath(std::make_shared<TextComponent>(window, std::string(), SMALL_FONT_LIGHT, DEFAULT_FONT_COLOR))
	, m_defaultExternalRomsPath(std::make_shared<TextComponent>(window, std::string(), SMALL_FONT_LIGHT, DEFAULT_FONT_COLOR))
	, m_platforms(std::make_shared< OptionListComponent<PlatformIds::PlatformId> >(window, "PLATFORM", false))
	, m_platformExternalRomsPath(std::make_shared<TextComponent>(window, std::string(), SMALL_FONT_LIGHT, DEFAULT_FONT_COLOR))
	, m_settingsEdited(false)
{
	m_defaultRomsPath->setText(STRING_SETTING(DEFAULT_ROMS_PATH_KEY));
	m_defaultExternalRomsPath->setText(STRING_SETTING(DEFAULT_EXTERNAL_ROMS_PATH_KEY));

	for (int i = PlatformIds::PLATFORM_UNKNOWN +1; i < PlatformIds::PLATFORM_COUNT; ++i) {
		const PlatformIds::PlatformId id = PlatformIds::PlatformId(i);

		if (id == PlatformIds::PLATFORM_IGNORE) {
			continue;
		}

		const std::string platformName = PlatformIds::getPlatformName(id);
		m_platforms->add(platformName, id, id == PlatformIds::THREEDO);
	}

	m_platformExternalRomsPath->setText(currentPlatformData().externalRomsPath.string());

	addWithLabel("ROMS", m_defaultRomsPath, true, true, [this](){ editDefaultRomsPath(); });
	addWithLabel("EXTERNAL ROMS", m_defaultExternalRomsPath, false, true, [this](){ editDefaultExternalRomsPath(); });
	addWithLabel("PLATFORM", m_platforms);
	addWithLabel("PLATFORM ROMS", m_platformExternalRomsPath, false, true, [this](){ editCurrentPlatformExternalRomsPath(); });

	m_platforms->setSelectedChangedCallback([this](const PlatformIds::PlatformId&){
		m_platformExternalRomsPath->setText(currentPlatformData().externalRomsPath.string());
		onSizeChanged();
	});

	setSize(Renderer::getScreenWidth() * 0.95f, Renderer::getScreenHeight() * 0.747f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

GuiRomsManager::~GuiRomsManager()
{
	if (m_settingsEdited) {
		Settings::getInstance()->saveFile();
	}
}

bool GuiRomsManager::input(InputConfig* config, Input input)
{
	if (config->isMappedTo("x", input) && input.value != 0) {
		showCurrentPlatformRomsManager();
		return true;
	}
	else if (config->isMappedTo("b", input) && input.value != 0) {
		delete this;
		return true;
	}

	return MenuComponent::input(config, input);
}

std::vector<HelpPrompt> GuiRomsManager::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = MenuComponent::getHelpPrompts();
	prompts.push_back(HelpPrompt("x", "manage platform roms"));
	prompts.push_back(HelpPrompt("a", "edit"));
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}

std::string GuiRomsManager::platformIdExternalRomsKey(PlatformIds::PlatformId platform)
{
	const std::string name = PlatformIds::getPlatformName(platform);
	return str(boost::format(PLATFORM_EXTERNAL_ROMS_PATH_KEY_FORMAT) % name);
}

fs::path GuiRomsManager::platformIdRomsPath(PlatformIds::PlatformId platform)
{
	const std::string path = getExpandedPath(STRING_SETTING(DEFAULT_ROMS_PATH_KEY));
	const std::string name = PlatformIds::getPlatformName(platform);

	if (!path.empty()) {
		return fs::absolute(name, path);
	}

	return fs::path();
}

fs::path GuiRomsManager::platformIdExternalRomsPath(PlatformIds::PlatformId platform)
{
	if (platform == PlatformIds::PLATFORM_UNKNOWN) {
		return fs::path();
	}

	const std::string key = platformIdExternalRomsKey(platform);
	return getExpandedPath(STRING_SETTING(key));
}

GuiRomsManager::PlatformData GuiRomsManager::currentPlatformData() const
{
	PlatformData data;
	data.id = m_platforms->getSelected();
	data.name = PlatformIds::getPlatformName(data.id);
	data.romsPath = platformIdRomsPath(data.id);
	data.externalRomsPath = platformIdExternalRomsPath(data.id);
	return data;
}

void GuiRomsManager::editDefaultRomsPath()
{
	const fs::path path = getExpandedPath(STRING_SETTING(DEFAULT_ROMS_PATH_KEY));
	auto updateValue = [this](const fs::path& filePath) {
		Settings::getInstance()->setString(DEFAULT_ROMS_PATH_KEY, filePath.string());
		m_defaultRomsPath->setText(filePath.string());
		m_settingsEdited = true;
		onSizeChanged();
	};
	auto fileChooser = new FileSystemSelectorComponent(mWindow, path, FileSystemSelectorComponent::FoldersHideFiles);
	fileChooser->setAcceptCallback(updateValue);

	mWindow->pushGui(fileChooser);
}

void GuiRomsManager::editDefaultExternalRomsPath()
{
	const fs::path path = getExpandedPath(STRING_SETTING(DEFAULT_EXTERNAL_ROMS_PATH_KEY));
	auto updateValue = [this](const fs::path& filePath) {
		Settings::getInstance()->setString(DEFAULT_EXTERNAL_ROMS_PATH_KEY, filePath.string());
		m_defaultExternalRomsPath->setText(filePath.string());
		m_settingsEdited = true;
		onSizeChanged();
	};
	auto fileChooser = new FileSystemSelectorComponent(mWindow, path, FileSystemSelectorComponent::FoldersHideFiles);
	fileChooser->setAcceptCallback(updateValue);

	mWindow->pushGui(fileChooser);
}

void GuiRomsManager::editCurrentPlatformExternalRomsPath()
{
	const GuiRomsManager::PlatformData data = currentPlatformData();
	const std::string key = platformIdExternalRomsKey(data.id);
	const fs::path defaultPath = getExpandedPath(STRING_SETTING(DEFAULT_EXTERNAL_ROMS_PATH_KEY));
	const fs::path path = data.externalRomsPath.empty() ? defaultPath : data.externalRomsPath;
	auto updateValue = [this, key](const fs::path& filePath) {
		Settings::getInstance()->setString(key, filePath.string());
		m_platformExternalRomsPath->setText(filePath.string());
		m_settingsEdited = true;
		onSizeChanged();
	};
	auto fileChooser = new FileSystemSelectorComponent(mWindow, path, FileSystemSelectorComponent::FoldersHideFiles);
	fileChooser->setAcceptCallback(updateValue);

	mWindow->pushGui(fileChooser);
}

void GuiRomsManager::showCurrentPlatformRomsManager()
{
	const GuiRomsManager::PlatformData data = currentPlatformData();

	if (data.romsPath.empty() || !fs::is_directory(data.externalRomsPath)) {
		return;
	}

	if (!fs::exists(data.romsPath)) {
		if (!createDirectories(data.romsPath)) {
			return;
		}
	}

	mWindow->pushGui(new RomsListView(mWindow, m_fileData.get(), data));
}
