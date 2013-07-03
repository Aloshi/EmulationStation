#ifndef _GUICOMPONENT_H_
#define _GUICOMPONENT_H_

#include "InputConfig.h"
#include "Vector2.h"

class Window;

class GuiComponent
{
public:
	GuiComponent(Window* window);
	virtual ~GuiComponent();

	//Called when input is received.
	//Return true if the input is consumed, false if it should continue to be passed to other children.
	virtual bool input(InputConfig* config, Input input);

	//Called when time passes.  Default implementation also calls update(deltaTime) on children - so you should probably call GuiComponent::update(deltaTime) at some point.
	virtual void update(int deltaTime);

	//Called when it's time to render.  Translates the OpenGL matrix, calls onRender() (which renders children), then un-translates the OpenGL matrix.
	//You probably don't need to override this, and should use the protected method onRender.
	virtual void render();

	//Called when the Renderer initializes.  Passes to children.
	virtual void init();

	//Called when the Renderer deinitializes.  Passes to children.
	virtual void deinit();

	virtual Vector2i getGlobalOffset();
	Vector2i getOffset();
	void setOffset(Vector2i offset);
	void setOffset(int x, int y);
	virtual void onOffsetChanged() {};

	Vector2u getSize();
    void setSize(Vector2u size);
    void setSize(unsigned int w, unsigned int h);
    virtual void onSizeChanged() {};
	
	void setParent(GuiComponent* parent);
	GuiComponent* getParent();

	void addChild(GuiComponent* cmp);
	void removeChild(GuiComponent* cmp);
	void clearChildren();
	unsigned int getChildCount();
	GuiComponent* getChild(unsigned int i);
	unsigned char getOpacity();
	void setOpacity(unsigned char opacity);

protected:
	//Default implementation just renders children - you should probably always call GuiComponent::onRender at some point in your custom onRender.
	virtual void onRender();

	unsigned char mOpacity;
	Window* mWindow;
	GuiComponent* mParent;
	Vector2i mOffset;
	Vector2u mSize;
	std::vector<GuiComponent*> mChildren;
};

#endif
