#pragma once

#include "../GuiComponent.h"
#include "../resources/Font.h"
#include <vector>
#include <functional>
#include "../Renderer.h"
#include "NinePatchComponent.h"

//Used to display a list of options.
//Can select one or multiple options.

template<typename T>
class OptionListComponent : public GuiComponent
{
public:
	OptionListComponent(Window* window, bool multiSelect = false) : GuiComponent(window),
		mCursor(0), mScrollOffset(0), mMultiSelect(multiSelect), mEditing(false), mBox(window, ":/textbox.png")
	{
	}

	struct ListEntry
	{
		std::string text;
		unsigned int color;
		bool selected;
		T object;
	};


	bool input(InputConfig* config, Input input)
	{
		if(input.value != 0)
		{
			if(config->isMappedTo("b", input))
			{
				close();
				return true;
			}
			if(config->isMappedTo("a", input))
			{
				if(mEditing)
				{
					select(mCursor);
					if(!mMultiSelect)
						close();
				}else{
					open();
				}
				
				return true;
			}
			if(mEditing && mEntries.size() > 1)
			{
				if(config->isMappedTo("up", input))
				{
					if(mCursor > 0)
						mCursor--;
					
					return true;
				}
				if(config->isMappedTo("down", input))
				{
					if(mCursor < mEntries.size() - 1)
						mCursor++;

					return true;
				}
			}
		}

		return GuiComponent::input(config, input);
	}

	void render(const Eigen::Affine3f& parentTrans)
	{
		std::shared_ptr<Font> font = getFont();
		
		//draw the option list
		if(mEditing)
		{
			Eigen::Affine3f trans = parentTrans * getTransform();

			unsigned int renderCount = mTextCaches.size() - mScrollOffset;

			float height = (float)renderCount * font->getHeight();
			trans.translate(Eigen::Vector3f(0, -height / 2 + font->getHeight() * 0.5f, 0));

			mBox.fitTo(Eigen::Vector2f(mSize.x(), height));
			mBox.render(trans);

			Renderer::setMatrix(trans);
			Renderer::drawRect(0, 0, (int)getSize().x(), (int)height, 0xFFFFFFFF);

			for(unsigned int i = mScrollOffset; i < renderCount; i++)
			{
				Renderer::setMatrix(trans);

				char rectOpacity = 0x00;
				if(i == mCursor)
					rectOpacity += 0x22;
				if(mEntries.at(i).selected)
					rectOpacity += 0x44;

				Renderer::drawRect(0, 0, (int)mSize.x(), font->getHeight(), 0x00000000 | rectOpacity);

				Renderer::setMatrix(trans);
				font->renderTextCache(mTextCaches.at(i));

				trans = trans.translate(Eigen::Vector3f(0, (float)font->getHeight(), 0));
			}
		}else{
			Renderer::setMatrix(parentTrans * getTransform());

			unsigned int color = 0x000000FF;

			if(mMultiSelect)
			{
				//draw "# selected"
				unsigned int selectedCount = 0;
				for(auto it = mEntries.begin(); it != mEntries.end(); it++)
				{
					if(it->selected)
						selectedCount++;
				}

				std::stringstream ss;
				ss << selectedCount << " selected";
				font->drawText(ss.str(), Eigen::Vector2f(0, 0), color);

			}else{
				//draw selected option
				bool found = false;
				for(auto it = mEntries.begin(); it != mEntries.end(); it++)
				{
					if(it->selected)
					{
						font->drawText(it->text, Eigen::Vector2f(0, 0), color);
						found = true;
						break;
					}
				}

				if(!found)
					font->drawText("Not set", Eigen::Vector2f(0, 0), color);
			}
		}

		renderChildren(parentTrans * getTransform());
	}

	ListEntry makeEntry(const std::string& name, unsigned int color, T obj, bool selected = false) const
	{
		ListEntry e = {name, color, selected, obj};
		return e;
	}

	void populate(std::vector<T>& vec, std::function<ListEntry(const T&)> selector)
	{
		for(auto it = vec.begin(); it != vec.end(); it++)
		{
			ListEntry e = selector(*it);
			if(!e.text.empty())
				mEntries.push_back(e);
		}

		updateTextCaches();
	}

	void addEntry(ListEntry e)
	{
		mEntries.push_back(e);
		updateTextCaches();
	}

	std::vector<const ListEntry*> getSelected()
	{
		std::vector<const ListEntry*> ret;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
		{
			if((*it).selected)
				ret.push_back(&(*it));
		}

		return ret;
	}
private:
	void select(unsigned int i)
	{
		if(i >= mEntries.size())
			return;

		if(!mMultiSelect)
			for(auto it = mEntries.begin(); it != mEntries.end(); it++)
				it->selected = false;

		mEntries.at(i).selected = !mEntries.at(i).selected;
		updateTextCaches();
	}

	void close()
	{
		mEditing = false;
	}

	void open()
	{
		mEditing = true;
	}

	void updateTextCaches()
	{
		for(auto it = mTextCaches.begin(); it != mTextCaches.end(); it++)
		{
			delete *it;
		}
		mTextCaches.clear();

		TextCache* cache;
		std::shared_ptr<Font> font = getFont();
		Eigen::Vector2f newSize = getSize();
		newSize[1] = (float)font->getHeight();
		for(unsigned int i = 0; i < mEntries.size(); i++)
		{
			cache = font->buildTextCache(mEntries.at(i).text, 0, 0, mEntries.at(i).color);
			mTextCaches.push_back(cache);

			if(cache->metrics.size.x() > newSize.x())
				newSize[0] = cache->metrics.size.x();
		}

		setSize(newSize);
	}

	std::shared_ptr<Font> getFont()
	{
		return Font::get(FONT_SIZE_MEDIUM);
	}


	unsigned int mCursor;
	unsigned int mScrollOffset;
	bool mMultiSelect;
	bool mEditing;

	NinePatchComponent mBox;

	std::vector<ListEntry> mEntries;
	std::vector<TextCache*> mTextCaches;
};
