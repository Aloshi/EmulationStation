#pragma once

#include "../GuiComponent.h"

class ImageComponent;
class TextureResource;
class TextCache;
class Font;

class HelpComponent : public GuiComponent
{
public:
	HelpComponent(Window* window);

	void clearPrompts();
	void addPrompt(const char* icon, const char* text);

	void render(const Eigen::Affine3f& parent) override;

private:
	std::shared_ptr<Font> getFont() const;
	std::shared_ptr<TextureResource> getIconTexture(const char* name);
	
	std::map< std::string, std::shared_ptr<TextureResource> > mIconCache;

	struct Prompt
	{
		std::shared_ptr<ImageComponent> icon;
		std::shared_ptr<TextCache> textCache;
	};

	std::vector<Prompt> mPrompts;
};
