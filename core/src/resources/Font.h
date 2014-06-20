#pragma once

#include <string>
#include "platform.h"
#include GLHEADER
#include <ft2build.h>
#include FT_FREETYPE_H
#include <Eigen/Dense>
#include "resources/ResourceManager.h"
#include "ThemeData.h"

class TextCache;

#define FONT_SIZE_SMALL ((unsigned int)(0.035f * Renderer::getScreenHeight()))
#define FONT_SIZE_MEDIUM ((unsigned int)(0.045f * Renderer::getScreenHeight()))
#define FONT_SIZE_LARGE ((unsigned int)(0.085f * Renderer::getScreenHeight()))

#define FONT_PATH_LIGHT ":/opensans_hebrew_condensed_light.ttf"
#define FONT_PATH_REGULAR ":/opensans_hebrew_condensed_regular.ttf"

enum Alignment
{
	ALIGN_LEFT,
	ALIGN_CENTER, // centers both horizontally and vertically
	ALIGN_RIGHT
};

//A TrueType Font renderer that uses FreeType and OpenGL.
//The library is automatically initialized when it's needed.
class Font : public IReloadable
{
public:
	static void initLibrary();

	static std::shared_ptr<Font> get(int size, const std::string& path = getDefaultPath());

	virtual ~Font();

	Eigen::Vector2f sizeText(std::string text, float lineSpacing = 1.5f) const; // Returns the expected size of a string when rendered.  Extra spacing is applied to the Y axis.
	TextCache* buildTextCache(const std::string& text, float offsetX, float offsetY, unsigned int color);
	TextCache* buildTextCache(const std::string& text, Eigen::Vector2f offset, unsigned int color, float xLen, Alignment alignment = ALIGN_LEFT, float lineSpacing = 1.5f);
	void renderTextCache(TextCache* cache);
	
	std::string wrapText(std::string text, float xLen) const; // Inserts newlines into text to make it wrap properly.
	Eigen::Vector2f sizeWrappedText(std::string text, float xLen, float lineSpacing = 1.5f) const; // Returns the expected size of a string after wrapping is applied.
	Eigen::Vector2f getWrappedTextCursorOffset(std::string text, float xLen, int cursor, float lineSpacing = 1.5f) const; // Returns the position of of the cursor after moving "cursor" characters.

	float getHeight(float lineSpacing = 1.5f) const;
	float getLetterHeight() const;

	void unload(std::shared_ptr<ResourceManager>& rm) override;
	void reload(std::shared_ptr<ResourceManager>& rm) override;

	int getSize() const;
	inline const std::string& getPath() const { return mPath; }

	inline static const char* getDefaultPath() { return FONT_PATH_REGULAR; }

	static std::shared_ptr<Font> getFromTheme(const ThemeData::ThemeElement* elem, unsigned int properties, const std::shared_ptr<Font>& orig);

	size_t getMemUsage() const; // returns an approximation of VRAM used by this font's texture (in bytes)
	static size_t getTotalMemUsage(); // returns an approximation of total VRAM used by font textures (in bytes)

private:
	static FT_Library sLibrary;
	static std::map< std::pair<std::string, int>, std::weak_ptr<Font> > sFontMap;

	Font(int size, const std::string& path);

	void init(ResourceData data);
	void deinit();

	void buildAtlas(ResourceData data); //Builds a "texture atlas," one big OpenGL texture with glyphs 32 to 128.

	//contains sizing information for every glyph.
	struct CharData
	{
		int texX;
		int texY;
		int texW;
		int texH;

		float advX; //!<The horizontal distance to advance to the next character after this one
		float advY; //!<The vertical distance to advance to the next character after this one

		float bearingX; //!<The horizontal distance from the cursor to the start of the character
		float bearingY; //!<The vertical distance from the cursor to the start of the character
	};

	CharData mCharData[128];
	
	GLuint mTextureID;

	int mTextureWidth; //OpenGL texture width
	int mTextureHeight; //OpenGL texture height
	int mMaxGlyphHeight;
	float mFontScale; //!<Font scale factor. It is > 1.0 if the font would be to big for the texture

	int mSize;
	const std::string mPath;

	float getNewlineStartOffset(const std::string& text, const unsigned int& charStart, const float& xLen, const Alignment& alignment);

	friend TextCache;
};

// Used to store a sort of "pre-rendered" string.
// When a TextCache is constructed (Font::buildTextCache()), the vertices and texture coordinates of the string are calculated and stored in the TextCache object.
// Rendering a previously constructed TextCache (Font::renderTextCache) every frame is MUCH faster than rebuilding one every frame.
// Keep in mind you still need the Font object to render a TextCache (as the Font holds the OpenGL texture), and if a Font changes your TextCache may become invalid.
class TextCache
{
public:
	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	};

	struct CacheMetrics
	{
		Eigen::Vector2f size;
	} metrics;

	void setColor(unsigned int color);

	TextCache(int verts, Vertex* v, GLubyte* c, const CacheMetrics& m);
	~TextCache();

	int vertCount;
	Vertex* verts;
	GLubyte* colors;
};
