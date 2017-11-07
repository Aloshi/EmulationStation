#include "views/gamelist/IGameListView.h"
#include "Window.h"
#include "guis/GuiMetaDataEd.h"
#include "guis/GuiMenu.h"
#include "guis/GuiGamelistOptions.h"
#include "views/ViewController.h"
#include "Settings.h"
#include "Log.h"
#include "Sound.h"

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
		ViewController::get()->reloadGameListView(this, true);
		return true;
	}

	return GuiComponent::input(config, input);
}

void IGameListView::setTheme(const std::shared_ptr<ThemeData>& theme)
{
	mTheme = theme;
	onThemeChanged(theme);
}

HelpStyle IGameListView::getHelpStyle()
{
	HelpStyle style;
	style.applyTheme(mTheme, getName());
	return style;
}

void IGameListView::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	float scaleX = trans.linear()(0,0);
	float scaleY = trans.linear()(1,1);

	Eigen::Vector2i pos(trans.translation()[0], trans.translation()[1]);
	Eigen::Vector2i size(mSize.x() * scaleX, mSize.y() * scaleY);

	Renderer::pushClipRect(pos, size);
	renderChildren(trans);
	Renderer::popClipRect();
}
