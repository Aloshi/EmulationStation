#include "GuiMsgBoxYesNo.h"
#include "../Renderer.h"

GuiMsgBoxYesNo::GuiMsgBoxYesNo(Window* window, const std::string& text, std::function<void()> yesCallback, std::function<void()> noCallback) : GuiComponent(window),
	mYesCallback(yesCallback), 
	mNoCallback(noCallback), 
	mBackground(window), 
	mText(window), 
	mInputText(window)
{
	const float paddingX = 32;
	const float paddingY = 16;

	const float maxWidth = Renderer::getScreenWidth() * 0.7f;

	float width = mText.getFont()->sizeText(text).x() + paddingX;
	if(width > maxWidth)
		width = maxWidth;

	mText.setCentered(true);
	mText.setColor(0x777777FF);
	mText.setPosition(paddingX / 2, paddingY / 2);
	mText.setSize(width - paddingX, 0);
	mText.setText(text);

	mInputText.setCentered(true);
	mInputText.setColor(0x0044BBFF);
	mInputText.setFont(Font::get(FONT_SIZE_SMALL));
	mInputText.setPosition(paddingX / 2, mText.getPosition().y() + mText.getSize().y());
	mInputText.setSize(width - paddingX, 0);
	mInputText.setText("[A - yes]         [B - no]");

	setSize(width, mInputText.getPosition().y() + mInputText.getSize().y() + paddingY/2);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);

	mBackground.setImagePath(":/frame.png");
	mBackground.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-32, -32));
}

bool GuiMsgBoxYesNo::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			if(mYesCallback)
				mYesCallback();

			delete this;
			return true;
		}else if(config->isMappedTo("b", input))
		{
			if(mNoCallback)
				mNoCallback();

			delete this;
			return true;
		}
	}

	return false;
}

void GuiMsgBoxYesNo::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	
	mBackground.render(trans);

	Renderer::setMatrix(trans);
	Renderer::drawRect(0, (int)(mText.getPosition().y() + mText.getSize().y()), (int)mSize.x(), 1, 0xC6C7C6FF);

	mText.render(trans);
	mInputText.render(trans);

	renderChildren(trans);
}
