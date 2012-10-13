#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"
#include "GuiFastSelect.h"
#include <boost/filesystem.hpp>

//this is just a default value; the true value is in mTheme->getListOffsetX();
const float GuiGameList::sInfoWidth = 0.5;


GuiGameList::GuiGameList(bool useDetail)
{
	//std::cout << "Creating GuiGameList\n";
	mDetailed = useDetail;

	mTheme = new GuiTheme(); //not a child because it's rendered manually by GuiGameList::onRender (to make sure it's rendered first)

	//The GuiGameList can use the older, simple game list if so desired.
	//The old view only shows a list in the center of the screen; the new view can display an image and description.
	//Those with smaller displays may prefer the older view.
	if(mDetailed)
	{
		mList = new GuiList<FileData*>(Renderer::getScreenWidth() * sInfoWidth, Renderer::getFontHeight(Renderer::LARGE) + 2);

		mScreenshot = new GuiImage(Renderer::getScreenWidth() * sInfoWidth * 0.5, Renderer::getScreenHeight() * mTheme->getGameImageOffsetY(), "", Renderer::getScreenWidth() * sInfoWidth * 0.7, 0, false);
		mScreenshot->setOrigin(mTheme->getGameImageOriginX(), mTheme->getGameImageOriginY());
		addChild(mScreenshot);
	}else{
		mList = new GuiList<FileData*>(0, Renderer::getFontHeight(Renderer::LARGE) + 2);
		mScreenshot = NULL;
	}

	addChild(mList);

	setSystemId(0);

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);
}

GuiGameList::~GuiGameList()
{
	Renderer::unregisterComponent(this);
	delete mList;

	if(mDetailed)
	{
		delete mScreenshot;
		delete mTheme;
	}

	InputManager::unregisterComponent(this);
}

void GuiGameList::setSystemId(int id)
{
	if(SystemData::sSystemVector.size() == 0)
	{
		std::cerr << "Error - no systems found!\n";
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

void GuiGameList::onRender()
{
	if(mTheme)
		mTheme->render();

	//header
	if(!mTheme->getHeaderHidden())
		Renderer::drawCenteredText(mSystem->getName(), 0, 1, 0xFF0000, Renderer::LARGE);

	if(mDetailed)
	{
		//divider
		if(!mTheme->getDividersHidden())
			Renderer::drawRect(Renderer::getScreenWidth() * mTheme->getListOffsetX() - 4, Renderer::getFontHeight(Renderer::LARGE) + 2, 8, Renderer::getScreenHeight(), 0x0000FF);

		//if we're not scrolling and we have selected a non-folder
		if(!mList->isScrolling() && mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
		{
			GameData* game = (GameData*)mList->getSelectedObject();

			std::string desc = game->getDescription();
			if(!desc.empty())
				Renderer::drawWrappedText(desc, Renderer::getScreenWidth() * 0.03, mScreenshot->getOffsetY() + mScreenshot->getHeight() + 12, Renderer::getScreenWidth() * (mTheme->getListOffsetX() - 0.03), mTheme->getDescColor(), Renderer::SMALL);
		}
	}
}

void GuiGameList::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::BUTTON1 && mFolder->getFileCount() > 0)
	{
		if(!keyDown)
		{
			//play select sound
			mTheme->getMenuSelectSound()->play();

			FileData* file = mList->getSelectedObject();
			if(file->isFolder()) //if you selected a folder, add this directory to the stack, and use the selected one
			{
				mFolderStack.push(mFolder);
				mFolder = (FolderData*)file;
				updateList();
			}else{
				//wait for the sound to finish or we'll never hear it...
				while(mTheme->getMenuSelectSound()->isPlaying());

				mSystem->launchGame((GameData*)file);
			}
		}
	}

	//if there's something on the directory stack, return to it
	if(button == InputManager::BUTTON2 && keyDown && mFolderStack.size())
	{
		mFolder = mFolderStack.top();
		mFolderStack.pop();
		updateList();
		updateDetailData();

		//play the back sound
		mTheme->getMenuBackSound()->play();
	}

	//only allow switching systems if more than one exists (otherwise it'll reset your position when you switch and it's annoying)
	if(SystemData::sSystemVector.size() > 1)
	{
		if(button == InputManager::RIGHT && keyDown)
		{
			setSystemId(mSystemId + 1);
		}
		if(button == InputManager::LEFT && keyDown)
		{
			setSystemId(mSystemId - 1);
		}
	}

	//open the "start menu"
	if(button == InputManager::MENU && keyDown)
	{
		new GuiMenu(this);
	}

	//open the fast select menu
	if(button == InputManager::SELECT && keyDown)
	{
		new GuiFastSelect(this, mList, mList->getSelectedObject()->getName()[0], mTheme->getBoxData(), mTheme->getFastSelectColor(), mTheme->getMenuScrollSound());
	}

	if(mDetailed)
	{
		if(button == InputManager::UP || button == InputManager::DOWN)
		{
			if(!keyDown)
				updateDetailData();
			else
				mScreenshot->setImage(""); //clear the image when we start scrolling
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
			mList->addObject(file->getName(), file, mTheme->getSecondaryColor());
		else
			mList->addObject(file->getName(), file, mTheme->getPrimaryColor());
	}
}

void GuiGameList::updateTheme()
{
	if(!mTheme)
		return;

	std::string defaultPath = getenv("HOME");
	defaultPath += "/.emulationstation/es_theme.xml";
	std::string themePath = mSystem->getStartPath() + "/theme.xml";

	if(boost::filesystem::exists(themePath))
		mTheme->readXML(themePath);
	else if(boost::filesystem::exists(defaultPath))
		mTheme->readXML(defaultPath);
	else
		mTheme->readXML(""); //clears any current theme

	mList->setSelectorColor(mTheme->getSelectorColor());
	mList->setSelectedTextColor(mTheme->getSelectedTextColor());
	mList->setCentered(mTheme->getListCentered());
	mList->setScrollSound(mTheme->getMenuScrollSound());

	if(mDetailed)
	{
		mList->setOffsetX(mTheme->getListOffsetX() * Renderer::getScreenWidth());
		mList->setTextOffsetX(mTheme->getListTextOffsetX() * Renderer::getScreenWidth());

		mScreenshot->setOffsetX(mTheme->getGameImageOffsetX() * Renderer::getScreenWidth());
		mScreenshot->setOffsetY(mTheme->getGameImageOffsetY() * Renderer::getScreenHeight());
		mScreenshot->setOrigin(mTheme->getGameImageOriginX(), mTheme->getGameImageOriginY());
	}
}

void GuiGameList::updateDetailData()
{
	if(!mDetailed)
		return;

	if(mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
	{
		mScreenshot->setImage(((GameData*)mList->getSelectedObject())->getImagePath());
	}else{
		mScreenshot->setImage("");
	}
}

//these are called when the menu opens/closes
void GuiGameList::onPause()
{
	mTheme->getMenuOpenSound()->play();
	InputManager::unregisterComponent(this);
}

void GuiGameList::onResume()
{
	updateDetailData();
	InputManager::registerComponent(this);
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
GuiGameList* GuiGameList::create()
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

	return new GuiGameList(detailed);
}
