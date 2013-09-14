#ifndef _GUIFASTSELECT_H_
#define _GUIFASTSELECT_H_

#include "../GuiComponent.h"
#include "../SystemData.h"
#include "../FolderData.h"
#include "../Sound.h"
#include "ThemeComponent.h"
#include "TextListComponent.h"
#include "NinePatchComponent.h"

class GuiGameList;

class GuiFastSelect : public GuiComponent
{
public:
	GuiFastSelect(Window* window, GuiGameList* parent, TextListComponent<FileData*>* list, char startLetter, ThemeComponent* theme);
	~GuiFastSelect();

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& parentTrans) override;

private:
	static const std::string LETTERS;
	static const int SCROLLSPEED;
	static const int SCROLLDELAY;

	void setListPos();
	void scroll();
	void setLetterID(int id);

	GuiGameList* mParent;
	TextListComponent<FileData*>* mList;
	ThemeComponent * mTheme;
	NinePatchComponent mBox;

	size_t mLetterID;
	
	unsigned int mTextColor;

	int mScrollTimer, mScrollOffset;
	bool mScrolling;

	std::shared_ptr<Sound> mScrollSound;
};

#endif
