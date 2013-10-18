#pragma once

#include "../GuiComponent.h"
#include "../resources/Font.h"
#include <vector>
#include <functional>
#include "../Renderer.h"
#include "../Window.h"
#include "NinePatchComponent.h"
#include "../Window.h"

//Used to display a list of options.
//Can select one or multiple options.

template<typename T>
class OptionListComponent : public GuiComponent
{
public:
	OptionListComponent(Window* window, bool multiSelect = false) : GuiComponent(window),
		mMultiSelect(multiSelect)
	{
		if(multiSelect)
			setSize(getFont()->sizeText("0 selected"));
		else
			setSize(getFont()->sizeText("Not set"));
	}

	struct ListEntry
	{
		std::string text;
		bool selected;
		T object;
	};


	bool input(InputConfig* config, Input input) override
	{
		if(input.value != 0)
		{
			if(config->isMappedTo("a", input))
			{
				open();
				return true;
			}
		}
		return GuiComponent::input(config, input);
	}

	void render(const Eigen::Affine3f& parentTrans)
	{
		std::shared_ptr<Font> font = getFont();
		
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

		renderChildren(parentTrans * getTransform());
	}

	ListEntry makeEntry(const std::string& name, T obj, bool selected = false) const
	{
		ListEntry e;
		e.text = name;
		e.object = obj;
		e.selected = selected;
		return e;
	}

	void populate(std::vector<T>& vec, std::function<ListEntry(const T&)> selector)
	{
		for(auto it = vec.begin(); it != vec.end(); it++)
		{
			ListEntry e = selector(*it);
			if(!e.text.empty())
				addEntry(e);
		}
	}

	void addEntry(ListEntry e)
	{
		mEntries.push_back(e);

		Eigen::Vector2f size = getFont()->sizeText(e.text);
		if(size.x() > mSize.x())
			setSize(size.x(), mSize.y());
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

	std::vector<T> getSelectedObjects()
	{
		std::vector<T> ret;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
		{
			if((*it).selected)
				ret.push_back(it->object);
		}

		return ret;
	}

private:
	void open()
	{
		mWindow->pushGui(new OptionListPopup(mWindow, *this));
	}

	void select(unsigned int i)
	{
		if(i >= mEntries.size())
			return;

		if(!mMultiSelect)
			for(auto it = mEntries.begin(); it != mEntries.end(); it++)
				it->selected = false;

		mEntries.at(i).selected = !mEntries.at(i).selected;
	}

	std::shared_ptr<Font> getFont()
	{
		return Font::get(FONT_SIZE_MEDIUM);
	}


	class OptionListPopup : public GuiComponent
	{
	public:
		OptionListPopup(Window* window, OptionListComponent<T>& optList) : GuiComponent(window), 
			mOptList(optList), mBox(window, ":/textbox.png"), mCursor(0), mScrollOffset(0)
		{
			//find global position
			GuiComponent* p = &mOptList;
			do {
				mPosition += p->getPosition();
			} while(p = p->getParent());

			mSize = mOptList.getSize();
			updateTextCaches();
		}

		void render(const Eigen::Affine3f& parentTrans) override
		{
			Eigen::Affine3f trans = parentTrans * getTransform();

			std::shared_ptr<Font> font = mOptList.getFont();

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
				if(mOptList.mEntries.at(i).selected)
					rectOpacity += 0x44;

				Renderer::drawRect(0, 0, (int)mSize.x(), font->getHeight(), 0x00000000 | rectOpacity);

				Renderer::setMatrix(trans);
				font->renderTextCache(mTextCaches.at(i));

				trans = trans.translate(Eigen::Vector3f(0, (float)font->getHeight(), 0));
			}
		}

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
					mOptList.select(mCursor);
					if(!mOptList.mMultiSelect)
						close();
				
					return true;
				}
				if(mOptList.mEntries.size() > 1)
				{
					if(config->isMappedTo("up", input))
					{
						if(mCursor > 0)
							mCursor--;
					
						return true;
					}
					if(config->isMappedTo("down", input))
					{
						if(mCursor < mOptList.mEntries.size() - 1)
							mCursor++;

						return true;
					}
				}
			}

			return GuiComponent::input(config, input);
		}

	private:
		void close()
		{
			delete this;
		}

		void updateTextCaches()
		{
			for(auto it = mTextCaches.begin(); it != mTextCaches.end(); it++)
			{
				delete *it;
			}
			mTextCaches.clear();

			TextCache* cache;
			std::shared_ptr<Font> font = mOptList.getFont();
			for(unsigned int i = 0; i < mOptList.mEntries.size(); i++)
			{
				cache = font->buildTextCache(mOptList.mEntries.at(i).text, 0, 0, 0x000000FF);
				mTextCaches.push_back(cache);
			}
		}

		OptionListComponent<T>& mOptList;
		NinePatchComponent mBox;

		unsigned int mCursor;
		unsigned int mScrollOffset;
		std::vector<TextCache*> mTextCaches;
	};

	bool mMultiSelect;
	
	std::vector<ListEntry> mEntries;
};
