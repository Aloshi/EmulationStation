#ifndef _GUILIST_H_
#define _GUILIST_H_

#include "../Renderer.h"
#include "../Font.h"
#include "../GuiComponent.h"
#include "../InputManager.h"
#include <vector>
#include <string>
#include "../Sound.h"

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

	void addObject(std::string name, listType obj, unsigned int color = 0xFF0000);
	void clear();

	void onPause();
	void onResume();

	std::string getSelectedName();
	listType getSelectedObject();
	int getSelection();
	void stopScrolling();
	bool isScrolling();

	void setSelectorColor(unsigned int selectorColor);
	void setSelectedTextColor(unsigned int selectedColor);
	void setCentered(bool centered);
	void setScrollSound(Sound* sound);
	void setTextOffsetX(int textoffsetx);

	int getObjectCount();
	listType getObject(int i);
	void setSelection(int i);

private:
	static const int SCROLLDELAY = 507;
	static const int SCROLLTIME = 200;

	void scroll(); //helper method, scrolls in whatever direction scrollDir is

	int mScrollDir, mScrollAccumulator;
	bool mScrolling;

	Font* mFont;
	unsigned int mSelectorColor, mSelectedTextColorOverride;
	bool mDrawCentered;

	int mTextOffsetX;

	struct ListRow
	{
		std::string name;
		listType object;
		unsigned int color;
	};

	std::vector<ListRow> mRowVector;
	int mSelection;
	Sound* mScrollSound;
};

#include "GuiList.cpp"

#endif
