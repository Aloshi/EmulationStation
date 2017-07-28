#include "views/SystemView.h"
#include "SystemData.h"
#include "Renderer.h"
#include "Log.h"
#include "Window.h"
#include "views/ViewController.h"
#include "animations/LambdaAnimation.h"
#include "PowerSaver.h"
#include "SystemData.h"
#include "Settings.h"
#include "Util.h"

// buffer values for scrolling velocity (left, stopped, right)
const int logoBuffersLeft[] = { -5, -2, -1 };
const int logoBuffersRight[] = { 1, 2, 5 };

SystemView::SystemView(Window* window) : IList<SystemViewData, SystemData*>(window, LIST_SCROLL_STYLE_SLOW, LIST_ALWAYS_LOOP),
										 mViewNeedsReload(true),
										 mSystemInfo(window, "SYSTEM INFO", Font::get(FONT_SIZE_SMALL), 0x33333300, ALIGN_CENTER)
{
	mCamOffset = 0;
	mExtrasCamOffset = 0;
	mExtrasFadeOpacity = 0.0f;

	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	populate();
}

void SystemView::populate()
{
	mEntries.clear();

	for(auto it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); it++)
	{
		const std::shared_ptr<ThemeData>& theme = (*it)->getTheme();

		if(mViewNeedsReload)
			getViewElements(theme);

		Entry e;
		e.name = (*it)->getName();
		e.object = *it;

		// make logo
		if(theme->getElement("system", "logo", "image"))
		{
			std::string path = theme->getElement("system", "logo", "image")->get<std::string>("path");

			if(!path.empty() && ResourceManager::getInstance()->fileExists(path))
			{
				ImageComponent* logo = new ImageComponent(mWindow, false, false);
				logo->setMaxSize(Eigen::Vector2f(mCarousel.logoSize.x(), mCarousel.logoSize.y()));
				logo->applyTheme((*it)->getTheme(), "system", "logo", ThemeFlags::PATH | ThemeFlags::COLOR);
				logo->setPosition((mCarousel.logoSize.x() - logo->getSize().x()) / 2,
					(mCarousel.logoSize.y() - logo->getSize().y()) / 2); // center
				e.data.logo = std::shared_ptr<GuiComponent>(logo);

				ImageComponent* logoSelected = new ImageComponent(mWindow, false, false);
				logoSelected->setMaxSize(Eigen::Vector2f(mCarousel.logoSize.x() * mCarousel.logoScale, mCarousel.logoSize.y() * mCarousel.logoScale));
				logoSelected->applyTheme((*it)->getTheme(), "system", "logo", ThemeFlags::PATH | ThemeFlags::COLOR);
				logoSelected->setPosition((mCarousel.logoSize.x() - logoSelected->getSize().x()) / 2,
					(mCarousel.logoSize.y() - logoSelected->getSize().y()) / 2); // center
				e.data.logoSelected = std::shared_ptr<GuiComponent>(logoSelected);
			}
		}
		if (!e.data.logo)
		{
			// no logo in theme; use text
			TextComponent* text = new TextComponent(mWindow,
				(*it)->getName(),
				Font::get(FONT_SIZE_LARGE),
				0x000000FF,
				ALIGN_CENTER);
			text->setSize(mCarousel.logoSize);
			text->applyTheme((*it)->getTheme(), "system", "logoText", ThemeFlags::FONT_PATH | ThemeFlags::COLOR | ThemeFlags::FORCE_UPPERCASE);
			e.data.logo = std::shared_ptr<GuiComponent>(text);

			TextComponent* textSelected = new TextComponent(mWindow, 
				(*it)->getName(), 
				Font::get((int)(FONT_SIZE_LARGE * mCarousel.logoScale)),
				0x000000FF,
				ALIGN_CENTER);
			textSelected->setSize(mCarousel.logoSize);
			textSelected->applyTheme((*it)->getTheme(), "system", "logoText", ThemeFlags::FONT_PATH | ThemeFlags::COLOR | ThemeFlags::FORCE_UPPERCASE);
			e.data.logoSelected = std::shared_ptr<GuiComponent>(textSelected);
		}

		// delete any existing extras
		for (auto extra : e.data.backgroundExtras)
			delete extra;
		e.data.backgroundExtras.clear();

		// make background extras
		e.data.backgroundExtras = ThemeData::makeExtras((*it)->getTheme(), "system", mWindow);

		// sort the extras by z-index
		std:stable_sort(e.data.backgroundExtras.begin(), e.data.backgroundExtras.end(),  [](GuiComponent* a, GuiComponent* b) {
			return b->getZIndex() > a->getZIndex();
		});

		this->add(e);
	}
}

