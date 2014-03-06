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

private:
	NinePatchComponent mBackground;
	TextComponent mTitle;
	ComponentList mList;
};
