#ifndef _GUICOMPONENT_H_
#define _GUICOMPONENT_H_

#include "InputConfig.h"
#include <memory>
#include <Eigen/Dense>

class Window;
class Animation;
class AnimationController;

class GuiComponent
{
public:
	GuiComponent(Window* window);
	virtual ~GuiComponent();

	virtual void textInput(const char* text);

	//Called when input is received.
	//Return true if the input is consumed, false if it should continue to be passed to other children.
	virtual bool input(InputConfig* config, Input input);

	//Called when time passes.  Default implementation also calls update(deltaTime) on children - so you should probably call GuiComponent::update(deltaTime) at some point.
	virtual void update(int deltaTime);

	//Called when it's time to render.  By default, just calls renderChildren(parentTrans * getTransform()).
	//You probably want to override this like so:
	//1. Calculate the new transform that your control will draw at with Eigen::Affine3f t = parentTrans * getTransform().
	//2. Set the renderer to use that new transform as the model matrix - Renderer::setMatrix(t);
	//3. Draw your component.
	//4. Tell your children to render, based on your component's transform - renderChildren(t).
	virtual void render(const Eigen::Affine3f& parentTrans);

	Eigen::Vector3f getPosition() const;
	void setPosition(const Eigen::Vector3f& offset);
	void setPosition(float x, float y, float z = 0.0f);
	virtual void onPositionChanged() {};

	Eigen::Vector2f getSize() const;
    void setSize(const Eigen::Vector2f& size);
    void setSize(float w, float h);
    virtual void onSizeChanged() {};
	
	void setParent(GuiComponent* parent);
	GuiComponent* getParent() const;

	void addChild(GuiComponent* cmp);
	void removeChild(GuiComponent* cmp);
	void clearChildren();
	unsigned int getChildCount() const;
	GuiComponent* getChild(unsigned int i) const;

	// animation will be automatically deleted when it completes or is stopped.
	void setAnimation(Animation* animation, std::function<void()> finishedCallback = nullptr, bool reverse = false, unsigned char slot = 0);
	void stopAnimation(unsigned char slot);

	virtual unsigned char getOpacity() const;
	virtual void setOpacity(unsigned char opacity);

	const Eigen::Affine3f getTransform();

	virtual std::string getValue() const;
	virtual void setValue(const std::string& value);

	virtual void onFocusGained() {};
	virtual void onFocusLost() {};

protected:
	void renderChildren(const Eigen::Affine3f& transform) const;

	unsigned char mOpacity;
	Window* mWindow;

	GuiComponent* mParent;
	std::vector<GuiComponent*> mChildren;

	Eigen::Vector3f mPosition;
	Eigen::Vector2f mSize;

public:
	const static unsigned char MAX_ANIMATIONS = 4;

private:
	Eigen::Affine3f mTransform; //Don't access this directly! Use getTransform()!
	AnimationController* mAnimationMap[MAX_ANIMATIONS];
};

#endif