void SystemView::goToSystem(SystemData* system, bool animate)
{
	setCursor(system);

	if(!animate)
		finishAnimation(0);
}

bool SystemView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->getDeviceId() == DEVICE_KEYBOARD && input.value && input.id == SDLK_r && SDL_GetModState() & KMOD_LCTRL && Settings::getInstance()->getBool("Debug"))
		{
			LOG(LogInfo) << " Reloading all";
			ViewController::get()->reloadAll();
			return true;
		}

		switch (mCarousel.type)
		{
		case VERTICAL:
			if (config->isMappedTo("up", input))
			{
				listInput(-1);
				return true;
			}
			if (config->isMappedTo("down", input))
			{
				listInput(1);
				return true;
			}
			break;
		case HORIZONTAL:
		default:
			if (config->isMappedTo("left", input))
			{
				listInput(-1);
				return true;
			}
			if (config->isMappedTo("right", input))
			{
				listInput(1);
				return true;
			}
			break;
		}

		if(config->isMappedTo("a", input))
		{
			stopScrolling();
			ViewController::get()->goToGameList(getSelected());
			return true;
		}
		if (config->isMappedTo("x", input))
		{
			// get random system
			// go to system
			setCursor(SystemData::getRandomSystem());
			//ViewController::get()->goToRandomGame();
			return true;
		}
	}else{
		if(config->isMappedTo("left", input) ||
			config->isMappedTo("right", input) ||
			config->isMappedTo("up", input) ||
			config->isMappedTo("down", input))
			listInput(0);
		if(config->isMappedTo("select", input) && Settings::getInstance()->getBool("ScreenSaverControls"))
		{
			mWindow->startScreenSaver();
			mWindow->renderScreenSaver();
			return true;
		}
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
	// update help style
	updateHelpPrompts();

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


	// animate mSystemInfo's opacity (fade out, wait, fade back in)

	cancelAnimation(1);
	cancelAnimation(2);

	std::string transition_style = Settings::getInstance()->getString("TransitionStyle");
	bool goFast = transition_style == "instant";
	const float infoStartOpacity = mSystemInfo.getOpacity() / 255.f;

	Animation* infoFadeOut = new LambdaAnimation(
		[infoStartOpacity, this] (float t)
	{
		mSystemInfo.setOpacity((unsigned char)(lerp<float>(infoStartOpacity, 0.f, t) * 255));
	}, (int)(infoStartOpacity * (goFast ? 10 : 150)));

	unsigned int gameCount = getSelected()->getDisplayedGameCount();

	// also change the text after we've fully faded out
	setAnimation(infoFadeOut, 0, [this, gameCount] {
		std::stringstream ss;

		if (!getSelected()->isGameSystem())
			ss << "CONFIGURATION";
		else
			ss << gameCount << " GAMES AVAILABLE";

		mSystemInfo.setText(ss.str());
	}, false, 1);

	Animation* infoFadeIn = new LambdaAnimation(
		[this](float t)
	{
		mSystemInfo.setOpacity((unsigned char)(lerp<float>(0.f, 1.f, t) * 255));
	}, goFast ? 10 : 300);

	// wait 600ms to fade in
	setAnimation(infoFadeIn, goFast ? 0 : 2000, nullptr, false, 2);

	// no need to animate transition, we're not going anywhere (probably mEntries.size() == 1)
	if(endPos == mCamOffset && endPos == mExtrasCamOffset)
		return;

	Animation* anim;
	bool move_carousel = Settings::getInstance()->getBool("MoveCarousel");
	if(transition_style == "fade")
	{
		float startExtrasFade = mExtrasFadeOpacity;
		anim = new LambdaAnimation(
			[this, startExtrasFade, startPos, endPos, posMax, move_carousel](float t)
		{
			t -= 1;
			float f = lerp<float>(startPos, endPos, t*t*t + 1);
			if(f < 0)
				f += posMax;
			if(f >= posMax)
				f -= posMax;

			this->mCamOffset = move_carousel ? f : endPos;

			t += 1;
			if(t < 0.3f)
				this->mExtrasFadeOpacity = lerp<float>(0.0f, 1.0f, t / 0.3f + startExtrasFade);
			else if(t < 0.7f)
				this->mExtrasFadeOpacity = 1.0f;
			else
				this->mExtrasFadeOpacity = lerp<float>(1.0f, 0.0f, (t - 0.7f) / 0.3f);

			if(t > 0.5f)
				this->mExtrasCamOffset = endPos;

		}, 500);
	} else if (transition_style == "slide") {
		// slide
		anim = new LambdaAnimation(
			[this, startPos, endPos, posMax, move_carousel](float t)
		{
			t -= 1;
			float f = lerp<float>(startPos, endPos, t*t*t + 1);
			if(f < 0)
				f += posMax;
			if(f >= posMax)
				f -= posMax;

			this->mCamOffset = move_carousel ? f : endPos;
			this->mExtrasCamOffset = f;
		}, 500);
	} else {
		// instant
		anim = new LambdaAnimation(
			[this, startPos, endPos, posMax, move_carousel ](float t)
		{
			t -= 1;
			float f = lerp<float>(startPos, endPos, t*t*t + 1);
			if(f < 0)
				f += posMax;
			if(f >= posMax)
				f -= posMax;

			this->mCamOffset = move_carousel ? f : endPos;
			this->mExtrasCamOffset = endPos;
		}, move_carousel ? 500 : 1);
	}


	setAnimation(anim, 0, nullptr, false, 0);
}

