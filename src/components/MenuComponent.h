#pragma once

#include "NinePatchComponent.h"
#include "ComponentList.h"
#include "TextComponent.h"
#include "ComponentGrid.h"

class ButtonComponent;

class MenuComponent : public GuiComponent
{
public:
	MenuComponent(Window* window, const char* title);

	void onSizeChanged() override;

	inline void addRow(const ComponentListRow& row, bool setCursorHere = false) { mList->addRow(row, setCursorHere); }

	inline void addWithLabel(const std::string& label, const std::shared_ptr<GuiComponent>& comp, bool setCursorHere = false)
	{
		ComponentListRow row;
		row.addElement(std::make_shared<TextComponent>(mWindow, label, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.addElement(comp, false);
		addRow(row, setCursorHere);
	}

	void addButton(const std::string& label, const std::string& helpText, const std::function<void()>& callback);

	inline void setCursorToList() { mGrid.setCursorTo(mList); }
	inline void setCursorToButtons() { assert(mButtonGrid); mGrid.setCursorTo(mButtonGrid); }

private:
	void updateGrid();

	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<ComponentList> mList;
	std::shared_ptr<ComponentGrid> mButtonGrid;
	std::vector< std::shared_ptr<ButtonComponent> > mButtons;
};
