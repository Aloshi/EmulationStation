#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "GuiComponent.h"
#include "InputManager.h"
#include "resources/ResourceManager.h"
#include <vector>
#include "Font.h"

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

	InputManager* getInputManager();
	ResourceManager* getResourceManager();

private:
	InputManager* mInputManager;
	ResourceManager mResourceManager;
	std::vector<GuiComponent*> mGuiStack;

	std::vector< std::shared_ptr<Font> > mDefaultFonts;
};

#endif
