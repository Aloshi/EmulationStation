#ifndef _GUILIST_H_
#define _GUILIST_H_

#include "../Renderer.h"
#include "../GuiComponent.h"
#include "../InputManager.h"
#include <vector>
#include <string>

#define SCROLLDELAY 507
#define SCROLLTIME (57*6)

struct ListRow
{
	std::string name;
	void* object;
	int color;
};

//this should really be a template
class GuiList : public GuiComponent
{
public:
	GuiList(int offsetX = 0, int offsetY = 0);
	~GuiList();

	void onRender();
	void onTick(int deltaTime);
	void onInput(InputManager::InputButton button, bool keyDown);

	void addObject(std::string name, void* obj, int color = 0xFF0000);
	void clear();

	std::string getSelectedName();
	void* getSelectedObject();
	int getSelection();
private:
	int mScrollDir, mScrollAccumulator;
	bool mScrolling;

	int mOffsetX, mOffsetY;
	std::vector<ListRow> mRowVector;
	int mSelection;
};

#endif
