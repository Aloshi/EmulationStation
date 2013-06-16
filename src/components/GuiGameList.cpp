#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"
#include "GuiFastSelect.h"
#include <boost/filesystem.hpp>
#include "../Log.h"

Vector2i GuiGameList::getImagePos()
{
	return Vector2i((int)(Renderer::getScreenWidth() * mTheme->getFloat("gameImageOffsetX")), (int)(Renderer::getScreenHeight() * mTheme->getFloat("gameImageOffsetY")));
}

GuiGameList::GuiGameList(Window* window, bool useDetail) : GuiComponent(window), 
	mDescription(window), mTransitionImage(window, 0, 0, "", Renderer::getScreenWidth(), Renderer::getScreenHeight(), true)
{
	mDetailed = useDetail;

	mTheme = new ThemeComponent(mWindow, mDetailed);

	//The GuiGameList can use the older, simple game list if so desired.
	//The old view only shows a list in the center of the screen; the new view can display an image and description.
	//Those with smaller displays may prefer the older view.
	if(mDetailed)
	{
		mList = new TextListComponent<FileData*>(mWindow, (int)(Renderer::getScreenWidth() * mTheme->getFloat("listOffsetX")), Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, Renderer::getDefaultFont(Renderer::MEDIUM));

		mScreenshot = new ImageComponent(mWindow, getImagePos().x, getImagePos().y, "", (unsigned int)mTheme->getFloat("gameImageWidth"), (unsigned int)mTheme->getFloat("gameImageHeight"), false);
		mScreenshot->setOrigin(mTheme->getFloat("gameImageOriginX"), mTheme->getFloat("gameImageOriginY"));

		mImageAnimation = new AnimationComponent();
		mImageAnimation->addChild(mScreenshot);
	}else{
		mList = new TextListComponent<FileData*>(mWindow, 0, Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, Renderer::getDefaultFont(Renderer::MEDIUM));
		mScreenshot = NULL;
		mImageAnimation = NULL;
	}

	mDescription.setOffset(Vector2i((int)(Renderer::getScreenWidth() * 0.03), mScreenshot->getOffset().y + mScreenshot->getSize().y + 12));
	mDescription.setExtent(Vector2u((int)(Renderer::getScreenWidth() * (mTheme->getFloat("listOffsetX") - 0.03)), 0));
	
	mTransitionImage.setOffset(Renderer::getScreenWidth(), 0);
	mTransitionImage.setOrigin(0, 0);
	mTransitionAnimation.addChild(&mTransitionImage);

	//a hack! the GuiGameList doesn't use the children system right now because I haven't redone it to do so yet.
	//the list depends on knowing it's final window coordinates (getGlobalOffset), which requires knowing the where the GuiGameList is.
	//the GuiGameList now moves during screen transitions, so we have to let it know somehow.
	//this should be removed in favor of using real children soon.
	mList->setParent(this);

	setSystemId(0);
}

GuiGameList::~GuiGameList()
{
	//undo the parenting hack because otherwise it's not really a child and will try to remove itself on delete
	mList->setParent(NULL);
	delete mList;

	if(mDetailed)
	{
		delete mImageAnimation;
		delete mScreenshot;
	}

	delete mTheme;
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
	if(mTransitionImage.getOffset().x > 0) //transitioning in from the left
		mOffset.x = mTransitionImage.getOffset().x - Renderer::getScreenWidth();
	else //transitioning in from the right
		mOffset.x = mTransitionImage.getOffset().x + Renderer::getScreenWidth();

	Renderer::translate(mOffset);

	if(mTheme)
		mTheme->render();

	//header
	if(!mTheme->getBool("hideHeader"))
		Renderer::drawCenteredText(mSystem->getDescName(), 0, 1, 0xFF0000FF, Renderer::getDefaultFont(Renderer::LARGE));

	if(mDetailed)
	{
		//divider
		if(!mTheme->getBool("hideDividers"))
			Renderer::drawRect((int)(Renderer::getScreenWidth() * mTheme->getFloat("listOffsetX")) - 4, Renderer::getDefaultFont(Renderer::LARGE)->getHeight() + 2, 8, Renderer::getScreenHeight(), 0x0000FFFF);
		
		mScreenshot->render();
		
		//if we're not scrolling and we have selected a non-folder
		if(!mList->isScrolling() && !mList->getSelectedObject()->isFolder())
		{
			mDescription.render();
		}
	}

	mList->render();

	Renderer::translate(-mOffset);

	mTransitionImage.render();
}

