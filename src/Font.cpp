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

int Font::getSize() const { return mSize; }

std::map< std::pair<std::string, int>, std::weak_ptr<Font> > Font::sFontMap;

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

Font::Font(const ResourceManager& rm, const std::string& path, int size) : fontScale(1.0f), mSize(size), mPath(path)
{
	reload(rm);
}

Font::~Font()
{
	LOG(LogInfo) << "Destroying font \"" << mPath << "\" with size " << mSize << ".";
	deinit();
}

void Font::reload(const ResourceManager& rm)
{
	init(rm.getFileData(mPath));
}

void Font::unload(const ResourceManager& rm)
{
	deinit();
}

std::shared_ptr<Font> Font::get(ResourceManager& rm, const std::string& path, int size)
{
	if(path.empty())
	{
		LOG(LogError) << "Tried to get font with no path!";
		return std::shared_ptr<Font>();
	}

	std::pair<std::string, int> def(path, size);
	auto foundFont = sFontMap.find(def);
	if(foundFont != sFontMap.end())
	{
		if(!foundFont->second.expired())
			return foundFont->second.lock();
	}

	std::shared_ptr<Font> font = std::shared_ptr<Font>(new Font(rm, path, size));
	sFontMap[def] = std::weak_ptr<Font>(font);
	rm.addReloadable(font);
	return font;
}

void Font::init(ResourceData data)
{
	if(!libraryInitialized)
		initLibrary();

	mMaxGlyphHeight = 0;

	buildAtlas(data);
}

void Font::deinit()
{
	if(textureID)
		glDeleteTextures(1, &textureID);
}

void Font::buildAtlas(ResourceData data)
{
	if(FT_New_Memory_Face(sLibrary, data.ptr.get(), data.length, 0, &face))
	{
		LOG(LogError) << "Error creating font face!";
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

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
		charData[i].advX = (float)g->metrics.horiAdvance / 64.0f;
		charData[i].advY = (float)g->metrics.vertAdvance / 64.0f;
		charData[i].bearingX = (float)g->metrics.horiBearingX / 64.0f;
		charData[i].bearingY = (float)g->metrics.horiBearingY / 64.0f;

		if(charData[i].texH > mMaxGlyphHeight)
			mMaxGlyphHeight = charData[i].texH;

		x += g->bitmap.width + 1; //leave one pixel of space between glyphs
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face);

	if((y + maxHeight) >= textureHeight)
	{
		//failed to create a proper font texture
		LOG(LogWarning) << "Font \"" << mPath << "\" with size " << mSize << " exceeded max texture size! Trying again...";
		//try a 3/4th smaller size and redo initialization
		fontScale *= 1.25f;
		mSize = (int)(mSize * (1.0f / fontScale));
		deinit();
		init(data);
	}else{
		LOG(LogInfo) << "Created font \"" << mPath << "\" with size " << mSize << ".";
	}
}


void Font::drawText(std::string text, const Eigen::Vector2f& offset, unsigned int color)
{
	TextCache* cache = buildTextCache(text, offset[0], offset[1], color);
	renderTextCache(cache);
	delete cache;
}

void Font::renderTextCache(TextCache* cache)
{
	if(!textureID)
	{
		LOG(LogError) << "Error - tried to draw with Font that has no texture loaded!";
		return;
	}

	if(cache == NULL)
	{
		LOG(LogError) << "Attempted to draw NULL TextCache!";
		return;
	}

	if(cache->sourceFont != this)
	{
		LOG(LogError) << "Attempted to draw TextCache with font other than its source!";
		return;
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(TextCache::Vertex), &cache->verts[0].pos);
	glTexCoordPointer(2, GL_FLOAT, sizeof(TextCache::Vertex), &cache->verts[0].tex);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, cache->colors);

	glDrawArrays(GL_TRIANGLES, 0, cache->vertCount);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

Eigen::Vector2f Font::sizeText(std::string text) const
{
	float cwidth = 0.0f;
	for(unsigned int i = 0; i < text.length(); i++)
	{
		unsigned char letter = text[i];
		if(letter < 32 || letter >= 128)
			letter = 127;

		cwidth += charData[letter].advX * fontScale;
	}

	return Eigen::Vector2f(cwidth, getHeight());
}

int Font::getHeight() const
{
	return (int)(mMaxGlyphHeight * 1.5f * fontScale);
}




void Font::drawCenteredText(std::string text, float xOffset, float y, unsigned int color)
{
	Eigen::Vector2f pos = sizeText(text);
	
	pos[0] = (Renderer::getScreenWidth() - pos.x());
	pos[0] = (pos.x() / 2) + (xOffset / 2);
	pos[1] = y;

	drawText(text, pos, color);
}

//this could probably be optimized
//draws text and ensures it's never longer than xLen
void Font::drawWrappedText(std::string text, const Eigen::Vector2f& offset, float xLen, unsigned int color)
{
	float y = offset.y();

	std::string line, word, temp;
	Eigen::Vector2f textSize;
	size_t space, newline;

	while(text.length() > 0 || !line.empty()) //while there's text or we still have text to render
	{
		space = text.find(' ', 0);
		if(space == std::string::npos)
			space = text.length() - 1;


		word = text.substr(0, space + 1);

		//check if the next word contains a newline
		newline = word.find('\n', 0);
		if(newline != std::string::npos)
		{
			word = word.substr(0, newline);
			text.erase(0, newline + 1);
		}else{
			text.erase(0, space + 1);
		}

		temp = line + word;

		textSize = sizeText(temp);

		//if we're on the last word and it'll fit on the line, just add it to the line
		if((textSize.x() <= xLen && text.length() == 0) || newline != std::string::npos)
		{
			line = temp;
			word = "";
		}


		//if the next line will be too long or we're on the last of the text, render it
		if(textSize.x() > xLen || text.length() == 0 || newline != std::string::npos)
		{
			//render line now
			if(textSize.x() > 0) //make sure it's not blank
				drawText(line, Eigen::Vector2f(offset.x(), y), color);

			//increment y by height and some extra padding for the next line
			y += textSize.y() + 4;

			//move the word we skipped to the next line
			line = word;
		}else{
			//there's still space, continue building the line
			line = temp;
		}

	}
}

