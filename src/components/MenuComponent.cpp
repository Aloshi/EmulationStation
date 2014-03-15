#include "MenuComponent.h"
#include "ButtonComponent.h"

#define BUTTON_GRID_HEIGHT ((float)Font::get(FONT_SIZE_MEDIUM)->getHeight() + 32)

using namespace Eigen;

MenuComponent::MenuComponent(Window* window, const char* title) : GuiComponent(window), 
	mBackground(window), mGrid(window, Vector2i(1, 3))
{	
	addChild(&mBackground);
	addChild(&mGrid);

	mBackground.setImagePath(":/frame.png");

	// set up title which will never change
	mTitle = std::make_shared<TextComponent>(mWindow, strToUpper(title), Font::get(FONT_SIZE_LARGE), 0x555555FF, true);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false);

	// set up list which will never change (externally, anyway)
	mList = std::make_shared<ComponentList>(mWindow);
	mGrid.setEntry(mList, Vector2i(0, 1), true);

	updateGrid();
	updateSize();

	mGrid.resetCursor();
}

void MenuComponent::updateSize()
{
	float height = mTitle->getSize().y() + mList->getTotalRowHeight() + BUTTON_GRID_HEIGHT + 2;
	if(height > Renderer::getScreenHeight() * 0.7f)
		height = Renderer::getScreenHeight() * 0.7f;

	setSize(Renderer::getScreenWidth() * 0.5f, height);
}

void MenuComponent::onSizeChanged()
{
	mBackground.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-32, -32));

	// update grid row/col sizes
	mGrid.setRowHeightPerc(0, mTitle->getSize().y() / mSize.y());
	mGrid.setRowHeightPerc(2, (BUTTON_GRID_HEIGHT) / mSize.y());
	
	mGrid.setSize(mSize);
}

void MenuComponent::addButton(const std::string& name, const std::string& helpText, const std::function<void()>& callback)
{
	mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, strToUpper(name), helpText, callback));
	updateGrid();
	updateSize();
}

void MenuComponent::updateGrid()
{
	if(mButtonGrid)
		mGrid.removeEntry(mButtonGrid);

	if(mButtons.size())
	{
		mButtonGrid = makeButtonGrid(mWindow, mButtons);
		
		mGrid.setEntry(mButtonGrid, Vector2i(0, 2), true, false);
	}else{
		mButtonGrid.reset();
	}
}

std::vector<HelpPrompt> MenuComponent::getHelpPrompts()
{
	return mGrid.getHelpPrompts();
}

std::shared_ptr<ComponentGrid> makeButtonGrid(Window* window, const std::vector< std::shared_ptr<ButtonComponent> >& buttons)
{
	std::shared_ptr<ComponentGrid> buttonGrid = std::make_shared<ComponentGrid>(window, Vector2i(buttons.size(), 1));

	float buttonGridWidth = 16.0f * buttons.size(); // initialize to padding
	for(int i = 0; i < (int)buttons.size(); i++)
	{
		buttonGrid->setEntry(buttons.at(i), Vector2i(i, 0), true, false);
		buttonGridWidth += buttons.at(i)->getSize().x();
	}
	for(unsigned int i = 0; i < buttons.size(); i++)
	{
		buttonGrid->setColWidthPerc(i, (buttons.at(i)->getSize().x() + 16) / buttonGridWidth);
	}
		
	buttonGrid->setSize(buttonGridWidth, buttons.at(0)->getSize().y());

	return buttonGrid;
}
