#ifndef _GUILIST_H_
#define _GUILIST_H_

#include "../Renderer.h"
#include "../GuiComponent.h"
#include <vector>
#include <string>

class GuiList : public GuiComponent
{
public:
	GuiList();

	void onRender();

	void addObject(std::string name, void* obj);
	void clear();

	std::string getSelectedName();
	void* getSelectedObject();
private:
	std::vector<std::string> mNameVector;
	std::vector<void*> mPointerVector;
	unsigned int mSelection;
};

#endif
