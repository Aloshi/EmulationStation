#include "SystemView.h"
#include "../SystemData.h"
#include "../Renderer.h"
#include "../Log.h"
#include "../Window.h"
#include "ViewController.h"
#include "../animations/LambdaAnimation.h"
#include "../SystemData.h"

#define SELECTED_SCALE 1.2f
#define LOGO_PADDING (logoSize().x() * (SELECTED_SCALE - 1)/2)

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
		Entry e;
		e.name = (*it)->getName();
		e.object = *it;

		if((*it)->getTheme()->getElement("system", "header", "image"))
		{
			ImageComponent* logo = new ImageComponent(mWindow);
			logo->setMaxSize(logoSize());
			logo->applyTheme((*it)->getTheme(), "system", "header", ThemeFlags::PATH);
			logo->setPosition((logoSize().x() - logo->getSize().x()) / 2, 0);
			e.data.logo = std::shared_ptr<GuiComponent>(logo);
		}else{
			// no logo in theme; use text
			TextComponent* text = new TextComponent(mWindow);
			text->setFont(Font::get(FONT_SIZE_LARGE));
			text->setText((*it)->getName());
			text->setSize(logoSize());
			text->setCentered(true);
			e.data.logo = std::shared_ptr<GuiComponent>(text);
		}

		e.data.title = nullptr;
		
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

	const float logoSizeX = logoSize().x() + LOGO_PADDING;

	float posMax = logoSizeX * mEntries.size();
	float target = mCursor * logoSizeX;

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
	}, 400);

	setAnimation(anim);
}

void SystemView::render(const Eigen::Affine3f& parentTrans)
{
	if(size() == 0)
		return;

	const float logoSizeX = logoSize().x() + LOGO_PADDING;

	Eigen::Affine3f trans = getTransform() * parentTrans;
	
	// draw background image

	// draw system's stats

	// now for the list elements (logos)
	Eigen::Affine3f logoTrans = trans;

	int logoCount = (int)(mSize.x() / logoSizeX) + 2; // how many logos we need to draw
	int center = (int)(mCamOffset / logoSizeX + 0.5f);

	float xOff = (mSize.x() - logoSize().x())/2 - mCamOffset;
	float yOff = (mSize.y() - logoSize().y())/2;

	// this fixes the case where i != mCursor when wrapping around the end back to zero
	while(center >= (int)mEntries.size())
	{
		center -= mEntries.size();
		xOff += logoSizeX * mEntries.size();
	}

	for(int i = center - logoCount/2; i < center + logoCount/2 + logoCount%2; i++)
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
			if(i == mCursor) //scale our selection up
			{
				// fix the centering because we go by left corner and not center (bleh)
				logoTrans.translation() -= Eigen::Vector3f((comp->getSize().x() / 2) * (SELECTED_SCALE - 1), (comp->getSize().y() / 2) * (SELECTED_SCALE - 1), 0);
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
