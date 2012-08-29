//A truetype font loader and renderer, using FreeType 2 and OpenGL.

#ifndef _FONT_H_
#define _FONT_H_

#include <string>
#include <GLES/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

class Font
{
public:
	static void initLibrary();

	Font(std::string path, int size);
	~Font();

	FT_Face face;

	struct charPosData {
		int texX;
		int texY;
		int texW;
		int texH;

		int advX;
		int advY;

		int bearingY;
	};

	charPosData charData[128];

	GLuint textureID;

	void drawText(std::string text, int startx, int starty, int color);
	void sizeText(std::string text, int* w, int* h);
private:

	static int getDpiX();
	static int getDpiY();
	static FT_Library sLibrary;

	void buildAtlas();

	int textureWidth;
	int textureHeight;
	int mMaxGlyphHeight;
};

#endif
