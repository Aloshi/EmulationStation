#pragma once

#include "../FileData.h"
#include "../Renderer.h"

class Window;
class GuiComponent;
class FileData;
class ThemeData;

//GameListView needs to know:
//  What theme data to use
//  The root FileData for the tree it should explore

class GameListView : public GuiComponent
{
public:
	GameListView(Window* window, FileData* root) : GuiComponent(window), mRoot(root)
		{ setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight()); }

	virtual ~GameListView() {}

	// Called when a new file is added, a file is removed, or a file's metadata changes.
	virtual void onFileChanged(FileData* file, FileChangeType change) = 0;
	
	// Called to set or update theme.
	virtual void setTheme(const std::shared_ptr<ThemeData>& theme) = 0;

	virtual bool input(InputConfig* config, Input input) override;

	virtual FileData* getCursor() = 0;
	virtual void setCursor(FileData*) = 0;

protected:
	FileData* mRoot;
};
