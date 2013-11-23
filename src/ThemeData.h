#pragma once

#include <memory>
#include <map>
#include <string>
#include "resources/Font.h"
#include "resources/TextureResource.h"
#include "Renderer.h"
#include "AudioManager.h"
#include "Sound.h"

struct FontDef
{
	FontDef() {}
	FontDef(float sz, const std::string& p) : path(p), size(sz) {}

	std::string path;
	float size;

	inline const std::shared_ptr<Font>& get() const { if(!font) font = Font::get((int)(size * Renderer::getScreenHeight()), path); return font; }

private:
	mutable std::shared_ptr<Font> font;
};

struct ImageDef
{
	ImageDef() {}
	ImageDef(const std::string& p, bool t) : path(p), tile(t) {}

	std::string path;
	bool tile;

	inline const std::shared_ptr<TextureResource>& getTexture() const { if(!texture && !path.empty()) texture = TextureResource::get(path); return texture; }

private:
	mutable std::shared_ptr<TextureResource> texture;
};

struct SoundDef
{
	SoundDef() {}
	SoundDef(const std::string& p) : path(p) { sound = std::shared_ptr<Sound>(new Sound(path)); AudioManager::getInstance()->registerSound(sound); }

	std::string path;

	inline const std::shared_ptr<Sound>& get() const { return sound; }

private:
	std::shared_ptr<Sound> sound;
};

class ThemeData
{
public:
	static const std::shared_ptr<ThemeData>& getDefault();

	ThemeData();

	void setDefaults();
	void loadFile(const std::string& path);

	inline const FontDef& getFontDef(const std::string& identifier) const { return mFontMap.at(identifier); }
	inline std::shared_ptr<Font> getFont(const std::string& identifier) const { return mFontMap.at(identifier).get(); }
	inline const ImageDef& getImage(const std::string& identifier) const { return mImageMap.at(identifier); }
	inline unsigned int getColor(const std::string& identifier) const { return mColorMap.at(identifier); }
	void playSound(const std::string& identifier) const;

	inline void setFont(const std::string& identifier, FontDef def) { mFontMap[identifier] = def; }
	inline void setColor(const std::string& identifier, unsigned int color) { mColorMap[identifier] = color; }

private:
	static std::map<std::string, ImageDef> sDefaultImages;
	static std::map<std::string, unsigned int> sDefaultColors;
	static std::map<std::string, FontDef > sDefaultFonts;
	static std::map<std::string, SoundDef> sDefaultSounds;

	std::map<std::string, ImageDef> mImageMap;
	std::map<std::string, unsigned int> mColorMap;
	std::map<std::string, FontDef > mFontMap;
	std::map< std::string, SoundDef > mSoundMap;
};
