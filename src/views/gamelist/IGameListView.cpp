#include "IGameListView.h"
#include "../../Window.h"
#include "../../guis/GuiMetaDataEd.h"
#include "../../guis/GuiMenu.h"
#include "../../guis/GuiGamelistOptions.h"
#include "../ViewController.h"
#include "../../Settings.h"
#include "../../Log.h"
#include "../../Sound.h"

bool IGameListView::input(InputConfig* config, Input input)
{
	// select to open GuiGamelistOptions
	if(config->isMappedTo("select", input) && input.value)
	{
		Sound::getFromTheme(mTheme, getName(), "menuOpen")->play();
		mWindow->pushGui(new GuiGamelistOptions(mWindow, this->mRoot->getSystem()));
		return true;

	// Ctrl-R to reload a view when debugging
	}else if(Settings::getInstance()->getBool("Debug") && config->getDeviceId() == DEVICE_KEYBOARD && 
		(SDL_GetModState() & (KMOD_LCTRL | KMOD_RCTRL)) && input.id == SDLK_r && input.value != 0)
	{
		LOG(LogDebug) << "reloading view";
		mWindow->getViewController()->reloadGameListView(this, true);
		return true;
	}

	return GuiComponent::input(config, input);
}

void IGameListView::setTheme(const std::shared_ptr<ThemeData>& theme)
{
	mTheme = theme;
	onThemeChanged(theme);
}
