#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>

GuiGameList::GuiGameList()
{
	std::cout << "Creating GuiGameList\n";

	mList = new GuiList();
	addChild(mList);

	setSystemId(0);

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);
}

GuiGameList::~GuiGameList()
{
	Renderer::unregisterComponent(this);
	delete mList;

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

	Renderer::drawCenteredText(mSystem->getName(), 2, 0x0000FF);
}

void GuiGameList::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::BUTTON1 && mFolder->getFileCount() > 0)
	{
		if(!keyDown)
		{
			FileData* file = (FileData*)mList->getSelectedObject();
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

	//std::cout << "mFolderStack.size(): " << mFolderStack.size() << "\n";
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
}

void GuiGameList::updateList()
{
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
