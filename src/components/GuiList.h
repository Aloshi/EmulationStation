#ifndef _GUILIST_H_
#define _GUILIST_H_

#include "../Renderer.h"
#include "../GuiComponent.h"
#include "../InputManager.h"
#include <vector>
#include <string>

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
	GuiList();
	~GuiList();

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);

	void addObject(std::string name, void* obj, int color = 0xFF0000);
	void clear();

	std::string getSelectedName();
	void* getSelectedObject();
	int getSelection();
private:
	std::vector<ListRow> mRowVector;
	int mSelection;
};

#endif
