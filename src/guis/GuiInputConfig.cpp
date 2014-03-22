#include "GuiInputConfig.h"
#include "../Window.h"
#include "../Log.h"
#include "../components/TextComponent.h"
#include "../components/ImageComponent.h"
#include "../components/MenuComponent.h"
#include "../components/ButtonComponent.h"
#include "../Util.h"

static const int inputCount = 10;
static const char* inputName[inputCount] = { "Up", "Down", "Left", "Right", "A", "B", "Start", "Select", "PageUp", "PageDown"};
static const char* inputDispName[inputCount] = { "Up", "Down", "Left", "Right", "A", "B", "Start", "Select", "Page Up", "Page Down"};
static const char* inputIcon[inputCount] = { ":/help/dpad_up.png", ":/help/dpad_down.png", ":/help/dpad_left.png", ":/help/dpad_right.png", 
											":/help/a.png", ":/help/b.png", ":/help/start.png", ":/help/select.png", ":/help/l.png", ":/help/r.png" };

//MasterVolUp and MasterVolDown are also hooked up, but do not appear on this screen.
//If you want, you can manually add them to es_input.cfg.

using namespace Eigen;

GuiInputConfig::GuiInputConfig(Window* window, InputConfig* target, bool reconfigureAll, const std::function<void()>& okCallback) : GuiComponent(window), 
	mBackground(window, ":/frame.png"), mGrid(window, Vector2i(1, 5)), 
	mTargetConfig(target)
{
	LOG(LogInfo) << "Configuring device " << target->getDeviceId();

	if(reconfigureAll)
		target->clear();

	mConfiguringAll = reconfigureAll;
	mConfiguringRow = mConfiguringAll;

	addChild(&mBackground);
	addChild(&mGrid);

	mTitle = std::make_shared<TextComponent>(mWindow, "PLEASE CONFIGURE INPUT FOR", Font::get(FONT_SIZE_SMALL), 0x555555FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);
	
	mSubtitle1 = std::make_shared<TextComponent>(mWindow, target->getDeviceName(), Font::get(FONT_SIZE_MEDIUM), 0x555555FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mSubtitle1, Vector2i(0, 1), false, true);

	mSubtitle2 = std::make_shared<TextComponent>(mWindow, "(HOLD ANY BUTTON TO SKIP BUT NOT YET)", Font::get(FONT_SIZE_SMALL), 0x999999FF, TextComponent::ALIGN_CENTER);
	mGrid.setEntry(mSubtitle2, Vector2i(0, 2), false, true);

	mList = std::make_shared<ComponentList>(mWindow);
	mGrid.setEntry(mList, Vector2i(0, 3), true, true);
	for(int i = 0; i < inputCount; i++)
	{
		ComponentListRow row;
		
		// icon
		auto icon = std::make_shared<ImageComponent>(mWindow);
		icon->setImage(inputIcon[i]);
		icon->setResize(0, Font::get(FONT_SIZE_MEDIUM)->getHeight() * 0.8f);
		row.addElement(icon, false);

		// spacer between icon and text
		auto spacer = std::make_shared<GuiComponent>(mWindow);
		spacer->setSize(16, 0);
		row.addElement(spacer, false);

		auto text = std::make_shared<TextComponent>(mWindow, inputDispName[i], Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		row.addElement(text, true);

		auto mapping = std::make_shared<TextComponent>(mWindow, "-NOT DEFINED-", Font::get(FONT_SIZE_MEDIUM, FONT_PATH_LIGHT), 0x999999FF, TextComponent::ALIGN_RIGHT);
		setNotDefined(mapping); // overrides text and color set above
		row.addElement(mapping, true);
		mMappings.push_back(mapping);

		row.input_handler = [this, i, mapping](InputConfig* config, Input input) -> bool
		{
			if(!input.value)
				return false;

			if(mConfiguringRow)
			{
				if(!process(config, input, i, mapping)) // button press invalid; try again
					return true;
				if(mConfiguringAll)
				{
					if(!mList->moveCursor(1)) // try to move to the next one
					{
						// at bottom of list
						mConfiguringAll = false;
						mConfiguringRow = false;
						mGrid.moveCursor(Vector2i(0, 1));
					}else{
						// on another one
						setPress(mMappings.at(mList->getCursorId()));
					}
				}else{
					mConfiguringRow = false; // we only wanted to configure one row
				}
				return true;
			}else{
				// not configuring, start configuring when A is pressed
				if(config->isMappedTo("a", input) && input.value)
				{
					mConfiguringRow = true;
					setPress(mapping);
					return true;
				}
				return false;
			}
			
			return false;
		};
		mList->addRow(row);
	}

	// make the first one say "NOT DEFINED" if we're re-configuring everything
	if(mConfiguringAll)
		setPress(mMappings.front());

	// buttons
	std::vector< std::shared_ptr<ButtonComponent> > buttons;
	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "OK", "ok", [this, okCallback] { 
		mWindow->getInputManager()->writeDeviceConfig(mTargetConfig); // save
		if(okCallback)
			okCallback();
		delete this; 
	}));
	mButtonGrid = makeButtonGrid(mWindow, buttons);
	mGrid.setEntry(mButtonGrid, Vector2i(0, 4), true, false);

	setSize(Renderer::getScreenWidth() * 0.6f, Renderer::getScreenHeight() * 0.7f);
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
}

void GuiInputConfig::onSizeChanged()
{
	mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	// update grid
	mGrid.setSize(mSize);

	mGrid.setRowHeightPerc(0, mTitle->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(1, mSubtitle1->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(2, mSubtitle2->getFont()->getHeight() / mSize.y());
	mGrid.setRowHeightPerc(4, mButtonGrid->getSize().y() / mSize.y());
}

void GuiInputConfig::setPress(const std::shared_ptr<TextComponent>& text)
{
	text->setText("PRESS ANYTHING");
	text->setColor(0x656565FF);
}

void GuiInputConfig::setNotDefined(const std::shared_ptr<TextComponent>& text)
{
	text->setText("-NOT DEFINED-");
	text->setColor(0x999999FF);
}

void GuiInputConfig::error(const std::shared_ptr<TextComponent>& text, const std::string& msg)
{
	text->setText("ALREADY TAKEN");
	text->setColor(0x656565FF);
}

bool GuiInputConfig::process(InputConfig* config, Input input, int inputId, const std::shared_ptr<TextComponent>& text)
{
	// from some other input source
	if(config != mTargetConfig)
		return false;

	// if this input is mapped to something other than "nothing" or the current row, error
	// (if it's the same as what it was before, allow it)
	if(config->getMappedTo(input).size() > 0 && !config->isMappedTo(inputName[inputId], input))
	{
		error(text, "Already mapped!");
		return false;
	}

	text->setText(strToUpper(input.string()));
	text->setColor(0x777777FF);

	input.configured = true;
	config->mapInput(inputName[inputId], input);

	LOG(LogInfo) << "  Mapping [" << input.string() << "] -> " << inputName[inputId];

	return true;
}
