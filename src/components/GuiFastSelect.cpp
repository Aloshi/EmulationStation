#include "GuiFastSelect.h"
#include "../Renderer.h"
#include <iostream>

const std::string GuiFastSelect::LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int GuiFastSelect::SCROLLSPEED = 100;
const int GuiFastSelect::SCROLLDELAY = 507;

GuiFastSelect::GuiFastSelect(GuiComponent* parent, GuiList<FileData*>* list, char startLetter)
{
	mLetterID = LETTERS.find(toupper(startLetter));
	if(mLetterID == std::string::npos)
		mLetterID = 0;

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);

	mParent = parent;
	mList = list;

	mScrolling = false;
	mScrollTimer = 0;
	mScrollOffset = 0;

	unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();
	mBox = new GuiBox(sw * 0.2, sh * 0.2, sw * 0.6, sh * 0.6);
	addChild(mBox);

	//set test mBox info
	//mBox->setHorizontalImage("test.jpg");

	mParent->pause();
}

GuiFastSelect::~GuiFastSelect()
{
	Renderer::unregisterComponent(this);
	InputManager::unregisterComponent(this);

	removeChild(mBox);
	delete mBox;

	mParent->resume();
}

void GuiFastSelect::onRender()
{
	unsigned int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();

	Renderer::drawRect(sw * 0.2, sh * 0.2, sw * 0.6, sh * 0.6, 0x000FF0);
	Renderer::drawCenteredText(LETTERS.substr(mLetterID, 1), 0, sh * 0.5 - (Renderer::getFontHeight(Renderer::LARGE) * 0.5), 0xFF0000, Renderer::LARGE);
}

void GuiFastSelect::onInput(InputManager::InputButton button, bool keyDown)
{
	if(button == InputManager::UP && keyDown)
	{
		setLetterID(mLetterID - 1);
		mScrollOffset = -1;
	}

	if(button == InputManager::DOWN && keyDown)
	{
		setLetterID(mLetterID + 1);
		mScrollOffset = 1;
	}

	if((button == InputManager::UP || button == InputManager::DOWN) && !keyDown)
	{
		mScrolling = false;
		mScrollTimer = 0;
		mScrollOffset = 0;
	}

	if(button == InputManager::SELECT && !keyDown)
	{
		setListPos();
		delete this;
		return;
	}
}

void GuiFastSelect::onTick(int deltaTime)
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
			setLetterID(mLetterID + mScrollOffset);
		}
	}
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

	int mid;

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
