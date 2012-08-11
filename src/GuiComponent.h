#ifndef _GUICOMPONENT_H_
#define _GUICOMPONENT_H_

#include <vector>
#include "Renderer.h"
#include "InputManager.h"

class GuiComponent
{
public:
	GuiComponent();
	virtual ~GuiComponent();

	void render();
	virtual void onRender() { };
	virtual void onTick(int deltaTime) { };

	void pause();
	void resume();
	virtual void onPause() { };
	virtual void onResume() { };

	virtual void onInput(InputManager::InputButton button, bool keyDown) { };

	void addChild(GuiComponent* comp);
	void removeChild(GuiComponent* comp);
	void clearChildren();
	unsigned int getChildCount() { return mChildren.size(); }
	GuiComponent* getChild(unsigned int i) { return mChildren.at(i); }

	static void processTicks(int deltaTime);
private:
	static std::vector<GuiComponent*> sComponentVector;
	std::vector<GuiComponent*> mChildren;
};

#endif
