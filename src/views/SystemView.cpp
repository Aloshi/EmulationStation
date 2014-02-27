#include "SystemView.h"
#include "../SystemData.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"
#include "ViewController.h"
#include "../animations/LambdaAnimation.h"
#include "../SystemData.h"

#define SELECTED_SCALE 1.25f
#define LOGO_PADDING ((logoSize().x() * (SELECTED_SCALE - 1)/2) + (mSize.x() * 0.075f))

SystemView::SystemView(Window* window) : IList<SystemViewData, SystemData*>(window)
{
	mCamOffset = 0;

	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	populate();
}

void SystemView::populate()
{
	mEntries.clear();

	for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); it++)
	{
		const std::shared_ptr<ThemeData>& theme = (*it)->getTheme();

		Entry e;
		e.name = (*it)->getName();
		e.object = *it;

		// make logo
		if(theme->getElement("system", "header", "image"))
		{
			ImageComponent* logo = new ImageComponent(mWindow);
			logo->setMaxSize(logoSize());
			logo->applyTheme((*it)->getTheme(), "system", "header", ThemeFlags::PATH);
			logo->setPosition((logoSize().x() - logo->getSize().x()) / 2, (logoSize().y() - logo->getSize().y()) / 2); // vertically and horizontally center
			e.data.logo = std::shared_ptr<GuiComponent>(logo);
		}else{
			// no logo in theme; use text
			TextComponent* text = new TextComponent(mWindow);
			text->setFont(Font::get(FONT_SIZE_LARGE));
			text->setText((*it)->getName());
			text->setSize(logoSize().x(), 0);
			text->setPosition(0, (logoSize().y() - text->getSize().y()) / 2); // vertically center
			text->setCentered(true); // horizontally center
			e.data.logo = std::shared_ptr<GuiComponent>(text);
		}

		// make background extras
		e.data.backgroundExtras = std::shared_ptr<ThemeExtras>(new ThemeExtras(mWindow));
		e.data.backgroundExtras->setExtras(ThemeData::makeExtras((*it)->getTheme(), "system", mWindow));

		this->add(e);
	}
}

void SystemView::goToSystem(SystemData* system)
{
	setCursor(system);
}

bool SystemView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("left", input))
		{
			listInput(-1);
			return true;
		}
		if(config->isMappedTo("right", input))
		{
			listInput(1);
			return true;
		}
		if(config->isMappedTo("a", input))
		{
			stopScrolling();
			mWindow->getViewController()->goToGameList(getSelected());
			return true;
		}
	}else{
		if(config->isMappedTo("left", input) || config->isMappedTo("right", input))
			listInput(0);
	}

	return GuiComponent::input(config, input);
}

void SystemView::update(int deltaTime)
{
	listUpdate(deltaTime);
	GuiComponent::update(deltaTime);
}

void SystemView::onCursorChanged(const CursorState& state)
{
	float startPos = mCamOffset;

	float posMax = (float)mEntries.size();
	float target = (float)mCursor;

	// what's the shortest way to get to our target?
	// it's one of these...

	float endPos = target; // directly
	float dist = abs(endPos - startPos);
	
	if(abs(target + posMax - startPos) < dist)
		endPos = target + posMax; // loop around the end (0 -> max)
	if(abs(target - posMax - startPos) < dist)
		endPos = target - posMax; // loop around the start (max - 1 -> -1)

	Animation* anim = new LambdaAnimation(
		[startPos, endPos, posMax, this] (float t)
	{
		t -= 1;
		float f = lerp<float>(startPos, endPos, t*t*t + 1);
		if(f < 0)
			f += posMax;
		if(f >= posMax)
			f -= posMax;
		this->mCamOffset = f;
	}, 500);

	setAnimation(anim);
}

void SystemView::render(const Eigen::Affine3f& parentTrans)
{
	if(size() == 0)
		return;

	Eigen::Affine3f trans = getTransform() * parentTrans;
	
	// draw the list elements (titles, backgrounds, logos)
	Eigen::Affine3f logoTrans = trans;

	const float logoSizeX = logoSize().x() + LOGO_PADDING;

	int logoCount = (int)(mSize.x() / logoSizeX) + 2; // how many logos we need to draw
	int center = (int)(mCamOffset);

	// draw background extras
	Eigen::Affine3f extrasTrans = trans;
	for(int i = center - 1; i < center + 2; i++)
	{
		int index = i;
		while(index < 0)
			index += mEntries.size();
		while(index >= (int)mEntries.size())
			index -= mEntries.size();

		extrasTrans.translation() = trans.translation() + Eigen::Vector3f((i - mCamOffset) * mSize.x(), 0, 0);
		
		mEntries.at(index).data.backgroundExtras->render(extrasTrans);
	}

	// draw logos
	float xOff = (mSize.x() - logoSize().x())/2 - (mCamOffset * logoSizeX);
	float yOff = (mSize.y() - logoSize().y())/2;

	// background behind the logos
	Renderer::setMatrix(trans);
	Renderer::drawRect(0, yOff - (logoSize().y() * 0.2f) / 2, mSize.x(), logoSize().y() * 1.2f, 0xFFFFFFBB);

	for(int i = center - logoCount/2; i < center + logoCount/2 + 1; i++)
	{
		int index = i;
		while(index < 0)
			index += mEntries.size();
		while(index >= (int)mEntries.size())
			index -= mEntries.size();

		logoTrans.translation() = trans.translation() + Eigen::Vector3f(i * logoSizeX + xOff, yOff, 0);

		std::shared_ptr<GuiComponent> comp = mEntries.at(index).data.logo;
		if(comp)
		{
			if(index == mCursor) //scale our selection up
			{
				// fix the centering because we go by left corner and not center (bleh)
				logoTrans.translation() -= Eigen::Vector3f(((comp->getSize().x() + comp->getPosition().x()) / 2) * (SELECTED_SCALE - 1), 
					((comp->getSize().y() + comp->getPosition().y()) / 2) * (SELECTED_SCALE - 1), 0);
				logoTrans.scale(Eigen::Vector3f(SELECTED_SCALE, SELECTED_SCALE, 1.0f));
				mEntries.at(index).data.logo->render(logoTrans);
				logoTrans.scale(Eigen::Vector3f(1/SELECTED_SCALE, 1/SELECTED_SCALE, 1.0f));
			}else{
				mEntries.at(index).data.logo->render(logoTrans);
			}
		}
	}
}

std::vector<HelpPrompt> SystemView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("left/right", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	return prompts;
}
