#include "MenuComponent.h"

MenuComponent::MenuComponent(Window* window, const char* title) : GuiComponent(window), 
	mBackground(window), mTitle(window), mList(window)
{
	mBackground.setImagePath(":/frame.png");

	mTitle.setFont(Font::get(FONT_SIZE_LARGE));
	mTitle.setText(title);
	mTitle.setColor(0x555555FF);
	mTitle.setCentered(true);

	addChild(&mBackground);
	addChild(&mTitle);
	addChild(&mList);

	setSize(Renderer::getScreenWidth() * 0.5f, Renderer::getScreenHeight() * 0.75f);
}

void MenuComponent::onSizeChanged()
{
	mBackground.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-32, -32));

	const float titlePadding = mTitle.getFont()->getHeight() * 0.2f;

	mTitle.setSize(mSize.x(), (float)mTitle.getFont()->getHeight());
	mTitle.setPosition(0, titlePadding / 2);

	mList.setPosition(0, mTitle.getSize().y() + titlePadding);
	mList.setSize(mSize.x(), mSize.y() - mTitle.getSize().y() - titlePadding);
}
