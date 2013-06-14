#include "Font.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include "Renderer.h"
#include <boost/filesystem.hpp>
#include "Log.h"

FT_Library Font::sLibrary;
bool Font::libraryInitialized = false;

int Font::getDpiX() { return 96; }
int Font::getDpiY() { return 96; }

int Font::getSize() { return mSize; }

std::string Font::getDefaultPath()
{
	const int fontCount = 4;

#ifdef WIN32
	std::string fonts[] = {"DejaVuSerif.ttf",
		"Arial.ttf",
		"Verdana.ttf",
		"Tahoma.ttf" };

	//build full font path
	TCHAR winDir[MAX_PATH];
	GetWindowsDirectory(winDir, MAX_PATH);
#ifdef UNICODE
	char winDirChar[MAX_PATH*2];
	char DefChar = ' ';
    WideCharToMultiByte(CP_ACP, 0, winDir, -1, winDirChar, MAX_PATH, &DefChar, NULL);
	std::string fontPath(winDirChar);
#else
	std::string fontPath(winDir);
#endif
	fontPath += "\\Fonts\\";
	//prepend to font file names
	for(int i = 0; i < fontCount; i++)
	{
		fonts[i] = fontPath + fonts[i];
	}
#else
	std::string fonts[] = {"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSerif.ttf",
		"/usr/share/fonts/TTF/DejaVuSerif.ttf",
		"/usr/share/fonts/dejavu/DejaVuSerif.ttf",
		"font.ttf" };
#endif

	for(int i = 0; i < fontCount; i++)
	{
		if(boost::filesystem::exists(fonts[i]))
			return fonts[i];
	}

	LOG(LogError) << "Error - could not find a font!";

	return "";
}

void Font::initLibrary()
{
	if(FT_Init_FreeType(&sLibrary))
	{
		LOG(LogError) << "Error initializing FreeType!";
	}else{
		libraryInitialized = true;
	}
}

Font::Font(std::string path, int size)
	: fontScale(1.0f)
{
	mPath = path;
	mSize = size;

	init();
}

void Font::init()
{
	if(!libraryInitialized)
		initLibrary();

	mMaxGlyphHeight = 0;

	buildAtlas();
}

void Font::deinit()
{
	if(textureID)
		glDeleteTextures(1, &textureID);
}

void Font::buildAtlas()
{
	if(FT_New_Face(sLibrary, mPath.c_str(), 0, &face))
	{
		LOG(LogError) << "Error creating font face! (path: " << mPath.c_str();
		return;
	}

	//FT_Set_Char_Size(face, 0, size * 64, getDpiX(), getDpiY());
	FT_Set_Pixel_Sizes(face, 0, mSize);

	//find the size we should use
	FT_GlyphSlot g = face->glyph;
	int w = 0;
	int h = 0;

	/*for(int i = 32; i < 128; i++)
	{
		if(FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			fprintf(stderr, "Loading character %c failed!\n", i);
			continue;
		}

		w += g->bitmap.width;
		h = std::max(h, g->bitmap.rows);
	}*/

	//the max size (GL_MAX_TEXTURE_SIZE) is like 3300
	w = 2048;
	h = 512;

	textureWidth = w;
	textureHeight = h;

	//create the texture
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);

	//copy the glyphs into the texture
	int x = 0;
	int y = 0;
	int maxHeight = 0;
	for(int i = 32; i < 128; i++)
	{
		if(FT_Load_Char(face, i, FT_LOAD_RENDER))
			continue;

		 //prints rendered texture to the console
		/*std::cout << "uploading at x: " << x << ", w: " << g->bitmap.width << " h: " << g->bitmap.rows << "\n";

		for(int k = 0; k < g->bitmap.rows; k++)
		{
			for(int j = 0; j < g->bitmap.width; j++)
			{
				if(g->bitmap.buffer[g->bitmap.width * k + j])
					std::cout << ".";
				else
					std::cout << " ";
			}
			std::cout << "\n";
		}*/

		if(x + g->bitmap.width >= textureWidth)
		{
			x = 0;
			y += maxHeight + 1; //leave one pixel of space between glyphs
			maxHeight = 0;
		}

		if(g->bitmap.rows > maxHeight)
			maxHeight = g->bitmap.rows;

		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);


		charData[i].texX = x;
		charData[i].texY = y;
		charData[i].texW = g->bitmap.width;
		charData[i].texH = g->bitmap.rows;
		charData[i].advX = g->metrics.horiAdvance / 64.0f;
		charData[i].advY = g->metrics.vertAdvance / 64.0f;
		charData[i].bearingY = g->metrics.horiBearingY / 64.0f;

		if(charData[i].texH > mMaxGlyphHeight)
			mMaxGlyphHeight = charData[i].texH;

		x += g->bitmap.width + 1; //leave one pixel of space between glyphs
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face);

	if((y + maxHeight) >= textureHeight)
	{
		//failed to create a proper font texture
		LOG(LogError) << "Error - font with size " << mSize << " exceeded texture size! Trying again...";
		//try a 3/4th smaller size and redo initialization
		fontScale *= 1.25f;
		mSize = (int)(mSize * (1.0f / fontScale));
		deinit();
		init();
	}
	else {
		LOG(LogInfo) << "Created font with size " << mSize << std::endl;
	}

	//std::cout << "generated texture \"" << textureID << "\" (w: " << w << " h: " << h << ")" << std::endl;
}