void SystemView::render(const Eigen::Affine3f& parentTrans)
{
	if(size() == 0)
		return;  // nothing to render

	Eigen::Affine3f trans = getTransform() * parentTrans;

	auto systemInfoZIndex = mSystemInfo.getZIndex();
	auto minMax = std::minmax(mCarousel.zIndex, systemInfoZIndex);

	renderExtras(trans, INT16_MIN, minMax.first);
	renderFade(trans);

	if (mCarousel.zIndex > mSystemInfo.getZIndex()) {
		renderInfoBar(trans);
	} else {
		renderCarousel(trans);
	}

	renderExtras(trans, minMax.first, minMax.second);

	if (mCarousel.zIndex > mSystemInfo.getZIndex()) {
		renderCarousel(trans);
	} else {
		renderInfoBar(trans);
	}

	renderExtras(trans, minMax.second, INT16_MAX);
}

std::vector<HelpPrompt> SystemView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	if (mCarousel.type == VERTICAL)
		prompts.push_back(HelpPrompt("up/down", "choose"));
	else
		prompts.push_back(HelpPrompt("left/right", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("x", "random"));

	if (Settings::getInstance()->getBool("ScreenSaverControls"))
		prompts.push_back(HelpPrompt("select", "launch screensaver"));

	return prompts;
}

HelpStyle SystemView::getHelpStyle()
{
	HelpStyle style;
	style.applyTheme(mEntries.at(mCursor).object->getTheme(), "system");
	return style;
}

void  SystemView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	LOG(LogDebug) << "SystemView::onThemeChanged()";
	mViewNeedsReload = true;
	populate();
}

//  Get the ThemeElements that make up the SystemView.
void  SystemView::getViewElements(const std::shared_ptr<ThemeData>& theme)
{
	LOG(LogDebug) << "SystemView::getViewElements()";

	getDefaultElements();

	if (!theme->hasView("system"))
		return;

	const ThemeData::ThemeElement* carouselElem = theme->getElement("system", "systemcarousel", "carousel");
	if (carouselElem)
		getCarouselFromTheme(carouselElem);

	const ThemeData::ThemeElement* sysInfoElem = theme->getElement("system", "systemInfo", "text");
	if (sysInfoElem)
		mSystemInfo.applyTheme(theme, "system", "systemInfo", ThemeFlags::ALL);

	mViewNeedsReload = false;
}

