#include "guis/GuiGamelistFilter.h"
#include "guis/GuiMsgBox.h"
#include "views/ViewController.h"

#include "components/TextComponent.h"
#include "components/OptionListComponent.h"

GuiGamelistFilter::GuiGamelistFilter(Window* window, SystemData* system) : GuiComponent(window), mMenu(window, "FILTER GAMELIST BY"), mSystem(system)
{
	initializeMenu();
}

void GuiGamelistFilter::initializeMenu()
{
	addChild(&mMenu);

	// get filters from system

	mFilterIndex = mSystem->getIndex();

	ComponentListRow row;

	// show filtered menu
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, "RESET ALL FILTERS", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.makeAcceptInputHandler(std::bind(&GuiGamelistFilter::resetAllFilters, this));
	mMenu.addRow(row);
	row.elements.clear();

	addFiltersToMenu();

	mMenu.addButton("BACK", "back", std::bind(&GuiGamelistFilter::applyFilters, this));

	mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiGamelistFilter::resetAllFilters()
{
	mFilterIndex->clearAllFilters();
	for (std::map<FilterIndexType, std::shared_ptr< OptionListComponent<std::string> >>::iterator it = mFilterOptions.begin(); it != mFilterOptions.end(); ++it ) {
		std::shared_ptr< OptionListComponent<std::string> > optionList = it->second;
		optionList->selectNone();
	}
}

GuiGamelistFilter::~GuiGamelistFilter()
{
	mFilterOptions.clear();
}

void GuiGamelistFilter::addFiltersToMenu()
{
	std::vector<FilterDataDecl> decls = mFilterIndex->getFilterDataDecls();
	for (std::vector<FilterDataDecl>::iterator it = decls.begin(); it != decls.end(); ++it ) {

		FilterIndexType type = (*it).type; // type of filter
		std::map<std::string, int>* allKeys = (*it).allIndexKeys; // all possible filters for this type
		std::vector<std::string>* allFilteredKeys = (*it).currentFilteredKeys; // current keys being filtered for
		std::string menuLabel = (*it).menuLabel; // text to show in menu
		std::shared_ptr< OptionListComponent<std::string> > optionList;


		// add filters (with first one selected)
		ComponentListRow row;

		// add genres
		optionList = std::make_shared< OptionListComponent<std::string> >(mWindow, menuLabel, true);
		for(auto it: *allKeys)
		{
			optionList->add(it.first, it.first, mFilterIndex->isKeyBeingFilteredBy(it.first, type));
		}
		if (allKeys->size() > 0)
			mMenu.addWithLabel(menuLabel, optionList);

		mFilterOptions[type] = optionList;
	}
}

void GuiGamelistFilter::applyFilters()
{
	std::vector<FilterDataDecl> decls = mFilterIndex->getFilterDataDecls();
	for (std::map<FilterIndexType, std::shared_ptr< OptionListComponent<std::string> >>::iterator it = mFilterOptions.begin(); it != mFilterOptions.end(); ++it ) {
		std::shared_ptr< OptionListComponent<std::string> > optionList = it->second;
		std::vector<std::string> filters = optionList->getSelectedObjects();
		mFilterIndex->setFilter(it->first, &filters);
	}

	delete this;

}

bool GuiGamelistFilter::input(InputConfig* config, Input input)
{
	bool consumed = GuiComponent::input(config, input);
	if(consumed)
		return true;

	if(config->isMappedTo("b", input) && input.value != 0)
	{
		applyFilters();
	}


	return false;
}

std::vector<HelpPrompt> GuiGamelistFilter::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "back"));
	return prompts;
}
