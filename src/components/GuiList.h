#ifndef _GUILIST_H_
#define _GUILIST_H_

#include "../Renderer.h"
#include "../Font.h"
#include "../Gui.h"
#include "../InputManager.h"
#include <vector>
#include <string>
#include <memory>
#include "../Sound.h"

//A graphical list. Supports multiple colors for rows and scrolling.
//TODO - add truncation to text rendering if name exceeds a maximum width (a trailing elipses, perhaps).
template <typename listType>
class GuiList : public Gui
{
public:
	GuiList(Window* window, int offsetX, int offsetY, Font* font);
	~GuiList();

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

	void addObject(std::string name, listType obj, unsigned int color = 0xFF0000);
	void clear();

	std::string getSelectedName();
	listType getSelectedObject();
	int getSelection();
	void stopScrolling();
	bool isScrolling();

	void setSelectorColor(unsigned int selectorColor);
	void setSelectedTextColor(unsigned int selectedColor);
	void setCentered(bool centered);
	void setScrollSound(std::shared_ptr<Sound> & sound);
	void setTextOffsetX(int textoffsetx);

	int getObjectCount();
	listType getObject(int i);
	void setSelection(int i);

	void setFont(Font* f);

	int getOffsetX();
	int getOffsetY();
private:
	static const int SCROLLDELAY = 507;
	static const int SCROLLTIME = 200;

	int mOffsetX, mOffsetY;

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
	std::shared_ptr<Sound> mScrollSound;
};

#include "GuiList.cpp"

#endif
