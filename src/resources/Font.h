#ifndef _FONT_H_
#define _FONT_H_

#include <string>
#include "../platform.h"
#include GLHEADER
#include <ft2build.h>
#include FT_FREETYPE_H
#include <Eigen/Dense>
#include "ResourceManager.h"
#include "../ThemeData.h"

class TextCache;

#define FONT_SIZE_SMALL ((unsigned int)(0.035f * Renderer::getScreenHeight()))
#define FONT_SIZE_MEDIUM ((unsigned int)(0.045f * Renderer::getScreenHeight()))
#define FONT_SIZE_LARGE ((unsigned int)(0.085f * Renderer::getScreenHeight()))

#define FONT_PATH_LIGHT ":/opensans_hebrew_condensed_light.ttf"
#define FONT_PATH_REGULAR ":/opensans_hebrew_condensed_regular.ttf"

//A TrueType Font renderer that uses FreeType and OpenGL.
//The library is automatically initialized when it's needed.
class Font : public IReloadable
{
public:
	static void initLibrary();

	static std::shared_ptr<Font> get(int size, const std::string& path = getDefaultPath());

	~Font();

	FT_Face face;

	//contains sizing information for every glyph.
	struct charPosData {
		int texX;
		int texY;
		int texW;
		int texH;

		float advX; //!<The horizontal distance to advance to the next character after this one
		float advY; //!<The vertical distance to advance to the next character after this one

		float bearingX; //!<The horizontal distance from the cursor to the start of the character
		float bearingY; //!<The vertical distance from the cursor to the start of the character
	};

	charPosData charData[128];

	GLuint textureID;

	TextCache* buildTextCache(const std::string& text, float offsetX, float offsetY, unsigned int color);
	void renderTextCache(TextCache* cache);

	//Create a TextCache, render with it, then delete it.  Best used for short text or text that changes frequently.
	void drawText(std::string text, const Eigen::Vector2f& offset, unsigned int color);
	Eigen::Vector2f sizeText(std::string text) const; //Sets the width and height of a given string to supplied pointers. A dimension is skipped if its pointer is NULL.
	
	std::string wrapText(std::string text, float xLen) const;

	void drawWrappedText(std::string text, const Eigen::Vector2f& offset, float xLen, unsigned int color);
	Eigen::Vector2f sizeWrappedText(std::string text, float xLen) const;
	Eigen::Vector2f getWrappedTextCursorOffset(std::string text, float xLen, int cursor) const;

	void drawCenteredText(std::string text, float xOffset, float y, unsigned int color);

	float getHeight() const;
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
	static int getDpiX();
	static int getDpiY();

	static FT_Library sLibrary;
	static bool libraryInitialized;

	static std::map< std::pair<std::string, int>, std::weak_ptr<Font> > sFontMap;

	Font(int size, const std::string& path);

	void init(ResourceData data);
	void deinit();

	void buildAtlas(ResourceData data); //Builds a "texture atlas," one big OpenGL texture with glyphs 32 to 128.

	int textureWidth; //OpenGL texture width
	int textureHeight; //OpenGL texture height
	int mMaxGlyphHeight;
	float fontScale; //!<Font scale factor. It is > 1.0 if the font would be to big for the texture

	int mSize;
	const std::string mPath;
};

// Used to store a sort of "pre-rendered" string.
// When a TextCache is constructed (Font::buildTextCache()), the vertices and texture coordinates of the string are calculated and stored in the TextCache object.
// Rendering a TextCache (Font::renderTextCache) every frame is MUCH faster than calling Font::drawText() and its variants.
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

#endif
