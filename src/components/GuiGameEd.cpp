#include "GuiGameEd.h"
#include "../Renderer.h"
#include "../Log.h"

#define MDED_RESERVED_ROWS 3

GuiGameEd::GuiGameEd(Window* window, GameData* game, const std::vector<MetaDataDecl>& mdd) : GuiComponent(window), 
	mBox(mWindow, 0, 0, 0, 0),
	mList(window, Eigen::Vector2i(3, mdd.size() + MDED_RESERVED_ROWS)),
	mPathDisp(window),
	mGame(game),
	mDeleteButton(window), mFetchButton(window), mSaveButton(window)
{
	LOG(LogInfo) << "Creating GuiGameEd";

	//set size to 80% by 80% of the window
	mSize << Renderer::getScreenWidth() * 0.8f, Renderer::getScreenHeight() * 0.8f;

	//center us
	mPosition << (Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2, 0.0f;

	addChild(&mBox);
	mBox.setVerticalImage(":/bar.png");
	mBox.setHorizontalImage(":/bar.png");
	mBox.setCornerImage(":/corner.png");
	mBox.setBorderColor(0x666666FF);
	mBox.setPosition(0, 0);
	mBox.setSize(mSize);
	mBox.setBackgroundImage(":/bar.png");
	mBox.setBackgroundColor(0xAAAAAAAA);

	//initialize path display
	addChild(&mPathDisp);
	mPathDisp.setPosition(0, 0);
	mPathDisp.setSize(mSize.x(), 0);
	mPathDisp.setCentered(true);
	mPathDisp.setText(mGame->getBaseName());

	//initialize buttons
	mDeleteButton.setText("DELETE", 0x555555FF);
	mFetchButton.setText("FETCH", 0x555555FF);

	mSaveButton.setText("SAVE", 0x0000FFFF);
	mSaveButton.setPressedFunc([&] { save(); delete this; });

	//initialize metadata list
	addChild(&mList);
	populateList(mdd);
	mList.setPosition((mSize.x() - mList.getSize().x()) / 2, mPathDisp.getSize().y() + 4);
}

GuiGameEd::~GuiGameEd()
{
	LOG(LogInfo) << "Deleted GuiGameEd";
}

void GuiGameEd::populateList(const std::vector<MetaDataDecl>& mdd)
{
	//      PATH		//(centered, not part of componentlist)

	//---GAME NAME---   //(at 1,1; size 3,1) (TextEditComponent)
	//DEL   SCRP  ---   //(buttons)
	//Fav:  Y/N   ---	//metadata start
	//Desc: ...   ---	//multiline texteditcomponent
	//Img:  ...   ---
	//---   SAVE  ---

	using namespace Eigen;

	int y = 0;

	//delete button
	mList.setEntry(Vector2i(0, y), Vector2i(1, 1), &mDeleteButton, true, ComponentListComponent::AlignCenter);

	//fetch button
	mList.setEntry(Vector2i(1, y), Vector2i(1, 1), &mFetchButton, true, ComponentListComponent::AlignCenter);
	
	y++;

	for(auto iter = mdd.begin(); iter != mdd.end(); iter++)
	{
		TextComponent* label = new TextComponent(mWindow);
		label->setText(iter->key);
		mList.setEntry(Vector2i(0, y), Vector2i(1, 1), label, false, ComponentListComponent::AlignLeft);
		mLabels.push_back(label);

		GuiComponent* ed = MetaDataList::makeEditor(mWindow, iter->type);
		ed->setSize(mSize.x() / 2, ed->getSize().y());
		ed->setValue(mGame->metadata()->get(iter->key));
		mList.setEntry(Vector2i(1, y), Vector2i(1, 1), ed, true, ComponentListComponent::AlignRight);
		mEditors.push_back(ed);

		y++;
	}

	//save button
	mList.setEntry(Vector2i(0, y), Vector2i(2, 1), &mSaveButton, true, ComponentListComponent::AlignCenter);
}

void GuiGameEd::save()
{
	for(unsigned int i = 0; i < mLabels.size(); i++)
	{
		mGame->metadata()->set(mLabels.at(i)->getValue(), mEditors.at(i)->getValue());
	}
}
