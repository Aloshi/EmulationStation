#ifndef _GUILIST_H_
#define _GUILIST_H_

#include "../Renderer.h"
#include "../GuiComponent.h"
#include "../InputManager.h"
#include <vector>
#include <string>

//A graphical list. Supports multiple colors for rows and scrolling.
//TODO - add truncation to text rendering if name exceeds a maximum width (a trailing elipses, perhaps).
template <typename listType>
class GuiList : public GuiComponent
{
public:
	GuiList(int offsetX = 0, int offsetY = 0, Renderer::FontSize fontsize = Renderer::MEDIUM);
	~GuiList();

	void onRender();
	void onTick(int deltaTime);
	void onInput(InputManager::InputButton button, bool keyDown);

	void addObject(std::string name, listType obj, int color = 0xFF0000);
	void clear();

	void onPause();
	void onResume();

	std::string getSelectedName();
	listType getSelectedObject();
	int getSelection();
	bool isScrolling();

	void setSelectorColor(int selectorColor);
	void setSelectedTextColor(int selectedColor);
	void setCentered(bool centered);

	void setTextOffsetX(int textoffsetx);

private:
	static const int SCROLLDELAY = 507;
	static const int SCROLLTIME = 200;

	int mScrollDir, mScrollAccumulator;
	bool mScrolling;

	Renderer::FontSize mFont;
	int mSelectorColor, mSelectedTextColorOverride;
	bool mDrawCentered;

	int mTextOffsetX;

	struct ListRow
	{
		std::string name;
		listType object;
		int color;
	};

	std::vector<ListRow> mRowVector;
	int mSelection;
};

#include "GuiList.cpp"

#endif
