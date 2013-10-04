#pragma once

#include "../GuiComponent.h"
#include "../resources/Font.h"
#include <vector>
#include <functional>
#include "../Renderer.h"
#include "../Window.h"

//Used to display a list of options.
//Can select one or multiple options.

template<typename T>
class OptionListComponent : public GuiComponent
{
public:
	OptionListComponent(Window* window) : GuiComponent(window),
		mClosedCallback(nullptr), mCursor(0), mScrollOffset(0)
	{
	}

	struct ListEntry
	{
		std::string text;
		unsigned int color;
		bool selected;
		T object;
	};


	void setClosedCallback(std::function<void(std::vector<const ListEntry*>)> callback)
	{
		mClosedCallback = callback;
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
				select(mCursor);
				return true;
			}
			if(mEntries.size() > 1)
			{
				if(config->isMappedTo("up", input))
				{
					if(mCursor > 0)
					{
						mCursor--;
						return true;
					}
				}
				if(config->isMappedTo("down", input))
				{
					if(mCursor < mEntries.size() - 1)
					{
						mCursor++;
						return true;
					}
				}
			}
		}

		return GuiComponent::input(config, input);
	}

	void render(const Eigen::Affine3f& parentTrans)
	{
		Eigen::Affine3f trans = parentTrans * getTransform();

		std::shared_ptr<Font> font = getFont();
		
		Renderer::pushClipRect(Eigen::Vector2i((int)trans.translation().x(), (int)trans.translation().y()), 
			Eigen::Vector2i((int)getSize().x(), (int)getSize().y()));

		for(unsigned int i = mScrollOffset; i < mTextCaches.size(); i++)
		{
			Renderer::setMatrix(trans);

			if(i == mCursor)
				Renderer::drawRect(0, 0, (int)mSize.x(), font->getHeight(), 0x000000FF);

			font->renderTextCache(mTextCaches.at(i));
			trans = trans.translate(Eigen::Vector3f(0, (float)font->getHeight(), 0));
		}

		Renderer::popClipRect();

		trans = parentTrans * getTransform();
		renderChildren(trans);
	}

	ListEntry makeEntry(const std::string& name, unsigned int color, T obj) const
	{
		ListEntry e = {name, color, false, obj};
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

		mEntries.at(i).selected = !mEntries.at(i).selected;
		updateTextCaches();
	}

	void close()
	{
		if(mClosedCallback)
			mClosedCallback(getSelected());
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
		newSize[1] = 0;
		for(unsigned int i = 0; i < mEntries.size(); i++)
		{
			cache = font->buildTextCache(mEntries.at(i).text, 0, 0, mEntries.at(i).color);
			mTextCaches.push_back(cache);

			if(cache->metrics.size.x() > newSize.x())
				newSize[0] = cache->metrics.size.x();

			newSize[1] += cache->metrics.size.y();
		}
		setSize(newSize);
	}

	std::shared_ptr<Font> getFont()
	{
		return Font::get(FONT_SIZE_SMALL);
	}


	std::function<void(std::vector<const ListEntry*>)> mClosedCallback;
	unsigned int mCursor;
	unsigned int mScrollOffset;

	std::vector<ListEntry> mEntries;
	std::vector<TextCache*> mTextCaches;
};