//  Render system carousel
void SystemView::renderCarousel(const Eigen::Affine3f& trans)
{
	Eigen::Vector2i clipPos((int)mCarousel.pos.x(), (int)mCarousel.pos.y());
	Eigen::Vector2i clipSize((int)mCarousel.size.x(), (int)mCarousel.size.y());

	Renderer::pushClipRect(clipPos, clipSize);

	// background box behind logos
	Renderer::setMatrix(trans);
	Renderer::drawRect(mCarousel.pos.x(), mCarousel.pos.y(), mCarousel.size.x(), mCarousel.size.y(), mCarousel.color);

	// draw logos
	Eigen::Vector2f logoSpacing(0.0, 0.0); // NB: logoSpacing will include the size of the logo itself as well!
	float xOff = 0.0;
	float yOff = 0.0;

	switch (mCarousel.type)
	{
		case VERTICAL:
			logoSpacing[1] = ((mCarousel.size.y() - (mCarousel.logoSize.y() * mCarousel.maxLogoCount)) / (mCarousel.maxLogoCount)) + mCarousel.logoSize.y();
			xOff = mCarousel.pos.x() + (mCarousel.size.x() / 2) - (mCarousel.logoSize.x() / 2);
			yOff = mCarousel.pos.y() + (mCarousel.size.y() - mCarousel.logoSize.y()) / 2 - (mCamOffset * logoSpacing[1]);
			break;
		case HORIZONTAL:
		default:
			logoSpacing[0] = ((mCarousel.size.x() - (mCarousel.logoSize.x() * mCarousel.maxLogoCount)) / (mCarousel.maxLogoCount)) + mCarousel.logoSize.x();
			xOff = mCarousel.pos.x() + (mCarousel.size.x() - mCarousel.logoSize.x()) / 2 - (mCamOffset * logoSpacing[0]);
			yOff = mCarousel.pos.y() + (mCarousel.size.y() / 2) - (mCarousel.logoSize.y() / 2);
			break;
	}

	Eigen::Affine3f logoTrans = trans;
	int center = (int)(mCamOffset);
	int logoCount = std::min(mCarousel.maxLogoCount, (int)mEntries.size());

	// Adding texture loading buffers depending on scrolling speed and status
	int bufferIndex = getScrollingVelocity() + 1;

	for (int i = center - logoCount / 2 + logoBuffersLeft[bufferIndex]; i <= center + logoCount / 2 + logoBuffersRight[bufferIndex]; i++)
	{
		int index = i;
		while (index < 0)
			index += mEntries.size();
		while (index >= (int)mEntries.size())
			index -= mEntries.size();

		logoTrans.translation() = trans.translation() + Eigen::Vector3f(i * logoSpacing[0] + xOff, i * logoSpacing [1] + yOff, 0);

		if (index == mCursor) //Selected System
		{
			const std::shared_ptr<GuiComponent>& comp = mEntries.at(index).data.logoSelected;
			comp->setOpacity(0xFF);
			comp->render(logoTrans);
		}
		else { // not selected systems
			const std::shared_ptr<GuiComponent>& comp = mEntries.at(index).data.logo;
			comp->setOpacity(0x80);
			comp->render(logoTrans);
		}
	}
	Renderer::popClipRect();
}

void SystemView::renderInfoBar(const Eigen::Affine3f& trans)
{
	Renderer::setMatrix(trans);
	mSystemInfo.render(trans);
}

