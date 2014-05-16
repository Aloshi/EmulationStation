#include "GuiGamelistOptions.h"
#include "GuiMetaDataEd.h"
#include "../views/gamelist/IGameListView.h"

GuiGamelistOptions::GuiGamelistOptions(Window* window, IGameListView* gamelist) : GuiComponent(window), 
	mGamelist(gamelist), 
	mMenu(window, "OPTIONS")
{
	addChild(&mMenu);

	// sort list by
	mListSort = std::make_shared<SortList>(mWindow, "SORT GAMES BY", false);
	for(unsigned int i = 0; i < FileSorts::SortTypes.size(); i++)
	{
		const FileData::SortType& sort = FileSorts::SortTypes.at(i);
		mListSort->add(sort.description, &sort, i == 0); // TODO - actually make the sort type persistent
	}

	mMenu.addWithLabel("SORT GAMES BY", mListSort);

	// edit game metadata
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, "EDIT THIS GAME'S METADATA", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.addElement(makeArrow(mWindow), false);
	row.makeAcceptInputHandler(std::bind(&GuiGamelistOptions::openMetaDataEd, this));
	mMenu.addRow(row);

	// center the menu
	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	mMenu.setPosition((mSize.x() - mMenu.getSize().x()) / 2, (mSize.y() - mMenu.getSize().y()) / 2);
}

GuiGamelistOptions::~GuiGamelistOptions()
{
	// apply sort
	FileData* root = mGamelist->getCursor()->getSystem()->getRootFolder();
	root->sort(*mListSort->getSelected()); // will also recursively sort children

	// notify that the root folder was sorted
	mGamelist->onFileChanged(root, FILE_SORTED);
}

void GuiGamelistOptions::openMetaDataEd()
{
	// open metadata editor
	FileData* file = mGamelist->getCursor();
	ScraperSearchParams p;
	p.game = file;
	p.system = file->getSystem();
	mWindow->pushGui(new GuiMetaDataEd(mWindow, &file->metadata, file->metadata.getMDD(), p, file->getPath().filename().string(), 
		std::bind(&IGameListView::onFileChanged, mGamelist, file, FILE_METADATA_CHANGED), [this, file] { 
			boost::filesystem::remove(file->getPath()); //actually delete the file on the filesystem
			file->getParent()->removeChild(file); //unlink it so list repopulations triggered from onFileChanged won't see it
			mGamelist->onFileChanged(file, FILE_REMOVED); //tell the view
			delete file; //free it
	}));
}

bool GuiGamelistOptions::input(InputConfig* config, Input input)
{
	if((config->isMappedTo("b", input) || config->isMappedTo("select", input)) && input.value)
	{
		delete this;
		return true;
	}

	return mMenu.input(config, input);
}

std::vector<HelpPrompt> GuiGamelistOptions::getHelpPrompts()
{
	auto prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt("b", "close"));
	return prompts;
}
