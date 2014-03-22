#include "HelpComponent.h"
#include "../Renderer.h"
#include "ImageComponent.h"
#include "../resources/Font.h"
#include "../Settings.h"
#include "../Log.h"
#include <boost/assign.hpp>

static const std::map<std::string, const char*> ICON_PATH_MAP = boost::assign::map_list_of
	("up/down", ":/help/dpad_updown.svg")
	("left/right", ":/help/dpad_leftright.svg")
	("up/down/left/right", ":/help/dpad_all.svg")
	("a", ":/help/button_a.svg")
	("b", ":/help/button_b.svg")
	("x", ":/help/button_x.svg")
	("y", ":/help/button_y.svg")
	("l", ":/help/button_l.svg")
	("r", ":/help/button_r.svg")
	("start", ":/help/button_start.svg")
	("select", ":/help/button_select.svg");

HelpComponent::HelpComponent(Window* window) : GuiComponent(window)
{
}

void HelpComponent::clearPrompts()
{
	mPrompts.clear();
}

void HelpComponent::addPrompt(const char* icon, const char* text)
{
	if(!Settings::getInstance()->getBool("ShowHelpPrompts"))
		return;

	Prompt p;

	std::shared_ptr<Font> font = getFont();

	// make the icon
	p.icon = std::shared_ptr<ImageComponent>(new ImageComponent(mWindow));
	p.icon->setResize(0, (float)FONT_SIZE_SMALL);
	p.icon->setImage(getIconTexture(icon));
	p.icon->setPosition(0.0f, mPrompts.size() ? mPrompts.back().icon->getPosition().y() + mPrompts.back().icon->getSize().y() + 10 : 0);
	p.icon->setOpacity(0xEE);
	
	// make the text
	const float textY = (p.icon->getSize().y() - (float)font->getHeight())/2;
	p.textCache = std::shared_ptr<TextCache>(font->buildTextCache(text, p.icon->getSize().x() + 6, textY, 0x888888EE));

	mPrompts.push_back(p);

	setPosition(0, (float)Renderer::getScreenHeight() - (p.icon->getPosition().y() + p.icon->getSize().y() + 6));
}

std::shared_ptr<Font> HelpComponent::getFont() const
{
	// font size controls icon height
	return Font::get(FONT_SIZE_SMALL);
}

std::shared_ptr<TextureResource> HelpComponent::getIconTexture(const char* name)
{
	auto it = mIconCache.find(name);
	if(it != mIconCache.end())
		return it->second;
	
	auto pathLookup = ICON_PATH_MAP.find(name);
	if(pathLookup == ICON_PATH_MAP.end())
	{
		LOG(LogError) << "Unknown help icon \"" << name << "\"!";
		return nullptr;
	}
	if(!ResourceManager::getInstance()->fileExists(pathLookup->second))
	{
		LOG(LogError) << "Help icon \"" << name << "\" - corresponding image file \"" << pathLookup->second << "\" misisng!";
		return nullptr;
	}

	std::shared_ptr<TextureResource> tex = TextureResource::get(pathLookup->second);
	mIconCache[std::string(name)] = tex;
	return tex;
}

void HelpComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	
	std::shared_ptr<Font> font = getFont();
	for(auto it = mPrompts.begin(); it != mPrompts.end(); it++)
	{
		it->icon->render(trans);
		// we actually depend on it->icon->render to call Renderer::setMatrix to draw at the right Y offset (efficiency!)
		// if for some reason this breaks in the future, it should be equivalent to translating parentTrans by it->icon->getPosition()
		font->renderTextCache(it->textCache.get());
	}
}
