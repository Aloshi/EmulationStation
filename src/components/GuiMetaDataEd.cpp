#include "GuiMetaDataEd.h"
#include "../Renderer.h"
#include "../Log.h"

#define MDED_RESERVED_ROWS 3

GuiMetaDataEd::GuiMetaDataEd(Window* window, MetaDataList* md, const std::vector<MetaDataDecl>& mdd, 
	const std::string& header, std::function<void()> saveCallback, std::function<void()> deleteFunc) : GuiComponent(window), 
	mBox(mWindow, ":/frame.png", 0xAAAAAAFF, 0xCCCCCCFF),
	mList(window, Eigen::Vector2i(3, mdd.size() + MDED_RESERVED_ROWS)),
	mHeader(window),
	mMetaData(md), 
	mSavedCallback(saveCallback), mDeleteFunc(deleteFunc), 
	mDeleteButton(window), mFetchButton(window), mSaveButton(window)
{
	LOG(LogInfo) << "Creating GuiMetaDataEd";

	//set size to 80% by 80% of the window
	mSize << Renderer::getScreenWidth() * 0.8f, Renderer::getScreenHeight() * 0.8f;

	//center us
	mPosition << (Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2, 0.0f;

	addChild(&mBox);
	mBox.fitTo(mSize);

	//initialize path display
	addChild(&mHeader);
	mHeader.setPosition(0, 0);
	mHeader.setSize(mSize.x(), 0);
	mHeader.setCentered(true);
	mHeader.setText(header);

	//initialize buttons
	mDeleteButton.setText("DELETE", mDeleteFunc ? 0xFF0000FF : 0x555555FF);
	if(mDeleteFunc)
		mDeleteButton.setPressedFunc([&] { mDeleteFunc(); delete this; });

	mFetchButton.setText("FETCH", 0x555555FF);

	mSaveButton.setText("SAVE", 0x0000FFFF);
	mSaveButton.setPressedFunc([&] { save(); delete this; });

	//initialize metadata list
	addChild(&mList);
	populateList(mdd);
	mList.setPosition((mSize.x() - mList.getSize().x()) / 2, mHeader.getSize().y() + 4);
}

GuiMetaDataEd::~GuiMetaDataEd()
{
	LOG(LogInfo) << "Deleted GuiMetaDataEd";
}

void GuiMetaDataEd::populateList(const std::vector<MetaDataDecl>& mdd)
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
		ed->setValue(mMetaData->get(iter->key));
		mList.setEntry(Vector2i(1, y), Vector2i(1, 1), ed, true, ComponentListComponent::AlignRight);
		mEditors.push_back(ed);

		y++;
	}

	//save button
	mList.setEntry(Vector2i(0, y), Vector2i(2, 1), &mSaveButton, true, ComponentListComponent::AlignCenter);
}

void GuiMetaDataEd::save()
{
	for(unsigned int i = 0; i < mLabels.size(); i++)
	{
		mMetaData->set(mLabels.at(i)->getValue(), mEditors.at(i)->getValue());
	}

	if(mSavedCallback)
		mSavedCallback();
}
