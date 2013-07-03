#ifndef _FONT_H_
#define _FONT_H_

#include <string>
#include "platform.h"
#include GLHEADER
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Vector2.h"

class TextCache;

//A TrueType Font renderer that uses FreeType and OpenGL.
//The library is automatically initialized when it's needed.
class Font
{
public:
	static void initLibrary();

	Font(std::string path, int size);
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

	TextCache* buildTextCache(const std::string& text, int offsetX, int offsetY, unsigned int color);
	void renderTextCache(TextCache* cache);

	//Create a TextCache, render with it, then delete it.  Best used for short text or text that changes frequently.
	void drawText(std::string text, int startx, int starty, int color);
	void sizeText(std::string text, int* w, int* h); //Sets the width and height of a given string to supplied pointers. A dimension is skipped if its pointer is NULL.
	int getHeight();

	void init();
	void deinit();

	int getSize();

	static std::string getDefaultPath();
private:
	static int getDpiX();
	static int getDpiY();

	static FT_Library sLibrary;
	static bool libraryInitialized;

	void buildAtlas(); //Builds a "texture atlas," one big OpenGL texture with glyphs 32 to 128.

	int textureWidth; //OpenGL texture width
	int textureHeight; //OpenGL texture height
	int mMaxGlyphHeight;
	float fontScale; //!<Font scale factor. It is > 1.0 if the font would be to big for the texture

	std::string mPath;
	int mSize;
};

class TextCache
{
public:
	struct Vertex
	{
		Vector2<GLfloat> pos;
		Vector2<GLfloat> tex;
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
