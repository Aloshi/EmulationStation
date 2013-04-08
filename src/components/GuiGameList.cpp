#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"
#include "GuiFastSelect.h"
#include <boost/filesystem.hpp>
#include "../Log.h"

GuiGameList::GuiGameList(Window* window, bool useDetail) : Gui(window)
{
	mDetailed = useDetail;

	mTheme = new GuiTheme(mWindow, mDetailed);

	//The GuiGameList can use the older, simple game list if so desired.
	//The old view only shows a list in the center of the screen; the new view can display an image and description.
	//Those with smaller displays may prefer the older view.
	if(mDetailed)
	{
		mList = new GuiList<FileData*>(mWindow, Renderer::getScreenWidth() * mTheme->getFloat("listOffsetX"), Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, Renderer::getDefaultFont(Renderer::MEDIUM));

		mScreenshot = new GuiImage(mWindow, Renderer::getScreenWidth() * mTheme->getFloat("gameImageOffsetX"), Renderer::getScreenHeight() * mTheme->getFloat("gameImageOffsetY"), "", mTheme->getFloat("gameImageWidth"), mTheme->getFloat("gameImageHeight"), false);
		mScreenshot->setOrigin(mTheme->getFloat("gameImageOriginX"), mTheme->getFloat("gameImageOriginY"));

		mImageAnimation = new GuiAnimation();
		mImageAnimation->addChild(mScreenshot);
	}else{
		mList = new GuiList<FileData*>(mWindow, 0, Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, Renderer::getDefaultFont(Renderer::MEDIUM));
		mScreenshot = NULL;
		mImageAnimation = NULL;
	}

	setSystemId(0);
}

GuiGameList::~GuiGameList()
{
	delete mList;

	if(mDetailed)
	{
		delete mImageAnimation;
		delete mScreenshot;
		delete mTheme;
	}
}

void GuiGameList::setSystemId(int id)
{
	if(SystemData::sSystemVector.size() == 0)
	{
		LOG(LogError) << "Error - no systems found!";
		return;
	}

	//make sure the id is within range
	if(id >= (int)SystemData::sSystemVector.size())
		id -= SystemData::sSystemVector.size();
	if(id < 0)
		id += SystemData::sSystemVector.size();

	mSystemId = id;
	mSystem = SystemData::sSystemVector.at(mSystemId);

	//clear the folder stack
	while(mFolderStack.size()){ mFolderStack.pop(); }

	mFolder = mSystem->getRootFolder();

	updateTheme();
	updateList();
	updateDetailData();
}

void GuiGameList::render()
{
	if(mTheme)
		mTheme->render();

	//header
	if(!mTheme->getBool("hideHeader"))
		Renderer::drawCenteredText(mSystem->getDescName(), 0, 1, 0xFF0000FF, Renderer::getDefaultFont(Renderer::LARGE));

	if(mDetailed)
	{
		//divider
		if(!mTheme->getBool("hideDividers"))
			Renderer::drawRect(Renderer::getScreenWidth() * mTheme->getFloat("listOffsetX") - 4, Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, 8, Renderer::getScreenHeight(), 0x0000FFFF);

		//if we're not scrolling and we have selected a non-folder
		if(!mList->isScrolling() && mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
		{
			GameData* game = (GameData*)mList->getSelectedObject();

			std::string desc = game->getDescription();
			if(!desc.empty())
				Renderer::drawWrappedText(desc, Renderer::getScreenWidth() * 0.03, mScreenshot->getOffsetY() + mScreenshot->getHeight() + 12, Renderer::getScreenWidth() * (mTheme->getFloat("listOffsetX") - 0.03), mTheme->getColor("description"), mTheme->getDescriptionFont());
		}

		mScreenshot->render();
	}

	mList->render();
}

void GuiGameList::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && mFolder->getFileCount() > 0 && input.value != 0)
	{
		//play select sound
		mTheme->getSound("menuSelect")->play();

		FileData* file = mList->getSelectedObject();
		if(file->isFolder()) //if you selected a folder, add this directory to the stack, and use the selected one
		{
			mFolderStack.push(mFolder);
			mFolder = (FolderData*)file;
			updateList();
		}else{
			mList->stopScrolling();

			//wait for the sound to finish or we'll never hear it...
			while(mTheme->getSound("menuSelect")->isPlaying());

			mSystem->launchGame(mWindow, (GameData*)file);
		}
	}

	//if there's something on the directory stack, return to it
	if(config->isMappedTo("b", input) && input.value != 0 && mFolderStack.size())
	{
		mFolder = mFolderStack.top();
		mFolderStack.pop();
		updateList();
		updateDetailData();

		//play the back sound
		mTheme->getSound("menuBack")->play();
	}

	//only allow switching systems if more than one exists (otherwise it'll reset your position when you switch and it's annoying)
	if(SystemData::sSystemVector.size() > 1 && input.value != 0)
	{
		if(config->isMappedTo("right", input))
		{
			setSystemId(mSystemId + 1);
		}
		if(config->isMappedTo("left", input))
		{
			setSystemId(mSystemId - 1);
		}
	}

	//open the "start menu"
	if(config->isMappedTo("menu", input) && input.value != 0)
	{
		new GuiMenu(mWindow, this);
	}

	//open the fast select menu
	if(config->isMappedTo("select", input) && input.value != 0)
	{
		new GuiFastSelect(mWindow, this, mList, mList->getSelectedObject()->getName()[0], mTheme->getBoxData(), mTheme->getColor("fastSelect"), mTheme->getSound("menuScroll"), mTheme->getFastSelectFont());
	}

	if(mDetailed)
	{
		if(config->isMappedTo("up", input) || config->isMappedTo("down", input) || config->isMappedTo("pageup", input) || config->isMappedTo("pagedown", input))
		{
			if(input.value == 0)
				updateDetailData();
			else
				clearDetailData();
		}
	}
}

