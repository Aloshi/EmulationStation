#include "GuiFastSelect.h"
#include "../Renderer.h"
#include <iostream>
#include "GuiGameList.h"
#include "../FileSorts.h"

#define DEFAULT_FS_IMAGE ":/frame.png"

const std::string GuiFastSelect::LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int GuiFastSelect::SCROLLSPEED = 100;
const int GuiFastSelect::SCROLLDELAY = 507;

int GuiFastSelect::sortTypeId = 0;

GuiFastSelect::GuiFastSelect(Window* window, GuiGameList* parent, TextListComponent<FileData*>* list, char startLetter, ThemeComponent * theme)
	: GuiComponent(window), mParent(parent), mList(list), mTheme(theme), mBox(mWindow, "")
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


	if(theme->getString("fastSelectFrame").empty())
	{
		mBox.setImagePath(DEFAULT_FS_IMAGE);
		//mBox.setEdgeColor(0x0096ffFF);
		mBox.setEdgeColor(0x005493FF);
		mBox.setCenterColor(0x5e5e5eFF);
	}else{
		mBox.setImagePath(theme->getString("fastSelectFrame"));
	}

	mBox.setPosition(sw * 0.2f, sh * 0.2f);
	mBox.setSize(sw * 0.6f, sh * 0.6f);
}

GuiFastSelect::~GuiFastSelect()
{
	mParent->updateDetailData();
}

void GuiFastSelect::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();

	unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();

	mBox.render(trans);

	Renderer::setMatrix(trans);
	std::shared_ptr<Font> letterFont = mTheme->getFastSelectFont();
	std::shared_ptr<Font> subtextFont = mTheme->getDescriptionFont();

	letterFont->drawCenteredText(LETTERS.substr(mLetterID, 1), 0, sh * 0.5f - (letterFont->getHeight() * 0.5f), mTextColor);
    subtextFont->drawCenteredText("Sort order:", 0, sh * 0.6f - (subtextFont->getHeight() * 0.5f), mTextColor);

	std::string sortString = "<- " + FileSorts::SortTypes.at(sortTypeId).description + " ->";
    subtextFont->drawCenteredText(sortString, 0, sh * 0.6f + (subtextFont->getHeight() * 0.5f), mTextColor);
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
		sortTypeId--;
		if(sortTypeId < 0)
			sortTypeId = FileSorts::SortTypes.size() - 1;

        mScrollSound->play();
		return true;
	}
    else if(config->isMappedTo("right", input) && input.value != 0)
	{
		sortTypeId = (sortTypeId + 1) % FileSorts::SortTypes.size();
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
		mParent->sort(FileSorts::SortTypes.at(sortTypeId));
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
