#include "GuiGameList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"
#include "GuiFastSelect.h"
#include <boost/filesystem.hpp>
#include "../Log.h"
#include "../Settings.h"


std::vector<FolderData::SortState> GuiGameList::sortStates;


Eigen::Vector3f GuiGameList::getImagePos()
{
	return Eigen::Vector3f(Renderer::getScreenWidth() * mTheme->getFloat("gameImageOffsetX"), Renderer::getScreenHeight() * mTheme->getFloat("gameImageOffsetY"), 0.0f);
}

bool GuiGameList::isDetailed() const
{
	if(mSystem == NULL)
		return false;

	return mSystem->hasGamelist();
}

GuiGameList::GuiGameList(Window* window) : GuiComponent(window), 
	mTheme(new ThemeComponent(mWindow)),
	mList(window, 0.0f, 0.0f, Font::get(*window->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_MEDIUM)), 
	mScreenshot(window),
	mDescription(window), 
	mDescContainer(window), 
	mTransitionImage(window, 0.0f, 0.0f, "", (float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight(), true), 
	mHeaderText(mWindow), 
    sortStateIndex(Settings::getInstance()->getInt("GameListSortIndex"))
{
	//first object initializes the vector
	if (sortStates.empty()) {
		sortStates.push_back(FolderData::SortState(FolderData::compareFileName, true, "file name, ascending"));
		sortStates.push_back(FolderData::SortState(FolderData::compareFileName, false, "file name, descending"));
		sortStates.push_back(FolderData::SortState(FolderData::compareRating, true, "database rating, ascending"));
		sortStates.push_back(FolderData::SortState(FolderData::compareRating, false, "database rating, descending"));
		sortStates.push_back(FolderData::SortState(FolderData::compareUserRating, true, "your rating, ascending"));
		sortStates.push_back(FolderData::SortState(FolderData::compareUserRating, false, "your rating, descending"));
        sortStates.push_back(FolderData::SortState(FolderData::compareTimesPlayed, true, "played least often"));
        sortStates.push_back(FolderData::SortState(FolderData::compareTimesPlayed, false, "played most often"));
		sortStates.push_back(FolderData::SortState(FolderData::compareLastPlayed, true, "played least recently"));
		sortStates.push_back(FolderData::SortState(FolderData::compareLastPlayed, false, "played most recently"));
	}

	mImageAnimation.addChild(&mScreenshot);
	mDescContainer.addChild(&mDescription);

	//scale delay with screen width (higher width = more text per line)
	//the scroll speed is automatically scaled by component size
	mDescContainer.setAutoScroll((int)(1500 + (Renderer::getScreenWidth() * 0.5)), 0.025f);

	mTransitionImage.setPosition((float)Renderer::getScreenWidth(), 0);
	mTransitionImage.setOrigin(0, 0);

	mHeaderText.setColor(0xFF0000FF);
	mHeaderText.setFont(Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_LARGE));
	mHeaderText.setPosition(0, 1);
	mHeaderText.setSize((float)Renderer::getScreenWidth(), 0);
	mHeaderText.setCentered(true);

	addChild(mTheme);
	addChild(&mHeaderText);
	addChild(&mScreenshot);
	addChild(&mDescContainer);
	addChild(&mList);
	addChild(&mTransitionImage);

	mTransitionAnimation.addChild(this);

	setSystemId(0);
}

GuiGameList::~GuiGameList()
{
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
	mWindow->normalizeNextUpdate(); //image loading can be slow
}

void GuiGameList::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	renderChildren(trans);
}