void GuiGameList::updateList()
{
	if(mDetailed)
		mScreenshot->setImage("");

	mList->clear();

	for(unsigned int i = 0; i < mFolder->getFileCount(); i++)
	{
		FileData* file = mFolder->getFile(i);

		if(file->isFolder())
			mList->addObject(file->getName(), file, mTheme->getColor("secondary"));
		else
			mList->addObject(file->getName(), file, mTheme->getColor("primary"));
	}
}

std::string GuiGameList::getThemeFile()
{
	std::string themePath;

	themePath = getenv("HOME");
	themePath += "/.emulationstation/" +  mSystem->getName() + "/theme.xml";
	if(boost::filesystem::exists(themePath))
		return themePath;

	themePath = mSystem->getStartPath() + "/theme.xml";
	if(boost::filesystem::exists(themePath))
		return themePath;

	themePath = getenv("HOME");
	themePath += "/.emulationstation/es_theme.xml";
	if(boost::filesystem::exists(themePath))
		return themePath;

	return "";
}

void GuiGameList::updateTheme()
{
	if(!mTheme)
		return;

	mTheme->readXML( getThemeFile() );

	mList->setSelectorColor(mTheme->getColor("selector"));
	mList->setSelectedTextColor(mTheme->getColor("selected"));
	mList->setScrollSound(mTheme->getSound("menuScroll"));

	//fonts
	mList->setFont(mTheme->getListFont());

	if(mDetailed)
	{
		mList->setCentered(mTheme->getBool("listCentered"));

		mList->setOffsetX(mTheme->getFloat("listOffsetX") * Renderer::getScreenWidth());
		mList->setTextOffsetX(mTheme->getFloat("listTextOffsetX") * Renderer::getScreenWidth());

		mScreenshot->setOffsetX(mTheme->getFloat("gameImageOffsetX") * Renderer::getScreenWidth());
		mScreenshot->setOffsetY(mTheme->getFloat("gameImageOffsetY") * Renderer::getScreenHeight());
		mScreenshot->setOrigin(mTheme->getFloat("gameImageOriginX"), mTheme->getFloat("gameImageOriginY"));
		mScreenshot->setResize(mTheme->getFloat("gameImageWidth"), mTheme->getFloat("gameImageHeight"), false);
	}
}

void GuiGameList::updateDetailData()
{
	if(!mDetailed)
		return;

	if(mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
	{
		mScreenshot->setOffset((mTheme->getFloat("gameImageOffsetX") - 0.05) * Renderer::getScreenWidth(), mTheme->getFloat("gameImageOffsetY") * Renderer::getScreenHeight());

		if(((GameData*)mList->getSelectedObject())->getImagePath().empty())
			mScreenshot->setImage(mTheme->getString("imageNotFoundPath"));
		else
			mScreenshot->setImage(((GameData*)mList->getSelectedObject())->getImagePath());

		mImageAnimation->fadeIn(15);
		mImageAnimation->move((int)(0.05 * Renderer::getScreenWidth()), 0, 5);
	}else{
		mScreenshot->setImage("");
	}
}

void GuiGameList::clearDetailData()
{
	if(mDetailed)
	{
		mImageAnimation->fadeOut(35);
	}
}

//called when the renderer shuts down/returns
//we have to manually call init/deinit on mTheme because it is not our child
void GuiGameList::onDeinit()
{
	mTheme->deinit();
}

void GuiGameList::onInit()
{
	mTheme->init();
}

extern bool IGNOREGAMELIST; //defined in main.cpp (as a command line argument)
GuiGameList* GuiGameList::create(Window* window)
{
	bool detailed = false;

	if(!IGNOREGAMELIST)
	{
		for(unsigned int i = 0; i < SystemData::sSystemVector.size(); i++)
		{
			if(SystemData::sSystemVector.at(i)->hasGamelist())
			{
				detailed = true;
				break;
			}
		}
	}

	GuiGameList* list = new GuiGameList(window, detailed);
	window->pushGui(list);
	return list;
}