// Draw background extras
void SystemView::renderExtras(const Eigen::Affine3f& trans, float lower, float upper)
{
	int extrasCenter = (int)mExtrasCamOffset;

	// Adding texture loading buffers depending on scrolling speed and status
	int bufferIndex = getScrollingVelocity() + 1;

	Renderer::pushClipRect(Eigen::Vector2i(0, 0), mSize.cast<int>());

	for (int i = extrasCenter + logoBuffersLeft[bufferIndex]; i <= extrasCenter + logoBuffersRight[bufferIndex]; i++)
	{
		int index = i;
		while (index < 0)
			index += mEntries.size();
		while (index >= (int)mEntries.size())
			index -= mEntries.size();

		Eigen::Affine3f extrasTrans = trans;
		if (mCarousel.type == HORIZONTAL)
			extrasTrans.translate(Eigen::Vector3f((i - mExtrasCamOffset) * mSize.x(), 0, 0));
		else
			extrasTrans.translate(Eigen::Vector3f(0, (i - mExtrasCamOffset) * mSize.y(), 0));

		Renderer::pushClipRect(Eigen::Vector2i(extrasTrans.translation()[0], extrasTrans.translation()[1]), mSize.cast<int>());
		SystemViewData data = mEntries.at(index).data;
		for(unsigned int j = 0; j < data.backgroundExtras.size(); j++)
		{
			GuiComponent* extra = data.backgroundExtras[j];
			if (extra->getZIndex() >= lower && extra->getZIndex() < upper) {
				extra->render(extrasTrans);
			}
		}
		Renderer::popClipRect();
	}
	Renderer::popClipRect();
}

void SystemView::renderFade(const Eigen::Affine3f& trans)
{
	// fade extras if necessary
	if (mExtrasFadeOpacity)
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0x00000000 | (unsigned char)(mExtrasFadeOpacity * 255));
	}
}

// Populate the system carousel with the legacy values
void  SystemView::getDefaultElements(void)
{
	// Carousel
	mCarousel.type = HORIZONTAL;
	mCarousel.size.x() = mSize.x();
	mCarousel.size.y() = 0.2325f * mSize.y();
	mCarousel.pos.x() = 0.0f;
	mCarousel.pos.y() = 0.5f * (mSize.y() - mCarousel.size.y());
	mCarousel.color = 0xFFFFFFD8;
	mCarousel.logoScale = 1.2f;
	mCarousel.logoSize.x() = 0.25f * mSize.x();
	mCarousel.logoSize.y() = 0.155f * mSize.y();
	mCarousel.maxLogoCount = 3;
	mCarousel.zIndex = 40;

	// System Info Bar
	mSystemInfo.setSize(mSize.x(), mSystemInfo.getFont()->getLetterHeight()*2.2f);
	mSystemInfo.setPosition(0, (mCarousel.pos.y() + mCarousel.size.y() - 0.2f));
	mSystemInfo.setBackgroundColor(0xDDDDDDD8);
	mSystemInfo.setRenderBackground(true);
	mSystemInfo.setFont(Font::get((int)(0.035f * mSize.y()), Font::getDefaultPath()));
	mSystemInfo.setColor(0x000000FF);
	mSystemInfo.setZIndex(50);
	mSystemInfo.setDefaultZIndex(50);
}

void SystemView::getCarouselFromTheme(const ThemeData::ThemeElement* elem)
{
	if (elem->has("type"))
		mCarousel.type = !(elem->get<std::string>("type").compare("vertical")) ? VERTICAL : HORIZONTAL;
	if (elem->has("size"))
		mCarousel.size = elem->get<Eigen::Vector2f>("size").cwiseProduct(mSize);
	if (elem->has("pos"))
		mCarousel.pos = elem->get<Eigen::Vector2f>("pos").cwiseProduct(mSize);
	if (elem->has("color"))
		mCarousel.color = elem->get<unsigned int>("color");
	if (elem->has("logoScale"))
		mCarousel.logoScale = elem->get<float>("logoScale");
	if (elem->has("logoSize"))
		mCarousel.logoSize = elem->get<Eigen::Vector2f>("logoSize").cwiseProduct(mSize);
	if (elem->has("maxLogoCount"))
		mCarousel.maxLogoCount = std::round(elem->get<float>("maxLogoCount"));
	if (elem->has("zIndex"))
		mCarousel.zIndex = elem->get<float>("zIndex");
}
