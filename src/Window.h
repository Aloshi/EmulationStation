#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "GuiComponent.h"
#include "InputManager.h"
#include <vector>
#include "resources/Font.h"

class ViewController;

class Window
{
public:
	Window();
	~Window();

	void pushGui(GuiComponent* gui);
	void removeGui(GuiComponent* gui);
	GuiComponent* peekGui();

	void input(InputConfig* config, Input input);
	void update(int deltaTime);
	void render();

	bool init(unsigned int width = 0, unsigned int height = 0);
	void deinit();

	inline InputManager* getInputManager() { return mInputManager; }
	inline ViewController* getViewController() { return mViewController; }

	void normalizeNextUpdate();

	void setZoomFactor(const float& zoom);
	void setCenterPoint(const Eigen::Vector2f& point);

	void setFadePercent(const float& perc);

	bool getAllowSleep();
	void setAllowSleep(bool sleep);
	
private:
	InputManager* mInputManager;
	ViewController* mViewController;
	std::vector<GuiComponent*> mGuiStack;

	std::vector< std::shared_ptr<Font> > mDefaultFonts;

	int mFrameTimeElapsed;
	int mFrameCountElapsed;
	int mAverageDeltaTime;
	std::string mFrameDataString;

	bool mNormalizeNextUpdate;

	float mZoomFactor;
	Eigen::Vector2f mCenterPoint;

	void updateMatrix();
	Eigen::Affine3f mMatrix;

	void postProcess();
	float mFadePercent;

	bool mAllowSleep;
};

#endif
