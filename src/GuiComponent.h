#ifndef _GUICOMPONENT_H_
#define _GUICOMPONENT_H_

#include <vector>
#include "Renderer.h"
#include "InputManager.h"

/*
The GuiComponent class is what everything that is rendered, updated by time (ticking), or takes input is subclassed from.
GuiComponents have a list of child GuiComponents. Rendering, ticking, pausing/resuming, init/deinit, are all automatically sent to children.
You can rely on the parent getting called first - this way, you can control what order components are rendered in.
You can also manually call the render/pause/resume/init/deinit methods if you so desire (e.g. want a child to render *before* its parent).

To make a GuiComponent render/take input, you must register with the Renderer or InputManager respectively (Renderer::registerComponent(comp) or InputManager::registerComponent(comp)).
All components are automatically ticked every frame, just add an onTick(int deltaTime) method.
onInput calls arrive before onRender calls.
*/

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

	void init();
	void deinit();
	virtual void onInit() { };
	virtual void onDeinit() { };

	virtual void onInput(InputManager::InputButton button, bool keyDown) { };

	void addChild(GuiComponent* comp);
	void removeChild(GuiComponent* comp);
	void clearChildren();
	unsigned int getChildCount() { return mChildren.size(); }
	GuiComponent* getChild(unsigned int i) { return mChildren.at(i); }


	int getOffsetX();
	int getOffsetY();
	void setOffsetX(int val);
	void setOffsetY(int val);

	static void processTicks(int deltaTime);
private:
	int mOffsetX, mOffsetY;

	static std::vector<GuiComponent*> sComponentVector;
	std::vector<GuiComponent*> mChildren;
};

#endif
