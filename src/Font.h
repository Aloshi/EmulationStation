#ifndef _FONT_H_
#define _FONT_H_

#include <string>
#include "platform.h"
#include GLHEADER
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Eigen/Dense"
#include "resources/ResourceManager.h"

class TextCache;

#define FONT_SIZE_SMALL ((unsigned int)(0.035f * Renderer::getScreenHeight()))
#define FONT_SIZE_MEDIUM ((unsigned int)(0.045f * Renderer::getScreenHeight()))
#define FONT_SIZE_LARGE ((unsigned int)(0.1f * Renderer::getScreenHeight()))

//A TrueType Font renderer that uses FreeType and OpenGL.
//The library is automatically initialized when it's needed.
class Font : public IReloadable
{
public:
	static void initLibrary();

	static std::shared_ptr<Font> get(ResourceManager& rm, const std::string& path, int size);

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
	
	void drawWrappedText(std::string text, const Eigen::Vector2f& offset, float xLen, unsigned int color);
	Eigen::Vector2f sizeWrappedText(std::string text, float xLen) const;

	void drawCenteredText(std::string text, float xOffset, float y, unsigned int color);

	int getHeight() const;

	void unload(const ResourceManager& rm) override;
	void reload(const ResourceManager& rm) override;

	int getSize() const;

	static std::string getDefaultPath();
private:
	static int getDpiX();
	static int getDpiY();

	static FT_Library sLibrary;
	static bool libraryInitialized;

	static std::map< std::pair<std::string, int>, std::weak_ptr<Font> > sFontMap;

	Font(const ResourceManager& rm, const std::string& path, int size);

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

class TextCache
{
public:
	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	};

	void setColor(unsigned int color);

	TextCache(int verts, Vertex* v, GLubyte* c, Font* f);
	~TextCache();

	const int vertCount;
	const Vertex* verts;
	const GLubyte* colors;
	const Font* sourceFont;
};

#endif
