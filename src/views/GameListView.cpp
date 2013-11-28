#include "GameListView.h"
#include "../Window.h"
#include "../components/GuiMetaDataEd.h"
#include "../components/GuiMenu.h"
#include "../components/GuiFastSelect.h"
#include "ViewController.h"

bool GameListView::input(InputConfig* config, Input input)
{
	if(config->getDeviceId() == DEVICE_KEYBOARD && input.id == SDLK_F3 && input.value != 0)
	{
		// open metadata editor
		FileData* file = getCursor();
		ScraperSearchParams p;
		p.game = file;
		p.system = file->getSystem();
		mWindow->pushGui(new GuiMetaDataEd(mWindow, &file->metadata, file->metadata.getMDD(), p, file->getPath().filename().string(), 
			std::bind(&GameListView::onFileChanged, this, file, FILE_METADATA_CHANGED), [file, this] { 
				boost::filesystem::remove(file->getPath()); //actually delete the file on the filesystem
				file->getParent()->removeChild(file); //unlink it so list repopulations triggered from onFileChanged won't see it
				onFileChanged(file, FILE_REMOVED); //tell the view
				delete file; //free it
		}));
		mTheme->playSound("menuOpenSound");
		return true;
	}else if(config->isMappedTo("menu", input) && input.value != 0)
	{
		// open menu
		mWindow->pushGui(new GuiMenu(mWindow));
		mTheme->playSound("menuOpenSound");
		return true;
	}else if(config->isMappedTo("select", input) && input.value != 0)
	{
		// open fast select
		mWindow->pushGui(new GuiFastSelect(mWindow, this));
		return true;
	}

	return GuiComponent::input(config, input);
}

void GameListView::setTheme(const std::shared_ptr<ThemeData>& theme)
{
	mTheme = theme;
	onThemeChanged(theme);
}
