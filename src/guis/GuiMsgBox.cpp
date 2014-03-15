#include "GuiMsgBox.h"
#include "../Renderer.h"
#include "../components/TextComponent.h"
#include "../components/ButtonComponent.h"
#include "../components/MenuComponent.h" // for makeButtonGrid

#define BUTTON_VERT_PADDING 32.0f

GuiMsgBox::GuiMsgBox(Window* window, const std::string& text, 
	const std::string& name1, const std::function<void()>& func1,
	const std::string& name2, const std::function<void()>& func2, 
	const std::string& name3, const std::function<void()>& func3) : GuiComponent(window), 
	mBackground(window, ":/frame.png"), mGrid(window, Eigen::Vector2i(1, 2))
{
	float width = Renderer::getScreenWidth() * 0.6f; // max width
	float minWidth = Renderer::getScreenWidth() * 0.3f; // minimum width

	mMsg = std::make_shared<TextComponent>(mWindow, text, Font::get(FONT_SIZE_MEDIUM), 0x777777FF, true);
	
	// create the buttons
	mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, name1, name1, std::bind(&GuiMsgBox::deleteMeAndCall, this, func1)));
	if(!name2.empty())
		mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, name2, name3, std::bind(&GuiMsgBox::deleteMeAndCall, this, func2)));
	if(!name3.empty())
		mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, name3, name3, std::bind(&GuiMsgBox::deleteMeAndCall, this, func3)));

	// put the buttons into a ComponentGrid
	mButtonGrid = makeButtonGrid(mWindow, mButtons);
	
	mGrid.setEntry(mMsg, Eigen::Vector2i(0, 0), false, true);
	mGrid.setEntry(mButtonGrid, Eigen::Vector2i(0, 1), true, false, Eigen::Vector2i(1, 1), GridFlags::BORDER_TOP);

	if(mMsg->getSize().x() > width)
	{
		mMsg->setSize(width, 0);
	}else{
		// mMsg is narrower than width
		// are buttons?
		if(mButtonGrid->getSize().x() < width)
		{
			width = std::max(mButtonGrid->getSize().x(), mMsg->getSize().x());
			width = std::max(width, minWidth);
		}
	}

	setSize(width, mMsg->getSize().y() + mButtonGrid->getSize().y() + BUTTON_VERT_PADDING);

	// center for good measure
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2.0f, (Renderer::getScreenHeight() - mSize.y()) / 2.0f);

	addChild(&mBackground);
	addChild(&mGrid);
}

void GuiMsgBox::onSizeChanged()
{
	mGrid.setSize(mSize);
	mBackground.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-32, -32));

	mGrid.setRowHeightPerc(1, (mButtonGrid->getSize().y() + BUTTON_VERT_PADDING) / mSize.y());
}

void GuiMsgBox::deleteMeAndCall(const std::function<void()>& func)
{
	if(func)
		func();

	delete this;
}
