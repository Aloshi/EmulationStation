#include "SystemView.h"
#include "../SystemData.h"
#include "../Renderer.h"
#include "../Log.h"

SystemView::SystemView(Window* window, SystemData* system) : GuiComponent(window), 
	mSystem(system),

	mHeaderImage(window),
	mHeaderText(window),
	mImage(window)
{
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);
	mHeaderImage.setResize(0, mSize.y() * 0.2f, true);

	mHeaderText.setPosition(0, 6);
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setCentered(true);

	mImage.setOrigin(0.5f, 0.5f);
	mImage.setPosition(mSize.x() / 2, mSize.y() * 0.6f);
	mImage.setResize(0, mSize.y() * 0.8f, false);

	addChild(&mImage);
	addChild(&mHeaderText);
	addChild(&mHeaderImage);
	
	updateData();
}

void SystemView::updateData()
{
	// header
	if(mSystem->getTheme()->getImage("headerImage").path.empty())
	{
		// use text
		mHeaderImage.setImage("");
		mHeaderText.setText(mSystem->getFullName());
	}else{
		// use image
		mHeaderText.setText("");
		mHeaderImage.setImage(mSystem->getTheme()->getImage("headerImage").getTexture());
	}

	mImage.setImage(mSystem->getTheme()->getImage("systemImage").getTexture());
}
