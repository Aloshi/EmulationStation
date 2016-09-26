#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/OptionListComponent.h"
#include "components/SwitchComponent.h"
#include "FileSorts.h"

class IGameListView;

class GuiGamelistOptions : public GuiComponent
{
public:
	GuiGamelistOptions(Window* window, SystemData* system);
	virtual ~GuiGamelistOptions();

	void save();
	inline void addSaveFunc(const std::function<void()>& func) { mSaveFuncs.push_back(func); };

	virtual bool input(InputConfig* config, Input input) override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void openMetaDataEd();
	void jumpToLetter();
	void SurpriseMe();


	MenuComponent mMenu;

	std::vector< std::function<void()> > mSaveFuncs;

	typedef OptionListComponent<char> LetterList;
	std::shared_ptr<LetterList> mJumpToLetterList;

	typedef OptionListComponent<const FileData::SortType*> SortList;
	std::shared_ptr<SortList> mListSort;

	std::shared_ptr<SwitchComponent> mFavoriteOption;
	bool mFavoriteStateChanged;
	bool mHiddenStateChanged;

	typedef OptionListComponent<std::string> SurpriseEnums;
	std::shared_ptr<SurpriseEnums> mSurpriseModes;

	SystemData* mSystem;
	IGameListView* getGamelist();
};
