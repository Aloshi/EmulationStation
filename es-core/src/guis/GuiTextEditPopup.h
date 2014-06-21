#include "GuiComponent.h"

#include "components/NinePatchComponent.h"
#include "components/ButtonComponent.h"
#include "components/ComponentGrid.h"
#include "components/TextEditComponent.h"
#include "components/TextComponent.h"

class GuiTextEditPopup : public GuiComponent
{
public:
	GuiTextEditPopup(Window* window, const std::string& title, const std::string& initValue, 
		const std::function<void(const std::string&)>& okCallback, bool multiLine, const char* acceptBtnText = "OK");

	bool input(InputConfig* config, Input input);
	void onSizeChanged();
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextEditComponent> mText;
	std::shared_ptr<ComponentGrid> mButtonGrid;

	bool mMultiLine;
};
