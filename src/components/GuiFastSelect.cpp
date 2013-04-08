#include "GuiFastSelect.h"
#include "../Renderer.h"
#include <iostream>
#include "GuiGameList.h"

const std::string GuiFastSelect::LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int GuiFastSelect::SCROLLSPEED = 100;
const int GuiFastSelect::SCROLLDELAY = 507;

GuiFastSelect::GuiFastSelect(Window* window, GuiGameList* parent, GuiList<FileData*>* list, char startLetter, GuiBoxData data, 
	int textcolor, Sound* scrollsound, Font* font) : Gui(window)
{
	mLetterID = LETTERS.find(toupper(startLetter));
	if(mLetterID == std::string::npos)
		mLetterID = 0;

	mParent = parent;
	mList = list;
	mScrollSound = scrollsound;
	mFont = font;

	mScrolling = false;
	mScrollTimer = 0;
	mScrollOffset = 0;

	unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();
	mBox = new GuiBox(window, sw * 0.2, sh * 0.2, sw * 0.6, sh * 0.6);
	mBox->setData(data);

	mTextColor = textcolor;
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
		Renderer::drawRect(sw * 0.2, sh * 0.2, sw * 0.6, sh * 0.6, 0x000FF0FF);

	mBox->render();

	Renderer::drawCenteredText(LETTERS.substr(mLetterID, 1), 0, sh * 0.5 - (mFont->getHeight() * 0.5), mTextColor, mFont);
}

void GuiFastSelect::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("up", input) && input.value != 0)
	{
		mScrollOffset = -1;
		scroll();
	}

	if(config->isMappedTo("down", input) && input.value != 0)
	{
		mScrollOffset = 1;
		scroll();
	}

	if((config->isMappedTo("up", input) || config->isMappedTo("down", input)) && input.value == 0)
	{
		mScrolling = false;
		mScrollTimer = 0;
		mScrollOffset = 0;
	}

	if(config->isMappedTo("select", input) && input.value == 0)
	{
		setListPos();
		delete this;
		return;
	}
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
