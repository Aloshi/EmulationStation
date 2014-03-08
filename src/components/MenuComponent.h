#pragma once

#include "NinePatchComponent.h"
#include "ComponentList.h"
#include "TextComponent.h"

class MenuComponent : public GuiComponent
{
public:
	MenuComponent(Window* window, const char* title);

	void onSizeChanged() override;

	inline void addRow(const ComponentListRow& row, bool setCursorHere = false) { mList.addRow(row, setCursorHere); }

	inline void addWithLabel(const std::string& label, const std::shared_ptr<GuiComponent>& comp, bool setCursorHere = false)
	{
		ComponentListRow row;
		row.addElement(std::make_shared<TextComponent>(mWindow, label, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.addElement(comp, false);
		addRow(row, setCursorHere);
	}

private:
	NinePatchComponent mBackground;
	TextComponent mTitle;
	ComponentList mList;
};
