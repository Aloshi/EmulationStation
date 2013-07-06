#include "GuiConsoleList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"
#include "GuiFastSelect.h"
#include <boost/filesystem.hpp>
#include "../Log.h"
#include "../Settings.h"

GuiConsoleList::GuiConsoleList(Window* window, GuiGameList* gameList) : GuiComponent(window)
{
	mCurrentIndex = -1;
	mGameList = gameList;

	mAnimator = new AnimationComponent();
	mAnimator->addChild(this);

	if(SystemData::sSystemVector.size() == 0)
	{
		LOG(LogError) << "Error - no systems found!";
		return;
	}

	ImageComponent* back = new ImageComponent(window);
	back->setTiling(true);
	back->setOrigin(0, 0);
	back->setImage("c:\\retromania\\consoles\\bg_pattern.png");
	this->addChild(back);

	for(unsigned int i = 0; i < SystemData::sSystemVector.size(); i++)
	{

		SystemData* data = SystemData::sSystemVector.at(i);

		std::string name = data->getDescName();

		std::string image = data->getImage();
		ImageComponent* img = NULL;
		
		if( image.compare("none") != 0 )
			img = new ImageComponent(window, Renderer::getScreenWidth()/2 + Renderer::getScreenWidth()/2 * i, Renderer::getScreenHeight() / 2, image, 0, Renderer::getScreenHeight()/2.5, true);
		else
			img = new ImageComponent(window, Renderer::getScreenWidth()/2 + Renderer::getScreenWidth()/2 * i, Renderer::getScreenHeight() / 2,  SystemData::getBlankConsoleImagePath(), 0, Renderer::getScreenHeight()/2.5, true);

		AnimationComponent* label = new AnimationComponent();
		TextComponent* text = new TextComponent(window);
		text->setText(name);
		text->setFont(Renderer::getDefaultFont(Renderer::LARGE));
		text->setOpacity(255);
		text->setColor(0xFFFFFFFF);
		text->setOffset(Renderer::getScreenWidth()/2 + Renderer::getScreenWidth()/2 * i - text->getSize().x/2, Renderer::getScreenHeight());
		label->addChild(text);

		ConsoleItem newConsole = {name, img, label, text};
		mConsoleVector.push_back(newConsole);
		
		this->addChild(newConsole.image);
		//add text label after console image
		this->addChild( text );
	}

	this->mSize = Vector2u( Renderer::getScreenWidth()/2 * (mConsoleVector.size()+1) , Renderer::getScreenHeight());
	back->setResize(mSize.x, mSize.y, true);

	setCurrentIndex(0);
}

void GuiConsoleList::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	for(unsigned int i = 0; i < mConsoleVector.size(); i++)
	{
		mConsoleVector.at(i).label->update(deltaTime);
	}

	mAnimator->update(deltaTime);
}

std::vector<ConsoleItem>* GuiConsoleList::getConsoles()
{
	return &mConsoleVector;
}

bool GuiConsoleList::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("right", input) && input.value != 0)
	{
		goToNext();
		return true;
	}

	if(config->isMappedTo("left", input)  && input.value != 0 )
	{
		goToPrev();
		return true;
	}

	if(config->isMappedTo("a", input))
	{
		mWindow->pushGui(mGameList);
		mGameList->setSystemId(mCurrentIndex);
		mGameList->doVerticalTransition(-1);
		return true;
	}

	//open the "start menu"
	if(config->isMappedTo("menu", input) && input.value != 0)
	{
		mWindow->pushGui(new GuiMenu(mWindow, mGameList));
		return true;
	}

	return false;
}

bool GuiConsoleList::goToNext()
{
	if(mCurrentIndex+1 < mConsoleVector.size())
	{
		//avoid animation corruption
		if(mAnimator->isAnimating()) return false;

		mCurrentIndex++;
		mAnimator->move(-(Renderer::getScreenWidth() / 2), 0, 50);

		if(mCurrentIndex-1 >= 0)
			mConsoleVector.at(mCurrentIndex-1).label->move(0,  Renderer::getScreenHeight()/4, 1000);

		mConsoleVector.at(mCurrentIndex).label->move(0, - (Renderer::getScreenHeight()/4), 25);

		return true;
	}

	return false;
}

bool GuiConsoleList::goToPrev()
{
	if(mCurrentIndex-1 >= 0)
	{
		//avoid animation corruption
		if(mAnimator->isAnimating()) return false;

		mCurrentIndex--;
		mAnimator->move(Renderer::getScreenWidth() / 2, 0, 50);

		if(mCurrentIndex+1 < mConsoleVector.size())
			mConsoleVector.at(mCurrentIndex+1).label->move(0,  (Renderer::getScreenHeight()/4), 1000);

		SDL_Delay(1000);
		mConsoleVector.at(mCurrentIndex).label->move(0, - (Renderer::getScreenHeight()/4), 25);

		return true;
	}

	return false;
}

bool GuiConsoleList::setCurrentIndex(int index)
{
	if(mCurrentIndex != index && index >= 0 && index < mConsoleVector.size())
	{
		mCurrentIndex = index;
		this->setOffset(-(Renderer::getScreenWidth() / 2 * index), this->getOffset().y);
		

		for(unsigned int i = 0; i < mConsoleVector.size(); i++)
		{
			mConsoleVector.at(i).text->setOffset(Renderer::getScreenWidth()/2 + Renderer::getScreenWidth()/2 * i - mConsoleVector.at(i).text->getSize().x/2, Renderer::getScreenHeight());
		}
		mConsoleVector.at(index).label->move(0, - (Renderer::getScreenHeight()/4), 25);

		return true;
	}

	return false;
}