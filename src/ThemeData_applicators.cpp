#include "ThemeData.h"

#include "Renderer.h"
#include "components/ImageComponent.h"
#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"
#include "Sound.h"
#include "Log.h"

using namespace ThemeFlags;

Eigen::Vector2f getScale(GuiComponent* comp)
{
	if(comp && comp->getParent())
		return comp->getParent()->getSize();
	
	return Eigen::Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
}

ThemeData::ThemeElement* ThemeData::getElement(const std::string& viewName, const std::string& elementName)
{
	auto viewIt = mViews.find(viewName);
	if(viewIt == mViews.end())
		return NULL;

	auto elemIt = viewIt->second.elements.find(elementName);
	if(elemIt == viewIt->second.elements.end())
		return NULL;

	return &elemIt->second;
}

void ThemeData::applyToImage(const std::string& viewName, const std::string& elementName, ImageComponent* image, unsigned int properties)
{
	LOG(LogInfo) << " req image [" << viewName << "." << elementName << "]  (flags: " << properties << ")";

	ThemeElement* elem = getElement(viewName, elementName);
	if(!elem)
	{
		LOG(LogInfo) << "    (missing)";
		return;
	}

	Eigen::Vector2f scale = getScale(image);
	
	if(properties & POSITION && elem->has("pos"))
	{
		Eigen::Vector2f denormalized = elem->get<Eigen::Vector2f>("pos").cwiseProduct(scale);
		image->setPosition(Eigen::Vector3f(denormalized.x(), denormalized.y(), 0));
	}

	if(properties & ThemeFlags::SIZE && elem->has("size"))
		image->setResize(elem->get<Eigen::Vector2f>("size").cwiseProduct(scale), true);

	if(properties & ORIGIN && elem->has("origin"))
		image->setOrigin(elem->get<Eigen::Vector2f>("origin"));

	if(properties & PATH && elem->has("path"))
		image->setImage(elem->get<std::string>("path"));
	
	if(properties & TILING && elem->has("tile"))
		image->setTiling(elem->get<bool>("tile"));
}

void ThemeData::applyToNinePatch(const std::string& viewName, const std::string& elementName, NinePatchComponent* patch, unsigned int properties)
{
	ThemeElement* elem = getElement(viewName, elementName);
	if(!elem)
		return;

	Eigen::Vector2f scale = getScale(patch);
	
	if(properties & POSITION && elem->has("pos"))
	{
		Eigen::Vector2f denormalized = elem->get<Eigen::Vector2f>("pos").cwiseProduct(scale);
		patch->setPosition(Eigen::Vector3f(denormalized.x(), denormalized.y(), 0));
	}

	if(properties & PATH && elem->has("path"))
		patch->setImagePath(elem->get<std::string>("path"));

	if(properties & ThemeFlags::SIZE && elem->has("size"))
		patch->setSize(elem->get<Eigen::Vector2f>("size").cwiseProduct(scale));
}

void ThemeData::applyToText(const std::string& viewName, const std::string& elementName, TextComponent* text, unsigned int properties)
{
	ThemeElement* elem = getElement(viewName, elementName);
	if(!elem)
		return;

	Eigen::Vector2f scale = getScale(text);

	if(properties & POSITION && elem->has("pos"))
	{
		Eigen::Vector2f denormalized = elem->get<Eigen::Vector2f>("pos").cwiseProduct(scale);
		text->setPosition(Eigen::Vector3f(denormalized.x(), denormalized.y(), 0));
	}

	if(properties & ThemeFlags::SIZE && elem->has("size"))
		text->setSize(elem->get<Eigen::Vector2f>("size").cwiseProduct(scale));

	if(properties & COLOR && elem->has("color"))
		text->setColor(elem->get<unsigned int>("color"));

	if(properties & CENTER && elem->has("center"))
		text->setCentered(elem->get<bool>("center"));

	if(properties & TEXT && elem->has("text"))
		text->setText(elem->get<std::string>("text"));

	// TODO - fonts
}

void ThemeData::playSound(const std::string& elementName)
{
	ThemeElement* elem = getElement("common", elementName);
	if(!elem)
		return;

	if(elem->has("path"))
	{
		const std::string path = elem->get<std::string>("path");
		auto cacheIt = mSoundCache.find(path);
		if(cacheIt != mSoundCache.end())
		{
			cacheIt->second->play();
			return;
		}
		
		std::shared_ptr<Sound> sound = std::shared_ptr<Sound>(new Sound(path));
		sound->play();
		mSoundCache[path] = sound;
	}
}
