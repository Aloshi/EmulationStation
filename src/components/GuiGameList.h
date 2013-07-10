#ifndef _GUIGAMELIST_H_
#define _GUIGAMELIST_H_

#include "../GuiComponent.h"
#include "TextListComponent.h"
#include "ImageComponent.h"
#include "ThemeComponent.h"
#include "AnimationComponent.h"
#include "TextComponent.h"
#include <string>
#include <stack>
#include "../SystemData.h"
#include "../GameData.h"
#include "../FolderData.h"
#include "ScrollableContainer.h"

//This is where the magic happens - GuiGameList is the parent of almost every graphical element in ES at the moment.
//It has a TextListComponent child that handles the game list, a ThemeComponent that handles the theming system, and an ImageComponent for game images.
class GuiGameList : public GuiComponent
{
	static std::vector<FolderData::SortState> sortStates;
	size_t sortStateIndex;

public:
	GuiGameList(Window* window);
	virtual ~GuiGameList();

	void setSystemId(int id);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

	void updateDetailData();

    const FolderData::SortState & getSortState() const;
	void setSortIndex(size_t index);
	void setNextSortIndex();
	void setPreviousSortIndex();
	void sort(FolderData::ComparisonFunction & comparisonFunction = FolderData::compareFileName, bool ascending = true);

	static GuiGameList* create(Window* window);

	bool isDetailed() const;

	static const float sInfoWidth;
private:
	void updateList();
	void updateTheme();
	void clearDetailData();
	void doTransition(int dir);

	std::string getThemeFile();

	SystemData* mSystem;
	FolderData* mFolder;
	std::stack<FolderData*> mFolderStack;
	int mSystemId;

	TextListComponent<FileData*> mList;
	ImageComponent mScreenshot;
	TextComponent mDescription;
	ScrollableContainer mDescContainer;
	AnimationComponent mImageAnimation;
	ThemeComponent* mTheme;

	ImageComponent mTransitionImage;
	AnimationComponent mTransitionAnimation;

	Eigen::Vector3f getImagePos();
};

#endif
