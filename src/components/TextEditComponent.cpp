#include "TextEditComponent.h"
#include "../Log.h"
#include "../resources/Font.h"
#include "../Window.h"
#include "../Renderer.h"

TextEditComponent::TextEditComponent(Window* window) : GuiComponent(window),
	mBox(window, ":/textbox.png"), mFocused(false), 
	mScrollOffset(0.0f, 0.0f), mCursor(0), mEditing(false)
{
	addChild(&mBox);
	
	onFocusLost();

	setSize(256, (float)getFont()->getHeight());
}

void TextEditComponent::onFocusGained()
{
	mBox.setImagePath(":/textbox_glow.png");
	mBox.setEdgeColor(0x51CCFF00 | getOpacity());
	
	SDL_StartTextInput();
	mFocused = true;
}

void TextEditComponent::onFocusLost()
{
	mBox.setImagePath(":/textbox.png");
	mBox.setEdgeColor(0xFFFFFF00 | getOpacity());
	
	SDL_StopTextInput();
	mFocused = false;
}

void TextEditComponent::onSizeChanged()
{
	mBox.fitTo(getSize());
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
		updateHelpPrompts();
		return true;
	}

	if(mEditing)
	{
		if(config->getDeviceId() == DEVICE_KEYBOARD && input.id == SDLK_RETURN)
		{
			if(isMultiline())
			{
				textInput("\n");
			}else{
				mEditing = false;
				updateHelpPrompts();
			}

			return true;
		}

		if((config->getDeviceId() == DEVICE_KEYBOARD && input.id == SDLK_ESCAPE) || (config->getDeviceId() != DEVICE_KEYBOARD && config->isMappedTo("b", input)))
		{
			mEditing = false;
			updateHelpPrompts();
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
			if(mCursor >= (int)mText.length())
				mCursor = mText.length();

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

	std::string wrappedText = (isMultiline() ? f->wrapText(mText, mSize.x()) : mText);
	mTextCache = std::unique_ptr<TextCache>(f->buildTextCache(wrappedText, 0, 0, 0x00000000 | getOpacity()));

	if(mCursor > (int)mText.length())
		mCursor = mText.length();
}

void TextEditComponent::onCursorChanged()
{
	std::shared_ptr<Font> font = getFont();

	if(isMultiline())
	{
		Eigen::Vector2f textSize = font->getWrappedTextCursorOffset(mText, mSize.x(), mCursor); 

		if(mScrollOffset.y() + mSize.y() < textSize.y() + font->getHeight()) //need to scroll down?
		{
			mScrollOffset[1] = textSize.y() - mSize.y() + font->getHeight();
		}else if(mScrollOffset.y() > textSize.y()) //need to scroll up?
		{
			mScrollOffset[1] = textSize.y();
		}
	}else{
		Eigen::Vector2f cursorPos = font->sizeText(mText.substr(0, mCursor));

		if(mScrollOffset.x() + mSize.x() < cursorPos.x())
		{
			mScrollOffset[0] = cursorPos.x() - mSize.x();
		}else if(mScrollOffset.x() > cursorPos.x())
		{
			mScrollOffset[0] = cursorPos.x();
		}
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

	trans.translate(Eigen::Vector3f(-mScrollOffset.x(), -mScrollOffset.y(), 0));

	Renderer::setMatrix(trans);

	std::shared_ptr<Font> f = getFont();
	if(mTextCache != NULL)
	{
		f->renderTextCache(mTextCache.get());
	}

	//draw cursor
	if(mEditing)
	{
		Eigen::Vector2f cursorPos;
		if(isMultiline())
		{
			cursorPos = f->getWrappedTextCursorOffset(mText, mSize.x(), mCursor);
		}else{
			cursorPos = f->sizeText(mText.substr(0, mCursor));
			cursorPos[1] = 0;
		}

		Renderer::drawRect((int)cursorPos.x(), (int)cursorPos.y(), 3, f->getHeight(), 0x000000FF);
	}

	Renderer::popClipRect();
}

std::shared_ptr<Font> TextEditComponent::getFont()
{
	return Font::get(FONT_SIZE_SMALL);
}

bool TextEditComponent::isMultiline()
{
	return (getSize().y() > (float)getFont()->getHeight());
}

bool TextEditComponent::isEditing() const
{
	return mEditing;
}

std::vector<HelpPrompt> TextEditComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	if(mEditing)
	{
		prompts.push_back(HelpPrompt("up/down/left/right", "move cursor"));
		prompts.push_back(HelpPrompt("b", "stop editing"));
	}else{
		prompts.push_back(HelpPrompt("a", "edit"));
	}
	return prompts;
}
