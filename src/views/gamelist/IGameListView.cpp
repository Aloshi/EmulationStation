#include "IGameListView.h"
#include "../../Window.h"
#include "../../guis/GuiMetaDataEd.h"
#include "../../guis/GuiMenu.h"
#include "../../guis/GuiFastSelect.h"
#include "../ViewController.h"
#include "../../Settings.h"

bool IGameListView::input(InputConfig* config, Input input)
{
	// F3 to open metadata editor
	if(config->getDeviceId() == DEVICE_KEYBOARD && input.id == SDLK_F3 && input.value != 0)
	{
		// open metadata editor
		FileData* file = getCursor();
		ScraperSearchParams p;
		p.game = file;
		p.system = file->getSystem();
		mWindow->pushGui(new GuiMetaDataEd(mWindow, &file->metadata, file->metadata.getMDD(), p, file->getPath().filename().string(), 
			std::bind(&IGameListView::onFileChanged, this, file, FILE_METADATA_CHANGED), [file, this] { 
				boost::filesystem::remove(file->getPath()); //actually delete the file on the filesystem
				file->getParent()->removeChild(file); //unlink it so list repopulations triggered from onFileChanged won't see it
				onFileChanged(file, FILE_REMOVED); //tell the view
				delete file; //free it
		}));
		Sound::getFromTheme(mTheme, getName(), "menuOpen")->play();
		return true;

	// Ctrl-R to reload a view when debugging
	}else if(Settings::getInstance()->getBool("DEBUG") && config->getDeviceId() == DEVICE_KEYBOARD && 
		(SDL_GetModState() & (KMOD_LCTRL | KMOD_RCTRL)) && input.id == SDLK_r && input.value != 0)
	{
		LOG(LogDebug) << "reloading view";
		mWindow->getViewController()->reloadGameListView(this, true);
		return true;
	// select opens the fast select GUI
	}else if(config->isMappedTo("select", input) && input.value != 0)
	{
		// open fast select
		mWindow->pushGui(new GuiFastSelect(mWindow, this));
		return true;
	}

	return GuiComponent::input(config, input);
}

void IGameListView::setTheme(const std::shared_ptr<ThemeData>& theme)
{
	mTheme = theme;
	onThemeChanged(theme);
}
