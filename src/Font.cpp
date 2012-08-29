#include "Font.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include "Renderer.h"

FT_Library Font::sLibrary;

int Font::getDpiX() { return 300; }
int Font::getDpiY() { return 300; }

void Font::initLibrary()
{
	if(FT_Init_FreeType(&sLibrary))
	{
		std::cout << "Error initializing FreeType!\n";
	}
}

Font::Font(std::string path, int size)
{
	if(FT_New_Face(sLibrary, path.c_str(), 0, &face))
	{
		std::cout << "Error creating font face! (path: " << path.c_str() << "\n";
		while(true);
	}

	FT_Set_Char_Size(face, 0, size * 64, getDpiX(), getDpiY());

	buildAtlas();
}

void Font::buildAtlas()
{
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

	//GLES requires a power of two
	//the max size (GL_MAX_TEXTURE_SIZE) is pretty small too, like 3300, so just force it
	w = 2048;
	h = 2048;

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
			y += maxHeight;
			maxHeight = 0;
		}

		if(g->bitmap.rows > maxHeight)
			maxHeight = g->bitmap.rows;

		glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);


		charData[i].texX = x;
		charData[i].texY = 0;
		charData[i].texW = g->bitmap.width;
		charData[i].texH = g->bitmap.rows;
		charData[i].advX = g->metrics.horiAdvance >> 6;
		charData[i].advY = g->advance.y >> 6;
		charData[i].bearingY = g->metrics.horiBearingY >> 6;

		x += g->bitmap.width;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	std::cout << "generated texture \"" << textureID << "\" (w: " << w << " h: " << h << ")" << std::endl;
	std::cout << "(final x: " << x << " y: " << y << ")" << std::endl;
}

Font::~Font()
{
	FT_Done_Face(face);

	glDeleteTextures(1, &textureID);
}



//why these aren't in an array:
//openGL reads these in the order they are in memory
//if I use an array, it will be 4 x values then 4 y values
//it'll read XX, XX, YY instead of XY, XY, XY
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

	starty += (face->height / face->units_per_EM) << 6; //(face->height / face->units_per_EM) << 6;

	int pointCount = text.length() * 2;
	point* points = new point[pointCount];
	tex* texs = new tex[pointCount];


	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//texture atlas width/height
	float tw = (float)textureWidth;
	float th = (float)textureHeight;

	int p = 0;
	int i = 0;

	float x = startx;
	float y = starty;
	for(; p < pointCount; i++, p++)
	{
		int letter = text[i];

		if(letter < 32 || letter >= 128)
			continue;

		points[p].pos0x = x;										points[p].pos0y = y + charData[letter].texH - charData[letter].bearingY;
		points[p].pos1x = x + charData[letter].texW;			points[p].pos1y = y - charData[letter].bearingY;
		points[p].pos2x = x;										points[p].pos2y = y - charData[letter].bearingY;

		texs[p].tex0x = charData[letter].texX / tw;									texs[p].tex0y = (charData[letter].texY + charData[letter].texH) / th;
		texs[p].tex1x = (charData[letter].texX + charData[letter].texW) / tw;	texs[p].tex1y = charData[letter].texY / th;
		texs[p].tex2x = charData[letter].texX / tw;									texs[p].tex2y = charData[letter].texY / th;


		p++;

		points[p].pos0x = x;										points[p].pos0y = y + charData[letter].texH  - charData[letter].bearingY;
		points[p].pos1x = x + charData[letter].texW;			points[p].pos1y = y  - charData[letter].bearingY;
		points[p].pos2x = x + charData[letter].texW;			points[p].pos2y = y + charData[letter].texH  - charData[letter].bearingY;

		texs[p].tex0x = charData[letter].texX / tw;									texs[p].tex0y = (charData[letter].texY + charData[letter].texH) / th;
		texs[p].tex1x = (charData[letter].texX + charData[letter].texW) / tw;	texs[p].tex1y = charData[letter].texY / th;
		texs[p].tex2x = texs[p].tex1x;														texs[p].tex2y = texs[p].tex0y;


		x += charData[letter].advX;
	}

	GLubyte* colors = new GLubyte[pointCount * 3 * 4];
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

	//not sure if this properly deletes or not
	delete[] points;
}

void Font::sizeText(std::string text, int* w, int* h)
{
	if(w != NULL)
	{
		int cwidth = 0;
		for(unsigned int i = 0; i < text.length(); i++)
		{
			cwidth += charData[(int)text[i]].advX;
		}

		*w = cwidth;
	}

	if(h != NULL)
		*h = (face->height / face->units_per_EM) << 6;
}
