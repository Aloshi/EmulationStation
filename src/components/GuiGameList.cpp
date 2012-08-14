#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"
#include <boost/filesystem.hpp>

//#define DRAWFRAMERATE

#ifdef DRAWFRAMERATE
	#include <sstream>
	extern float FRAMERATE;
#endif

const float GuiGameList::sInfoWidth = 0.5;

GuiGameList::GuiGameList(bool useDetail)
{
	std::cout << "Creating GuiGameList\n";
	mDetailed = useDetail;

	//The GuiGameList can use the older, simple game list if so desired.
	//The old view only shows a list in the center of the screen; the new view can display an image and description.
	//Those with smaller displays may prefer the older view.
	if(mDetailed)
	{
		mList = new GuiList<FileData*>(Renderer::getScreenWidth() * sInfoWidth, Renderer::getFontHeight(Renderer::LARGE) + 2);

		mScreenshot = new GuiImage(Renderer::getScreenWidth() * 0.2, Renderer::getFontHeight(Renderer::LARGE) + 2, "", Renderer::getScreenWidth() * 0.3);
		mScreenshot->setOrigin(0.5, 0.0);
		mScreenshot->setAlpha(true); //slower, but requested
		addChild(mScreenshot);
	}else{
		mList = new GuiList<FileData*>(0, Renderer::getFontHeight(Renderer::LARGE) + 2);
		mScreenshot = NULL;
	}

	mTheme = new GuiTheme(); //not a child because it's rendered manually by GuiGameList::onRender (to make sure it's rendered first)

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

	//clear the folder stack (I can't look up the proper method right now)
	while(mFolderStack.size()){ mFolderStack.pop(); }

	mFolder = mSystem->getRootFolder();

	updateTheme();
	updateList();
	updateDetailData();
}

void GuiGameList::onRender()
{
	Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0xFFFFFF);

	if(mTheme)
		mTheme->render();

	#ifdef DRAWFRAMERATE
		std::stringstream ss;
		ss << FRAMERATE;
		std::string fps;
		ss >> fps;
		Renderer::drawText(fps, 0, 0, 0x00FF00);
	#endif

	//header
	if(!mTheme->getHeaderHidden())
		Renderer::drawCenteredText(mSystem->getName(), 0, 1, 0xFF0000, Renderer::LARGE);

	if(mDetailed)
	{
		//divider
		if(!mTheme->getDividersHidden())
			Renderer::drawRect(Renderer::getScreenWidth() * sInfoWidth - 4, Renderer::getFontHeight(Renderer::LARGE) + 2, 8, Renderer::getScreenHeight(), 0x0000FF);

		//if we have selected a non-folder
		if(mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
		{
			GameData* game = (GameData*)mList->getSelectedObject();

			//todo: cache this
			std::string desc = game->getDescription();
			if(!desc.empty())
				Renderer::drawWrappedText(desc, Renderer::getScreenWidth() * 0.03, Renderer::getFontHeight(Renderer::LARGE) + mScreenshot->getHeight() + 10, Renderer::getScreenWidth() * (sInfoWidth - 0.03), mTheme->getDescColor(), Renderer::SMALL);
		}
	}
}

void GuiGameList::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::BUTTON1 && mFolder->getFileCount() > 0)
	{
		if(!keyDown)
		{
			FileData* file = mList->getSelectedObject();
			if(file->isFolder()) //if you selected a folder, add this directory to the stack, and use the selected one
			{
				mFolderStack.push(mFolder);
				mFolder = (FolderData*)file;
				updateList();
			}else{
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
	}

	if(button == InputManager::RIGHT && keyDown)
	{
		setSystemId(mSystemId + 1);
	}
	if(button == InputManager::LEFT && keyDown)
	{
		setSystemId(mSystemId - 1);
	}

	if(button == InputManager::MENU && keyDown)
	{
		new GuiMenu(this);
	}

	if(mDetailed)
	{
		if(!keyDown && (button == InputManager::UP || button == InputManager::DOWN))
		{
			updateDetailData();
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
	mList->setCentered(mTheme->getListCentered());
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
	InputManager::unregisterComponent(this);
}

void GuiGameList::onResume()
{
	InputManager::registerComponent(this);
}
