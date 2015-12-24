#pragma once

#include "FileData.h"
#include "Renderer.h"

class Window;
class GuiComponent;
class FileData;
class ThemeData;

// This is an interface that defines the minimum for a GameListView.
class IGameListView : public GuiComponent
{
public:
	IGameListView(Window* window, const FileData& root) : GuiComponent(window), mRoot(root)
		{ setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight()); }

	virtual ~IGameListView() {}

	virtual void onFilesChanged() = 0;
	virtual void onMetaDataChanged(const FileData& file) = 0;

	// Called whenever the theme changes.
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) = 0;

	void setTheme(const std::shared_ptr<ThemeData>& theme);
	inline const std::shared_ptr<ThemeData>& getTheme() const { return mTheme; }

	virtual const FileData& getCursor() = 0;
	virtual void setCursor(const FileData& file) = 0;

	virtual bool input(InputConfig* config, Input input) override;

	virtual const char* getName() const = 0;

	virtual HelpStyle getHelpStyle() override;
protected:
	FileData mRoot;
	std::shared_ptr<ThemeData> mTheme;
};
