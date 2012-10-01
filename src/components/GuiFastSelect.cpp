#include "GuiFastSelect.h"
#include "../Renderer.h"
#include <iostream>

const std::string GuiFastSelect::LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int GuiFastSelect::SCROLLSPEED = 120;
const int GuiFastSelect::SCROLLDELAY = 507;

GuiFastSelect::GuiFastSelect(GuiComponent* parent, SystemData* system, char startLetter)
{
	mLetterID = LETTERS.find(toupper(startLetter));
	if(mLetterID == std::string::npos)
		mLetterID = 0;

	Renderer::registerComponent(this);
	InputManager::registerComponent(this);

	mParent = parent;
	mSystem = system;

	mScrolling = false;
	mScrollTimer = 0;
	mScrollOffset = 0;

	mParent->pause();
}

GuiFastSelect::~GuiFastSelect()
{
	Renderer::unregisterComponent(this);
	InputManager::unregisterComponent(this);

	mParent->resume();
}

void GuiFastSelect::onRender()
{
	int sw = Renderer::getScreenWidth(), sh = Renderer::getScreenHeight();

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
