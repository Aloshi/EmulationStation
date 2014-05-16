#include "../GuiComponent.h"
#include "../components/MenuComponent.h"
#include "../components/OptionListComponent.h"

#include "../FileSorts.h"

class IGameListView;

class GuiGamelistOptions : public GuiComponent
{
public:
	GuiGamelistOptions(Window* window, SystemData* system);
	virtual ~GuiGamelistOptions();

	virtual bool input(InputConfig* config, Input input) override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void openMetaDataEd();

	MenuComponent mMenu;

	typedef OptionListComponent<const FileData::SortType*> SortList;
	std::shared_ptr<SortList> mListSort;
	
	SystemData* mSystem;
	IGameListView* getGamelist();
};
