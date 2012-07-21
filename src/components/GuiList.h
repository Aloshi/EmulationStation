#ifndef _GUILIST_H_
#define _GUILIST_H_

#include "../Renderer.h"
#include "../GuiComponent.h"
#include "../InputManager.h"
#include <vector>
#include <string>

class GuiList : public GuiComponent
{
public:
	GuiList();
	~GuiList();

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);

	void addObject(std::string name, void* obj);
	void clear();

	std::string getSelectedName();
	void* getSelectedObject();
	int getSelection();
private:
	std::vector<std::string> mNameVector;
	std::vector<void*> mPointerVector;
	int mSelection;
};

#endif