bool GuiGameList::input(InputConfig* config, Input input)
{
	mList->input(config, input);

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
			updateDetailData();
			return true;
		}else{
			mList->stopScrolling();

			//wait for the sound to finish or we'll never hear it...
			while(mTheme->getSound("menuSelect")->isPlaying());

			mSystem->launchGame(mWindow, (GameData*)file);
			return true;
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

		return true;
	}

	//only allow switching systems if more than one exists (otherwise it'll reset your position when you switch and it's annoying)
	if(SystemData::sSystemVector.size() > 1 && input.value != 0)
	{
		if(config->isMappedTo("right", input))
		{
			setSystemId(mSystemId + 1);
			doTransition(-1);
			return true;
		}
		if(config->isMappedTo("left", input))
		{
			setSystemId(mSystemId - 1);
			doTransition(1);
			return true;
		}
	}

	//open the "start menu"
	if(config->isMappedTo("menu", input) && input.value != 0)
	{
		mWindow->pushGui(new GuiMenu(mWindow, this));
		return true;
	}

	//open the fast select menu
	if(config->isMappedTo("select", input) && input.value != 0)
	{
		mWindow->pushGui(new GuiFastSelect(mWindow, this, mList, mList->getSelectedObject()->getName()[0], mTheme->getBoxData(), mTheme->getColor("fastSelect"), mTheme->getSound("menuScroll"), mTheme->getFastSelectFont()));
		return true;
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
		return true;
	}

	return false;
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

	themePath = getHomePath();
	themePath += "/.emulationstation/" +  mSystem->getName() + "/theme.xml";
	if(boost::filesystem::exists(themePath))
		return themePath;

	themePath = mSystem->getStartPath() + "/theme.xml";
	if(boost::filesystem::exists(themePath))
		return themePath;

	themePath = getHomePath();
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

		mList->setOffset((int)(mTheme->getFloat("listOffsetX") * Renderer::getScreenWidth()), mList->getOffset().y);
		mList->setTextOffsetX((int)(mTheme->getFloat("listTextOffsetX") * Renderer::getScreenWidth()));

		mScreenshot->setOffset((int)(mTheme->getFloat("gameImageOffsetX") * Renderer::getScreenWidth()), (int)(mTheme->getFloat("gameImageOffsetY") * Renderer::getScreenHeight()));
		mScreenshot->setOrigin(mTheme->getFloat("gameImageOriginX"), mTheme->getFloat("gameImageOriginY"));
		mScreenshot->setResize((int)mTheme->getFloat("gameImageWidth"), (int)mTheme->getFloat("gameImageHeight"), false);

		mDescription.setColor(mTheme->getColor("description"));
		mDescription.setFont(mTheme->getDescriptionFont());
	}
}

void GuiGameList::updateDetailData()
{
	if(!mDetailed)
		return;

	if(mList->getSelectedObject() && !mList->getSelectedObject()->isFolder())
	{
		if(((GameData*)mList->getSelectedObject())->getImagePath().empty())
			mScreenshot->setImage(mTheme->getString("imageNotFoundPath"));
		else
			mScreenshot->setImage(((GameData*)mList->getSelectedObject())->getImagePath());

		Vector2i imgOffset = Vector2i((int)(Renderer::getScreenWidth() * 0.10f), 0);
		mScreenshot->setOffset(getImagePos() - imgOffset);

		mImageAnimation->fadeIn(35);
		mImageAnimation->move(imgOffset.x, imgOffset.y, 20);

		mDescription.setOffset(Vector2i((int)(Renderer::getScreenWidth() * 0.03), mScreenshot->getOffset().y + mScreenshot->getSize().y + 12));
		mDescription.setText(((GameData*)mList->getSelectedObject())->getDescription());
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
void GuiGameList::deinit()
{
	if(mDetailed)
	{
		mScreenshot->deinit();
	}

	mTheme->deinit();
}

void GuiGameList::init()
{
	mTheme->init();

	if(mDetailed)
	{
		mScreenshot->init();
	}
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

void GuiGameList::update(int deltaTime)
{
	if(mDetailed)
		mImageAnimation->update(deltaTime);

	mTransitionAnimation.update(deltaTime);

	mList->update(deltaTime);
}

void GuiGameList::doTransition(int dir)
{
	mTransitionImage.copyScreen();
	mTransitionImage.setOpacity(255);
	mTransitionImage.setOffset(0, 0);
	mTransitionAnimation.move(Renderer::getScreenWidth() * dir, 0, 50);
}
