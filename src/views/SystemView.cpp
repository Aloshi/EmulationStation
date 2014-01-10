#include "SystemView.h"
#include "../SystemData.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"
#include "ViewController.h"

SystemView::SystemView(Window* window, SystemData* system) : GuiComponent(window), 
	mSystem(system),

	mHeaderImage(window),
	mHeaderText(window),
	mImage(window),
	mExtras(window)
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);
	mHeaderImage.setResize(0, mSize.y() * 0.2f);

	mHeaderText.setPosition(0, 6);
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setCentered(true);

	mImage.setOrigin(0.5f, 0.5f);
	mImage.setPosition(mSize.x() / 2, mSize.y() * 0.6f);
	mImage.setResize(0, mSize.y() * 0.8f);

	addChild(&mExtras);
	addChild(&mImage);
	addChild(&mHeaderText);
	addChild(&mHeaderImage);
	
	updateData();
}

void SystemView::updateData()
{
	using namespace ThemeFlags;

	mExtras.setExtras(ThemeData::makeExtras(mSystem->getTheme(), "system", mWindow));

	mHeaderImage.setImage("");
	mHeaderImage.applyTheme(mSystem->getTheme(), "system", "header", ALL);

	// header
	if(mHeaderImage.hasImage())
	{
		// use image
		mHeaderText.setText("");
	}else{
		// use text
		mHeaderImage.setImage("");
		mHeaderText.setText(mSystem->getFullName());
	}

	mImage.applyTheme(mSystem->getTheme(), "system", "system", ALL);
}

bool SystemView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("left", input))
		{
			mWindow->getViewController()->goToSystemView(mSystem->getPrev());
			return true;
		}
		if(config->isMappedTo("right", input))
		{
			mWindow->getViewController()->goToSystemView(mSystem->getNext());
			return true;
		}
		if(config->isMappedTo("a", input))
		{
			mWindow->getViewController()->goToGameList(mSystem);
			return true;
		}
	}

	return GuiComponent::input(config, input);
}
