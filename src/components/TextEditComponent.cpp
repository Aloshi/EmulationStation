#include "TextEditComponent.h"
#include "../Log.h"
#include "../Font.h"
#include "../Window.h"
#include "../Renderer.h"
#include "ComponentListComponent.h"

TextEditComponent::TextEditComponent(Window* window) : GuiComponent(window),
	mBox(window, 0, 0, 0, 0), mFocused(false), 
	mScrollOffset(0.0f), mCursor(0), mEditing(false)
{
	addChild(&mBox);
	
	onFocusLost();

	setSize(256, (float)getFont()->getHeight());
}

void TextEditComponent::onFocusGained()
{
	mBox.setHorizontalImage(":/glow_hor.png");
	mBox.setVerticalImage(":/glow_vert.png");
	mBox.setBorderColor(0x51CCFF00 | getOpacity());

	SDL_StartTextInput();
	mFocused = true;
}

void TextEditComponent::onFocusLost()
{
	mBox.setHorizontalImage(":/glow_off_hor.png");
	mBox.setVerticalImage(":/glow_off_vert.png");
	mBox.setBorderColor(0xFFFFFF00 | getOpacity());

	SDL_StopTextInput();
	mFocused = false;
}

void TextEditComponent::onSizeChanged()
{
	mBox.setSize(mSize);
}

void TextEditComponent::setValue(const std::string& val)
{
	mText = val;
	onTextChanged();
}

std::string TextEditComponent::getValue() const
{
	return mText;
}

void TextEditComponent::textInput(const char* text)
{
	if(mEditing)
	{
		if(text[0] == '\b')
		{
			if(mCursor > 0)
			{
				mText.erase(mText.begin() + mCursor - 1, mText.begin() + mCursor);
				mCursor--;
			}
		}else{
			mText.insert(mCursor, text);
			mCursor++;
		}
	}

	onTextChanged();
	onCursorChanged();
}

bool TextEditComponent::input(InputConfig* config, Input input)
{
	if(input.value == 0)
		return false;

	if(config->isMappedTo("a", input) && mFocused && !mEditing)
	{
		mEditing = true;
		return true;
	}

	if(mEditing)
	{
		if(config->getDeviceId() == DEVICE_KEYBOARD && input.id == SDLK_RETURN)
		{
			textInput("\n");
			return true;
		}

		if(config->isMappedTo("b", input))
		{
			mEditing = false;
			return true;
		}

		if(config->isMappedTo("up", input))
		{

		}else if(config->isMappedTo("down", input))
		{

		}else if(config->isMappedTo("left", input))
		{
			mCursor--;
			if(mCursor < 0)
				mCursor = 0;

			onCursorChanged();
		}else if(config->isMappedTo("right", input))
		{
			mCursor++;
			if(mText.length() == 0)
				mCursor = 0;
			if(mCursor > (int)mText.length() - 1)
				mCursor = mText.length() - 1;

			onCursorChanged();
		}

		//consume all input when editing text
		return true;
	}

	return false;
}

void TextEditComponent::onTextChanged()
{
	std::shared_ptr<Font> f = getFont();

	std::string wrappedText = f->wrapText(mText, mSize.x());
	mTextCache = std::unique_ptr<TextCache>(f->buildTextCache(wrappedText, 0, 0, 0x00000000 | getOpacity()));
}

void TextEditComponent::onCursorChanged()
{
	std::shared_ptr<Font> font = getFont();
	Eigen::Vector2f textSize = font->getWrappedTextCursorOffset(mText, mSize.x(), mCursor); //font->sizeWrappedText(mText.substr(0, mCursor), mSize.x());

	if(mScrollOffset + mSize.y() < textSize.y() + font->getHeight()) //need to scroll down?
	{
		mScrollOffset = textSize.y() - mSize.y() + font->getHeight();
	}else if(mScrollOffset > textSize.y()) //need to scroll up?
	{
		mScrollOffset = textSize.y();
	}
}

void TextEditComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = getTransform() * parentTrans;
	renderChildren(trans);

	Eigen::Vector2i clipPos((int)trans.translation().x(), (int)trans.translation().y());
	Eigen::Vector3f dimScaled = trans * Eigen::Vector3f(mSize.x(), mSize.y(), 0);
	Eigen::Vector2i clipDim((int)dimScaled.x() - trans.translation().x(), (int)dimScaled.y() - trans.translation().y());
	Renderer::pushClipRect(clipPos, clipDim);

	trans.translate(Eigen::Vector3f(0, -mScrollOffset, 0));

	Renderer::setMatrix(trans);

	std::shared_ptr<Font> f = getFont();
	if(mTextCache != NULL)
	{
		f->renderTextCache(mTextCache.get());
	}

	//draw cursor
	if(mEditing)
	{
		Eigen::Vector2f cursorPos = f->getWrappedTextCursorOffset(mText, mSize.x(), mCursor);
		Renderer::drawRect(cursorPos.x(), cursorPos.y(), 3, f->getHeight(), 0x000000FF);
	}

	Renderer::popClipRect();
}

std::shared_ptr<Font> TextEditComponent::getFont()
{
	return Font::get(*mWindow->getResourceManager(), Font::getDefaultPath(), FONT_SIZE_SMALL);
}
