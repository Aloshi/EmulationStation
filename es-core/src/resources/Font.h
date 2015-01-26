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

typedef unsigned long UnicodeChar;

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

	Eigen::Vector2f sizeText(std::string text, float lineSpacing = 1.5f); // Returns the expected size of a string when rendered.  Extra spacing is applied to the Y axis.
	TextCache* buildTextCache(const std::string& text, float offsetX, float offsetY, unsigned int color);
	TextCache* buildTextCache(const std::string& text, Eigen::Vector2f offset, unsigned int color, float xLen, Alignment alignment = ALIGN_LEFT, float lineSpacing = 1.5f);
	void renderTextCache(TextCache* cache);
	
	std::string wrapText(std::string text, float xLen); // Inserts newlines into text to make it wrap properly.
	Eigen::Vector2f sizeWrappedText(std::string text, float xLen, float lineSpacing = 1.5f); // Returns the expected size of a string after wrapping is applied.
	Eigen::Vector2f getWrappedTextCursorOffset(std::string text, float xLen, size_t cursor, float lineSpacing = 1.5f); // Returns the position of of the cursor after moving "cursor" characters.

	float getHeight(float lineSpacing = 1.5f) const;
	float getLetterHeight();

	void unload(std::shared_ptr<ResourceManager>& rm) override;
	void reload(std::shared_ptr<ResourceManager>& rm) override;

	int getSize() const;
	inline const std::string& getPath() const { return mPath; }

	inline static const char* getDefaultPath() { return FONT_PATH_REGULAR; }

	static std::shared_ptr<Font> getFromTheme(const ThemeData::ThemeElement* elem, unsigned int properties, const std::shared_ptr<Font>& orig);

	size_t getMemUsage() const; // returns an approximation of VRAM used by this font's texture (in bytes)
	static size_t getTotalMemUsage(); // returns an approximation of total VRAM used by font textures (in bytes)

	// utf8 stuff
	static size_t getNextCursor(const std::string& str, size_t cursor);
	static size_t getPrevCursor(const std::string& str, size_t cursor);
	static size_t moveCursor(const std::string& str, size_t cursor, int moveAmt); // negative moveAmt = move backwards, positive = move forwards
	static UnicodeChar readUnicodeChar(const std::string& str, size_t& cursor); // reads unicode character at cursor AND moves cursor to the next valid unicode char

private:
	static FT_Library sLibrary;
	static std::map< std::pair<std::string, int>, std::weak_ptr<Font> > sFontMap;

	Font(int size, const std::string& path);

	struct FontTexture
	{
		GLuint textureId;
		Eigen::Vector2i textureSize;

		Eigen::Vector2i writePos;
		int rowHeight;

		FontTexture();
		~FontTexture();
		bool findEmpty(const Eigen::Vector2i& size, Eigen::Vector2i& cursor_out);

		// you must call initTexture() after creating a FontTexture to get a textureId
		void initTexture(); // initializes the OpenGL texture according to this FontTexture's settings, updating textureId
		void deinitTexture(); // deinitializes the OpenGL texture if any exists, is automatically called in the destructor
	};

	struct FontFace
	{
		const ResourceData data;
		FT_Face face;

		FontFace(ResourceData&& d, int size);
		virtual ~FontFace();
	};

	void rebuildTextures();
	void unloadTextures();

	std::vector<FontTexture> mTextures;

	void getTextureForNewGlyph(const Eigen::Vector2i& glyphSize, FontTexture*& tex_out, Eigen::Vector2i& cursor_out);

	std::map< unsigned int, std::unique_ptr<FontFace> > mFaceCache;
	FT_Face getFaceForChar(UnicodeChar id);
	void clearFaceCache();

	struct Glyph
	{
		FontTexture* texture;
		
		Eigen::Vector2f texPos;
		Eigen::Vector2f texSize; // in texels!

		Eigen::Vector2f advance;
		Eigen::Vector2f bearing;
	};

	std::map<UnicodeChar, Glyph> mGlyphMap;

	Glyph* getGlyph(UnicodeChar id);

	int mMaxGlyphHeight;
	
	const int mSize;
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
protected:
	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	};

	struct VertexList
	{
		GLuint* textureIdPtr; // this is a pointer because the texture ID can change during deinit/reinit (when launching a game)
		std::vector<Vertex> verts;
		std::vector<GLubyte> colors;
	};

	std::vector<VertexList> vertexLists;

public:
	struct CacheMetrics
	{
		Eigen::Vector2f size;
	} metrics;

	void setColor(unsigned int color);

	friend Font;
};
