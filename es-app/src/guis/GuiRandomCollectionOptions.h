#pragma once
#ifndef ES_APP_GUIS_GUI_RANDOM_COLLECTION_OPTIONS_H
#define ES_APP_GUIS_GUI_RANDOM_COLLECTION_OPTIONS_H

#include "components/MenuComponent.h"


template<typename T>
class OptionListComponent;
class SwitchComponent;
class SystemData;
class GuiSettings;
struct CollectionSystemData;

typedef OptionListComponent<int> NumberList;
struct SystemGames
{
	std::string name;
	std::shared_ptr<NumberList> gamesSelection;
};

class GuiRandomCollectionOptions : public GuiComponent
{
public:
	GuiRandomCollectionOptions(Window* window);
	~GuiRandomCollectionOptions();
	bool input(InputConfig* config, Input input) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void initializeMenu();
	void saveSettings();
	void applyGroupSettings(std::string settingsLabel, const std::map<std::string, int> &initialValues, std::vector<SystemGames>* results);
	void addSystemsToMenu();
	void addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func);
	void selectEntries(std::map<std::string, CollectionSystemData> collection, std::string settingsLabel, int defaultValue, std::vector< SystemGames>* results);

	void selectSystems();
	void selectAutoCollections();
	void selectCustomCollections();

	bool mNeedsCollectionRefresh;

	std::vector< SystemGames> customCollectionLists;
	std::vector< SystemGames> autoCollectionLists;
	std::vector< SystemGames> systemLists;
	std::shared_ptr< NumberList> trimRandom;
	std::shared_ptr< OptionListComponent<std::string> > exclusionCollection;
	MenuComponent mMenu;
	SystemData* mSystem;
};

#endif // ES_APP_GUIS_GUI_RANDOM_COLLECTION_OPTIONS_H
