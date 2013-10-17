#include "GuiMsgBoxYesNo.h"
#include "../Renderer.h"

#define MSG_WIDTH 0.8f
#define MSG_PADDING ((1 - MSG_WIDTH) / 2)

GuiMsgBoxYesNo::GuiMsgBoxYesNo(Window* window, const std::string& text, std::function<void()> yesCallback, std::function<void()> noCallback) : GuiComponent(window),
	mYesCallback(yesCallback), 
	mNoCallback(noCallback), 
	mText(window), 
	mInputText(window)
{
	mText.setCentered(true);
	mText.setColor(0x00BB00FF);
	mText.setSize(Renderer::getScreenWidth() * MSG_WIDTH, 0);
	mText.setText(text);

	mInputText.setCentered(true);
	mInputText.setColor(0x0044BBFF);
	mInputText.setFont(Font::get(FONT_SIZE_SMALL));
	mInputText.setSize(Renderer::getScreenWidth() * MSG_WIDTH, 0);
	mInputText.setText("[A - yes]         [B - no]");

	mText.setPosition(Renderer::getScreenWidth() * MSG_PADDING, (Renderer::getScreenHeight() - mText.getSize().y() - mInputText.getSize().y()) / 2);
	mInputText.setPosition(Renderer::getScreenWidth() * MSG_PADDING, mText.getPosition().y() + mText.getSize().y());
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
	float height = mText.getSize().y() + mInputText.getSize().y();
	Renderer::setMatrix(parentTrans);
	Renderer::drawRect(0, (int)((Renderer::getScreenHeight() - height) / 2), Renderer::getScreenWidth(), (int)height, 0x111111FF);
	mText.render(parentTrans);
	mInputText.render(parentTrans);
}
