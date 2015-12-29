#pragma once
#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "GamelistDB.h"

class IGameListView;

class GuiGamelistOptions : public GuiComponent
{
public:
	GuiGamelistOptions(Window* window, SystemData* system);
	virtual ~GuiGamelistOptions();

	virtual bool input(InputConfig* config, Input input) override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func);
	void openMetaDataEd();
	void openFilterAddStart();
	void openFilterAdd(const FileData& file, const std::string &filterid);
	void jumpToLetter();
	
	MenuComponent mMenu;

	typedef OptionListComponent<char> LetterList;
	std::shared_ptr<LetterList> mJumpToLetterList;

	typedef OptionListComponent<int> SortList;
	std::shared_ptr<SortList> mListSort;
        
	std::shared_ptr<SliderComponent> mVolume;
	
	SystemData* mSystem;
	IGameListView* getGamelist();
};