Font::~Font()
{
	if(textureID)
		glDeleteTextures(1, &textureID);
}



//why these aren't in an array:
//openGL reads these in the order they are in memory
//if I use an array, it will be 4 x values then 4 y values
//it'll read XX, XX, YY instead of XY, XY, XY
//...
//that was the thinking at the time and honestly an array would have been smarter wow I'm dumb
struct point {
	GLfloat pos0x;
	GLfloat pos0y;

	GLfloat pos1x;
	GLfloat pos1y;

	GLfloat pos2x;
	GLfloat pos2y;
};

struct tex {
	GLfloat tex0x;
	GLfloat tex0y;

	GLfloat tex1x;
	GLfloat tex1y;

	GLfloat tex2x;
	GLfloat tex2y;
};

void Font::drawText(std::string text, int startx, int starty, int color)
{
	if(!textureID)
	{
		LOG(LogError) << "Error - tried to draw with Font that has no texture loaded!";
		return;
	}

	int pointCount = text.length() * 2;
	point* points = new point[pointCount];
	tex* texs = new tex[pointCount];
	GLubyte* colors = new GLubyte[pointCount * 3 * 4];

	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//texture atlas width/height
	float tw = (float)textureWidth;
	float th = (float)textureHeight;

	float x = (float)startx;
	float y = starty + mMaxGlyphHeight * 1.1f * fontScale; //padding (another 0.5% is added to the bottom through the sizeText function)

	int p = 0;
	for(int i = 0; p < pointCount; i++, p++)
	{
		unsigned char letter = text[i];

		if(letter < 32 || letter >= 128)
			letter = 127; //print [X] if character is not standard ASCII

		points[p].pos0x = x;
		points[p].pos0y = y + (charData[letter].texH - charData[letter].bearingY) * fontScale;
		points[p].pos1x = x + charData[letter].texW * fontScale;
		points[p].pos1y = y - charData[letter].bearingY * fontScale;
		points[p].pos2x = x;
		points[p].pos2y = points[p].pos1y;

		texs[p].tex0x = charData[letter].texX / tw;
		texs[p].tex0y = (charData[letter].texY + charData[letter].texH) / th;
		texs[p].tex1x = (charData[letter].texX + charData[letter].texW) / tw;
		texs[p].tex1y = charData[letter].texY / th;
		texs[p].tex2x = texs[p].tex0x;
		texs[p].tex2y = texs[p].tex1y;

		p++;

		points[p].pos0x = points[p-1].pos0x;
		points[p].pos0y = points[p-1].pos0y;
		points[p].pos1x = points[p-1].pos1x;
		points[p].pos1y = points[p-1].pos1y;
		points[p].pos2x = points[p-1].pos1x;
		points[p].pos2y = points[p-1].pos0y;

		texs[p].tex0x = texs[p-1].tex0x;
		texs[p].tex0y = texs[p-1].tex0y;
		texs[p].tex1x = texs[p-1].tex1x;
		texs[p].tex1y = texs[p-1].tex1y;
		texs[p].tex2x = texs[p-1].tex1x;
		texs[p].tex2y = texs[p-1].tex0y;

		x += charData[letter].advX * fontScale;
	}

	Renderer::buildGLColorArray(colors, color, pointCount * 3);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, points);
	glTexCoordPointer(2, GL_FLOAT, 0, texs);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);

	glDrawArrays(GL_TRIANGLES, 0, pointCount * 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	delete[] points;
	delete[] texs;
	delete[] colors;
}

void Font::sizeText(std::string text, int* w, int* h)
{
	float cwidth = 0.0f;
	for(unsigned int i = 0; i < text.length(); i++)
	{
		unsigned char letter = text[i];
		if(letter < 32 || letter >= 128)
			letter = 127;

		cwidth += charData[letter].advX * fontScale;
	}

	if(w != NULL)
		*w = (int)cwidth;

	if(h != NULL)
		*h = getHeight();
}

int Font::getHeight()
{
	return (int)(mMaxGlyphHeight * 1.5f * fontScale);
}
