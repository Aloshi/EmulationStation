#ifndef _GUIFASTSELECT_H_
#define _GUIFASTSELECT_H_

#include "../GuiComponent.h"
#include "../SystemData.h"

class GuiFastSelect : GuiComponent
{
public:
	GuiFastSelect(GuiComponent* parent, SystemData* system, char startLetter);
	~GuiFastSelect();

	void onRender();
	void onInput(InputManager::InputButton button, bool keyDown);
	void onTick(int deltaTime);
private:
	static const std::string LETTERS;
	static const int SCROLLSPEED;
	static const int SCROLLDELAY;

	void setLetterID(int id);

	SystemData* mSystem;
	size_t mLetterID;
	GuiComponent* mParent;
	int mScrollTimer, mScrollOffset;
	bool mScrolling;
};

#endif
