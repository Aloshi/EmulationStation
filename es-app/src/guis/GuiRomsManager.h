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
#include "PlatformId.h"
#include "components/OptionListComponent.h"

class FileData;
class RomsListView;

class GuiRomsManager : public MenuComponent
{
	friend class RomsListView;

private:
	struct PlatformData {
		PlatformIds::PlatformId id;
		std::string name;
		boost::filesystem::path romsPath;
		boost::filesystem::path externalRomsPath;
	};

public:
	GuiRomsManager(Window* window);
	~GuiRomsManager();

	bool input(InputConfig* config, Input input) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

	static std::string platformIdExternalRomsKey(PlatformIds::PlatformId platform);
	static boost::filesystem::path platformIdRomsPath(PlatformIds::PlatformId platform);
	static boost::filesystem::path platformIdExternalRomsPath(PlatformIds::PlatformId platform);

private:
	std::shared_ptr< FileData > m_fileData;
	std::shared_ptr<TextComponent> m_defaultRomsPath;
	std::shared_ptr<TextComponent> m_defaultExternalRomsPath;
	std::shared_ptr< OptionListComponent<PlatformIds::PlatformId> > m_platforms;
	std::shared_ptr<TextComponent> m_platformExternalRomsPath;
	bool m_settingsEdited;

	PlatformData currentPlatformData() const;
	void editDefaultRomsPath();
	void editDefaultExternalRomsPath();
	void editCurrentPlatformExternalRomsPath();
	void showCurrentPlatformRomsManager();
};
