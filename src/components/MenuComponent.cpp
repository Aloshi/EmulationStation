#include "MenuComponent.h"

MenuComponent::MenuComponent(Window* window, const char* title) : GuiComponent(window), 
	mBackground(window), mTitle(window), mList(window)
{
	mBackground.setImagePath(":/frame.png");

	mTitle.setFont(Font::get(FONT_SIZE_LARGE));
	mTitle.setText(title);
	mTitle.setCentered(true);

	addChild(&mBackground);
	addChild(&mTitle);
	addChild(&mList);

	setSize(Renderer::getScreenWidth() * 0.6f, Renderer::getScreenHeight() * 0.8f);
}

void MenuComponent::onSizeChanged()
{
	mBackground.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-2, -2));

	mTitle.setSize(mSize.x(), (float)mTitle.getFont()->getHeight());

	mList.setPosition(0, mTitle.getSize().y());
	mList.setSize(mSize.x(), mSize.y() - mTitle.getSize().y());
}