Eigen::Vector2f Font::sizeWrappedText(std::string text, float xLen) const
{
	Eigen::Vector2f out(0, 0);
	
	float y = 0;

	std::string line, word, temp;
	Eigen::Vector2f textSize;
	size_t space, newline;

	while(text.length() > 0 || !line.empty()) //while there's text or we still have text to render
	{
		space = text.find(' ', 0);
		if(space == std::string::npos)
			space = text.length() - 1;

		word = text.substr(0, space + 1);

		//check if the next word contains a newline
		newline = word.find('\n', 0);
		if(newline != std::string::npos)
		{
			word = word.substr(0, newline);
			text.erase(0, newline + 1);
		}else{
			text.erase(0, space + 1);
		}

		temp = line + word;

		textSize = sizeText(temp);

		//if we're on the last word and it'll fit on the line, just add it to the line
		if((textSize.x() <= xLen && text.length() == 0) || newline != std::string::npos)
		{
			line = temp;
			word = "";
		}

		//if the next line will be too long or we're on the last of the text, render it
		if(textSize.x() > xLen || text.length() == 0 || newline != std::string::npos)
		{
			//increment y by height and some extra padding for the next line
			y += textSize.y() + 4;

			//move the word we skipped to the next line
			line = word;

			//update our maximum known line width
			if(textSize.x() > out.x())
				out[0] = textSize.x();
		}else{
			//there's still space, continue building the line
			line = temp;
		}

	}

	out[1] = y;

	return out;
}




//=============================================================================================================
//TextCache
//=============================================================================================================

TextCache* Font::buildTextCache(const std::string& text, float offsetX, float offsetY, unsigned int color)
{
	if(!textureID)
	{
		LOG(LogError) << "Error - tried to build TextCache with Font that has no texture loaded!";
		return NULL;
	}

	const int triCount = text.length() * 2;
	const int vertCount = triCount * 3;
	TextCache::Vertex* vert = new TextCache::Vertex[vertCount];
	GLubyte* colors = new GLubyte[vertCount * 4];

	//texture atlas width/height
	float tw = (float)textureWidth;
	float th = (float)textureHeight;

	float x = offsetX;
	float y = offsetY + mMaxGlyphHeight * 1.1f * fontScale; //padding (another 0.5% is added to the bottom through the sizeText function)

	int charNum = 0;
	for(int i = 0; i < vertCount; i += 6, charNum++)
	{
		unsigned char letter = text[charNum];

		if(letter < 32 || letter >= 128)
			letter = 127; //print [X] if character is not standard ASCII

		//the glyph might not start at the cursor position, but needs to be shifted a bit
		const float glyphStartX = x + charData[letter].bearingX * fontScale;
		//order is bottom left, top right, top left
		vert[i + 0].pos << glyphStartX, y + (charData[letter].texH - charData[letter].bearingY) * fontScale;
		vert[i + 1].pos << glyphStartX + charData[letter].texW * fontScale, y - charData[letter].bearingY * fontScale;
		vert[i + 2].pos << glyphStartX, vert[i + 1].pos.y();

		Eigen::Vector2i charTexCoord(charData[letter].texX, charData[letter].texY);
		Eigen::Vector2i charTexSize(charData[letter].texW, charData[letter].texH);

		vert[i + 0].tex << charTexCoord.x() / tw, (charTexCoord.y() + charTexSize.y()) / th;
		vert[i + 1].tex << (charTexCoord.x() + charTexSize.x()) / tw, charTexCoord.y() / th;
		vert[i + 2].tex << vert[i + 0].tex.x(), vert[i + 1].tex.y();

		//next triangle (second half of the quad)
		vert[i + 3].pos = vert[i + 0].pos;
		vert[i + 4].pos = vert[i + 1].pos;
		vert[i + 5].pos[0] = vert[i + 1].pos.x();
		vert[i + 5].pos[1] = vert[i + 0].pos.y();

		vert[i + 3].tex = vert[i + 0].tex;
		vert[i + 4].tex = vert[i + 1].tex;
		vert[i + 5].tex[0] = vert[i + 1].tex.x();
		vert[i + 5].tex[1] = vert[i + 0].tex.y();

		x += charData[letter].advX * fontScale;
	}

	TextCache* cache = new TextCache(vertCount, vert, colors, this);
	if(color != 0x00000000)
		cache->setColor(color);

	return cache;
}

TextCache::TextCache(int verts, Vertex* v, GLubyte* c, Font* f) : vertCount(verts), verts(v), colors(c), sourceFont(f)
{
}

TextCache::~TextCache()
{
	delete[] verts;
	delete[] colors;
}

void TextCache::setColor(unsigned int color)
{
	Renderer::buildGLColorArray(const_cast<GLubyte*>(colors), color, vertCount);
}
