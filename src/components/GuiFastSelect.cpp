#include "GuiFastSelect.h"
#include "../Renderer.h"
#include <iostream>
#include "GuiGameList.h"

const std::string GuiFastSelect::LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int GuiFastSelect::SCROLLSPEED = 100;
const int GuiFastSelect::SCROLLDELAY = 507;

GuiFastSelect::GuiFastSelect(Window* window, GuiGameList* parent, TextListComponent<FileData*>* list, char startLetter, ThemeComponent * theme)
    : GuiComponent(window), mParent(parent), mList(list), mTheme(theme)
{
	mLetterID = LETTERS.find(toupper(startLetter));
	if(mLetterID == std::string::npos)
		mLetterID = 0;

    mScrollSound = mTheme->getSound("menuScroll");
    mTextColor = mTheme->getColor("fastSelect");

	mScrolling = false;
	mScrollTimer = 0;
	mScrollOffset = 0;

	unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();
	mBox = new GuiBox(window, (int)(sw * 0.2f), (int)(sh * 0.2f), (int)(sw * 0.6f), (int)(sh * 0.6f));
	mBox->setData(mTheme->getBoxData());
}

GuiFastSelect::~GuiFastSelect()
{
	mParent->updateDetailData();
	delete mBox;
}

void GuiFastSelect::render()
{
	unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();

	if(!mBox->hasBackground())
		Renderer::drawRect((int)(sw * 0.3f), (int)(sh * 0.3f), (int)(sw * 0.4f), (int)(sh * 0.4f), 0x000FF0AA);

	mBox->render();

	std::shared_ptr<Font> letterFont = mTheme->getFastSelectFont();
	std::shared_ptr<Font> subtextFont = mTheme->getDescriptionFont();

	letterFont->drawCenteredText(LETTERS.substr(mLetterID, 1), 0, (int)(sh * 0.5f - (letterFont->getHeight() * 0.5f)), mTextColor);
    subtextFont->drawCenteredText("Sort order:", 0, (int)(sh * 0.6f - (subtextFont->getHeight() * 0.5f)), mTextColor);

    std::string sortString = "<- " + mParent->getSortState().description + " ->";
    subtextFont->drawCenteredText(sortString, 0, (int)(sh * 0.6f + (subtextFont->getHeight() * 0.5f)), mTextColor);
}

bool GuiFastSelect::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("up", input) && input.value != 0)
	{
		mScrollOffset = -1;
		scroll();
		return true;
	}

	if(config->isMappedTo("down", input) && input.value != 0)
	{
		mScrollOffset = 1;
		scroll();
		return true;
	}

	if(config->isMappedTo("left", input) && input.value != 0)
	{
		mParent->setPreviousSortIndex();
        mScrollSound->play();
		return true;
	}
    else if(config->isMappedTo("right", input) && input.value != 0)
	{
		mParent->setNextSortIndex();
        mScrollSound->play();
		return true;
	}

	if((config->isMappedTo("up", input) || config->isMappedTo("down", input)) && input.value == 0)
	{
		mScrolling = false;
		mScrollTimer = 0;
		mScrollOffset = 0;
		return true;
	}

	if(config->isMappedTo("select", input) && input.value == 0)
	{
		setListPos();
		delete this;
		return true;
	}

	return false;
}

void GuiFastSelect::update(int deltaTime)
{
	if(mScrollOffset != 0)
	{
		mScrollTimer += deltaTime;

		if(!mScrolling && mScrollTimer >= SCROLLDELAY)
		{
			mScrolling = true;
			mScrollTimer = SCROLLSPEED;
		}

		if(mScrolling && mScrollTimer >= SCROLLSPEED)
		{
			mScrollTimer = 0;
			scroll();
		}
	}
}

void GuiFastSelect::scroll()
{
	setLetterID(mLetterID + mScrollOffset);
	mScrollSound->play();
}

void GuiFastSelect::setLetterID(int id)
{
	while(id < 0)
		id += LETTERS.length();
	while(id >= (int)LETTERS.length())
		id -= LETTERS.length();

	mLetterID = (size_t)id;
}

void GuiFastSelect::setListPos()
{
	char letter = LETTERS[mLetterID];

	int min = 0;
	int max = mList->getObjectCount() - 1;

	int mid = 0;

	while(max >= min)
	{
		mid = ((max - min) / 2) + min;

		char checkLetter = toupper(mList->getObject(mid)->getName()[0]);

		if(checkLetter < letter)
		{
			min = mid + 1;
		}else if(checkLetter > letter)
		{
			max = mid - 1;
		}else{
			//exact match found
			break;
		}
	}

	mList->setSelection(mid);
}
