#pragma once

#include "../GuiComponent.h"
#include "../resources/Font.h"
#include "../Renderer.h"
#include "../Window.h"
#include "TextComponent.h"
#include "MenuComponent.h"
#include <sstream>

//Used to display a list of options.
//Can select one or multiple options.

// if !multiSelect
// * <- curEntry ->

// always
// * press a -> open full list

template<typename T>
class OptionListComponent : public GuiComponent
{
private:
	struct OptionListData
	{
		std::string name;
		T object;
		bool selected;
	};

	class OptionListPopup : public GuiComponent
	{
	private:
		MenuComponent mMenu;
		OptionListComponent<T>* mParent;

	public:
		OptionListPopup(Window* window, OptionListComponent<T>* parent) : GuiComponent(window),
			mMenu(window, "optionlist"), mParent(parent)
		{
			auto font = Font::get(FONT_SIZE_MEDIUM);
			ComponentListRow row;
			for(auto it = mParent->mEntries.begin(); it != mParent->mEntries.end(); it++)
			{
				row.elements.clear();
				row.addElement(std::make_shared<TextComponent>(mWindow, it->name, font, 0x777777FF), true);

				mMenu.addRow(row);
			}

			mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);
			addChild(&mMenu);
		}

		bool input(InputConfig* config, Input input) override
		{
			if(config->isMappedTo("b", input) && input.value != 0)
			{
				delete this;
				return true;
			}

			return GuiComponent::input(config, input);
		}
		
		virtual ~OptionListPopup()
		{
			// commit changes
			
		}
	};

public:
	OptionListComponent(Window* window, bool multiSelect = false) : GuiComponent(window), mText(window), mMultiSelect(multiSelect)
	{
		auto font = Font::get(FONT_SIZE_MEDIUM);
		mText.setFont(font);
		mText.setColor(0x777777FF);
		mText.setCentered(true);
		addChild(&mText);

		setSize(Renderer::getScreenWidth() * 0.2f, (float)font->getHeight());
	}

	void onSizeChanged() override
	{
		mText.setSize(mSize);
	}

	bool input(InputConfig* config, Input input) override
	{
		if(input.value != 0)
		{
			if(config->isMappedTo("a", input))
			{
				open();
				return true;
			}
			if(!mMultiSelect)
			{
				if(config->isMappedTo("left", input))
				{
					// move selection to previous
				}else if(config->isMappedTo("right", input))
				{
					// move selection to next
				}
			}
		}
		return GuiComponent::input(config, input);
	}

	std::vector<T> getSelectedObjects()
	{
		std::vector<T> ret;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
		{
			if(it->selected)
				ret.push_back(it->object);
		}

		return ret;
	}

	T getSelected()
	{
		assert(mMultiSelect == false);
		auto selected = getSelectedObjects();
		assert(selected.size() == 1);
		return selected.at(0);
	}

	void add(const std::string& name, const T& obj, bool selected)
	{
		OptionListData e;
		e.name = name;
		e.object = obj;
		e.selected = selected;

		mEntries.push_back(e);
		onSelectedChanged();
	}

private:
	void open()
	{
		mWindow->pushGui(new OptionListPopup(mWindow, this));
	}

	void onSelectedChanged()
	{
		if(mMultiSelect)
		{
			// display # selected
			std::stringstream ss;
			ss << getSelectedObjects().size() << " selected";
			mText.setText(ss.str());
		}else{
			// display currently selected + l/r cursors
			for(auto it = mEntries.begin(); it != mEntries.end(); it++)
			{
				if(it->selected)
				{
					mText.setText(it->name);
					break;
				}
			}
		}
	}

	bool mMultiSelect;

	TextComponent mText;

	std::vector<OptionListData> mEntries;
};

