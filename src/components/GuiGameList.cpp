#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"

#define SCREENSHOTWIDTH 256
#define SCREENSHOTHEIGHT 256

GuiGameList::GuiGameList(bool useDetail)
{
	std::cout << "Creating GuiGameList\n";
	mDetailed = useDetail;

	//The GuiGameList can use the older, simple game list if so desired.
	//The old view only shows a list in the center of the screen; the new view can display a screenshto and description.
	//Those with smaller displays may prefer the older view.
	if(mDetailed)
	{
		mList = new GuiList<FileData*>(Renderer::getScreenWidth() * 0.4, Renderer::getFontHeight(Renderer::LARGE) + 2);

		mScreenshot = new GuiImage(Renderer::getScreenWidth() * 0.2, Renderer::getFontHeight(Renderer::LARGE) + 2, "", Renderer::getScreenWidth() * 0.3);
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

	updateList();
}

void GuiGameList::onRender()
{
	Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0xFFFFFF);

	Renderer::drawCenteredText(mSystem->getName(), 0, 1, 0x0000FF, Renderer::LARGE);


	if(mDetailed)
	{
		Renderer::drawRect(Renderer::getScreenWidth() * 0.4, Renderer::getFontHeight(Renderer::LARGE) + 2, 8, Renderer::getScreenHeight(), 0x0000FF);

		//if we have selected a non-folder
		if(mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
		{
			GameData* game = (GameData*)mList->getSelectedObject();

			//todo: cache this
			std::string desc = game->getDescription();
			if(!desc.empty())
				Renderer::drawWrappedText(desc, 2, Renderer::getFontHeight(Renderer::LARGE) + SCREENSHOTHEIGHT + 12, Renderer::getScreenWidth() * 0.4, 0xFF0000);
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
			if(file->isFolder())
			{
				//set current directory to this or something
				mFolderStack.push(mFolder);
				mFolder = (FolderData*)file;
				updateList();
			}else{
				mSystem->launchGame((GameData*)file);
			}
		}
	}

	if(button == InputManager::BUTTON2 && keyDown && mFolderStack.size())
	{
		mFolder = mFolderStack.top();
		mFolderStack.pop();
		updateList();
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
			if(mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
			{
				mScreenshot->setImage(((GameData*)mList->getSelectedObject())->getImagePath());
			}else{
				mScreenshot->setImage("");
			}
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
			mList->addObject(file->getName(), file, 0x00C000);
		else
			mList->addObject(file->getName(), file);
	}
}

//these are called when the menu opens/closes
//the second bit should be moved to GuiList
void GuiGameList::onPause()
{
	InputManager::unregisterComponent(this);
	InputManager::unregisterComponent(mList);
}

void GuiGameList::onResume()
{
	InputManager::registerComponent(this);
	InputManager::registerComponent(mList);
}