bool GuiGameList::input(InputConfig* config, Input input)
{
	mList.input(config, input);

	if(config->isMappedTo("a", input) && mFolder->getFileCount() > 0 && input.value != 0)
	{
		//play select sound
		mTheme->getSound("menuSelect")->play();

		FileData* file = mList.getSelectedObject();
		if(file->isFolder()) //if you selected a folder, add this directory to the stack, and use the selected one
		{
			mFolderStack.push(mFolder);
			mFolder = (FolderData*)file;
			updateList();
			updateDetailData();
			return true;
		}else{
			mList.stopScrolling();

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

	//change sort order
	if(config->isMappedTo("sortordernext", input) && input.value != 0) {
		setNextSortIndex();
		//std::cout << "Sort order is " << FolderData::getSortStateName(sortStates.at(sortStateIndex).comparisonFunction, sortStates.at(sortStateIndex).ascending) << std::endl;
	}
	else if(config->isMappedTo("sortorderprevious", input) && input.value != 0) {
		setPreviousSortIndex();
		//std::cout << "Sort order is " << FolderData::getSortStateName(sortStates.at(sortStateIndex).comparisonFunction, sortStates.at(sortStateIndex).ascending) << std::endl;
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
        mWindow->pushGui(new GuiFastSelect(mWindow, this, &mList, mList.getSelectedObject()->getName()[0], mTheme));
		return true;
	}

	if(isDetailed())
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

const FolderData::SortState & GuiGameList::getSortState() const
{
    return sortStates.at(sortStateIndex);
}

void GuiGameList::setSortIndex(size_t index)
{
	//make the index valid
	if (index >= sortStates.size()) {
		index = 0;
	}
	if (index != sortStateIndex) {
		//get sort state from vector and sort list
		sortStateIndex = index;
		sort(sortStates.at(sortStateIndex).comparisonFunction, sortStates.at(sortStateIndex).ascending);
	}
    //save new index to settings
    Settings::getInstance()->setInt("GameListSortIndex", sortStateIndex);
}

void GuiGameList::setNextSortIndex()
{
	//make the index wrap around
	if ((sortStateIndex - 1) >= sortStates.size()) {
		setSortIndex(0);
	}
	setSortIndex(sortStateIndex + 1);
}

void GuiGameList::setPreviousSortIndex()
{
	//make the index wrap around
	if (((int)sortStateIndex - 1) < 0) {
		setSortIndex(sortStates.size() - 1);
	}
	setSortIndex(sortStateIndex - 1);
}

void GuiGameList::sort(FolderData::ComparisonFunction & comparisonFunction, bool ascending)
{
	//resort list and update it
	mFolder->sort(comparisonFunction, ascending);
	updateList();
	updateDetailData();
}

void GuiGameList::updateList()
{
	mList.clear();

	for(unsigned int i = 0; i < mFolder->getFileCount(); i++)
	{
		FileData* file = mFolder->getFile(i);

		if(file->isFolder())
			mList.addObject(file->getName(), file, mTheme->getColor("secondary"));
		else
			mList.addObject(file->getName(), file, mTheme->getColor("primary"));
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
	mTheme->readXML(getThemeFile(), isDetailed());

	mList.setSelectorColor(mTheme->getColor("selector"));
	mList.setSelectedTextColor(mTheme->getColor("selected"));
	mList.setScrollSound(mTheme->getSound("menuScroll"));

	mList.setFont(mTheme->getListFont());
	mList.setPosition(0.0f, Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_LARGE)->getHeight() + 2.0f);

	if(!mTheme->getBool("hideHeader"))
	{
		mHeaderText.setText(mSystem->getDescName());
	}else{
		mHeaderText.setText("");
	}

	if(isDetailed())
	{
		mList.setCentered(mTheme->getBool("listCentered"));

		mList.setPosition(mTheme->getFloat("listOffsetX") * Renderer::getScreenWidth(), mList.getPosition().y());
		mList.setTextOffsetX((int)(mTheme->getFloat("listTextOffsetX") * Renderer::getScreenWidth()));

		mScreenshot.setPosition(mTheme->getFloat("gameImageOffsetX") * Renderer::getScreenWidth(), mTheme->getFloat("gameImageOffsetY") * Renderer::getScreenHeight());
		mScreenshot.setOrigin(mTheme->getFloat("gameImageOriginX"), mTheme->getFloat("gameImageOriginY"));
		mScreenshot.setResize(mTheme->getFloat("gameImageWidth") * Renderer::getScreenWidth(), mTheme->getFloat("gameImageHeight") * Renderer::getScreenHeight(), false);

		mDescription.setColor(mTheme->getColor("description"));
		mDescription.setFont(mTheme->getDescriptionFont());
	}else{
		mList.setCentered(true);
		mList.setPosition(0, mList.getPosition().y());
		mList.setTextOffsetX(0);
	}
}

void GuiGameList::updateDetailData()
{
	if(!isDetailed())
	{
		mScreenshot.setImage("");
		mDescription.setText("");
	}else{
		//if we've selected a game
		if(mList.getSelectedObject() && !mList.getSelectedObject()->isFolder())
		{
			//set image to either "not found" image or metadata image
			if(((GameData*)mList.getSelectedObject())->getImagePath().empty())
				mScreenshot.setImage(mTheme->getString("imageNotFoundPath"));
			else
				mScreenshot.setImage(((GameData*)mList.getSelectedObject())->getImagePath());

			Eigen::Vector3f imgOffset = Eigen::Vector3f(Renderer::getScreenWidth() * 0.10f, 0, 0);
			mScreenshot.setPosition(getImagePos() - imgOffset);

			mImageAnimation.fadeIn(35);
			mImageAnimation.move(imgOffset.x(), imgOffset.y(), 20);

			mDescContainer.setPosition(Eigen::Vector3f(Renderer::getScreenWidth() * 0.03f, getImagePos().y() + mScreenshot.getSize().y() + 12, 0));
			mDescContainer.setSize(Eigen::Vector2f(Renderer::getScreenWidth() * (mTheme->getFloat("listOffsetX") - 0.03f), Renderer::getScreenHeight() - mDescContainer.getPosition().y()));
			mDescContainer.setScrollPos(Eigen::Vector2d(0, 0));
			mDescContainer.resetAutoScrollTimer();

			mDescription.setPosition(0, 0);
			mDescription.setSize(Eigen::Vector2f(Renderer::getScreenWidth() * (mTheme->getFloat("listOffsetX") - 0.03f), 0));
			mDescription.setText(((GameData*)mList.getSelectedObject())->getDescription());
		}else{
			mScreenshot.setImage("");
			mDescription.setText("");
		}
	}
}

void GuiGameList::clearDetailData()
{
	if(isDetailed())
	{
		mImageAnimation.fadeOut(35);
		mDescription.setText("");
	}
}

GuiGameList* GuiGameList::create(Window* window)
{
	GuiGameList* list = new GuiGameList(window);
	window->pushGui(list);
	return list;
}

void GuiGameList::update(int deltaTime)
{
	mTransitionAnimation.update(deltaTime);
	mImageAnimation.update(deltaTime);
	GuiComponent::update(deltaTime);
}

void GuiGameList::doTransition(int dir)
{
	mTransitionImage.copyScreen();
	mTransitionImage.setOpacity(255);

	//put the image of what's currently onscreen at what will be (in screen coords) 0, 0
	mTransitionImage.setPosition((float)Renderer::getScreenWidth() * dir, 0);

	//move the entire thing offscreen so we'll move into place
	setPosition((float)Renderer::getScreenWidth() * -dir, mPosition[1]);

	mTransitionAnimation.move(Renderer::getScreenWidth() * dir, 0, 50);
}
