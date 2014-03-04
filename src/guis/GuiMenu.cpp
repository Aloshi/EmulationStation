#include "GuiMenu.h"
#include "GuiSettingsMenu.h"
#include "GuiScraperStart.h"
#include "../Window.h"
#include "../Sound.h"
#include "../Log.h"
#include "GuiMsgBoxYesNo.h"
#include <initializer_list>
#include "../Settings.h"

GuiMenu::GuiMenu(Window* window) : GuiComponent(window), mMenu(window, "MAIN MENU")
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, (mSize.y() - mMenu.getSize().y()) / 2);

	// populate our list
	addEntry("GENERAL SETTINGS", 0x777777FF, true, 
		[this] { mWindow->pushGui(new GuiSettingsMenu(mWindow)); }
	);

	addEntry("SCRAPE NOW", 0x777777FF, true, 
		[this] { mWindow->pushGui(new GuiScraperStart(mWindow)); }
	);

	addEntry("RESTART SYSTEM", 0x990000FF, false, 
		[this] {
			mWindow->pushGui(new GuiMsgBoxYesNo(mWindow, "Do you really want to restart the system?",
			[] { 
				if(system("sudo shutdown -r now") != 0)
					LOG(LogWarning) << "Restart terminated with non-zero result!";
			})
		);}
	);

	addEntry("SHUTDOWN SYSTEM", 0x990000FF, false, 
		[this] {
			mWindow->pushGui(new GuiMsgBoxYesNo(mWindow, "Do you really want to shutdown the system?",
				[] { 
					if(system("sudo shutdown -h now") != 0)
						LOG(LogWarning) << "Shutdown terminated with non-zero result!";
				}));
		}
	);

	if(!Settings::getInstance()->getBool("DONTSHOWEXIT"))
	{
		addEntry("EXIT EMULATIONSTATION", 0x990000FF, false,
			[] {
				SDL_Event ev;
				ev.type = SDL_QUIT;
				SDL_PushEvent(&ev);
			}
		);
	}
	
	addChild(&mMenu);
}

void GuiMenu::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	
	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = std::make_shared<ImageComponent>(mWindow);
		bracket->setImage(":/sq_bracket.png");
		if(bracket->getTextureSize().y() > font->getHeight())
			bracket->setResize(0, (float)font->getHeight());

		row.addElement(bracket, false);
	}
	
	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if((config->isMappedTo("b", input) || config->isMappedTo("menu", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return GuiComponent::input(config, input);
}
