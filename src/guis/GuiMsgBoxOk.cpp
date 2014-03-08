#include "GuiMsgBoxOk.h"
#include "../Renderer.h"

#define MSG_WIDTH 0.8f
#define MSG_PADDING ((1 - MSG_WIDTH) / 2)

GuiMsgBoxOk::GuiMsgBoxOk(Window* window, const std::string& text, std::function<void()> callback) : GuiComponent(window),
	mCallback(callback),
	mBackground(window), 
	mText(window),
	mOkText(window)
{
	mText.setCentered(true);
	mText.setColor(0x00BB00FF);
	mText.setSize(Renderer::getScreenWidth() * MSG_WIDTH, 0);
	mText.setText(text);

	mOkText.setCentered(true);
	mOkText.setColor(0x0044BBFF);
	mOkText.setFont(Font::get(FONT_SIZE_SMALL));
	mOkText.setSize(Renderer::getScreenWidth() * MSG_WIDTH, 0);
	mOkText.setText("[A]");

	mText.setPosition(Renderer::getScreenWidth() * MSG_PADDING, (Renderer::getScreenHeight() - mText.getSize().y() - mOkText.getSize().y()) / 2);
	mOkText.setPosition(Renderer::getScreenWidth() * MSG_PADDING, mText.getPosition().y() + mText.getSize().y());
}

bool GuiMsgBoxOk::input(InputConfig* config, Input input)
{
	if(input.value != 0 && 
		(config->isMappedTo("a", input) || config->isMappedTo("b", input)))
	{
		if(mCallback)
			mCallback();

		delete this;
		return true;
	}

	return false;
}

void GuiMsgBoxOk::render(const Eigen::Affine3f& parentTrans)
{
	float height = mText.getSize().y() + mOkText.getSize().y();
	Renderer::setMatrix(parentTrans);
	Renderer::drawRect(0, (int)((Renderer::getScreenHeight() - height) / 2), Renderer::getScreenWidth(), (int)height, 0x111111FF);
	mText.render(parentTrans);
	mOkText.render(parentTrans);
}
