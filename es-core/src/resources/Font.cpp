#include "resources/Font.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <boost/filesystem.hpp>
#include "Renderer.h"
#include "Log.h"
#include "Util.h"

FT_Library Font::sLibrary = NULL;

int Font::getSize() const { return mSize; }

std::map< std::pair<std::string, int>, std::weak_ptr<Font> > Font::sFontMap;

void Font::initLibrary()
{
	assert(sLibrary == NULL);
	if(FT_Init_FreeType(&sLibrary))
	{
		sLibrary = NULL;
		LOG(LogError) << "Error initializing FreeType!";
	}
}

size_t Font::getMemUsage() const
{
	if(!mTextureID)
		return 0;

	return mTextureWidth * mTextureHeight * 4;
}

size_t Font::getTotalMemUsage()
{
	size_t total = 0;

	auto it = sFontMap.begin();
	while(it != sFontMap.end())
	{
		if(it->second.expired())
		{
			it = sFontMap.erase(it);
			continue;
		}

		total += it->second.lock()->getMemUsage();
		it++;
	}

	return total;
}

Font::Font(int size, const std::string& path) : mFontScale(1.0f), mSize(size), mPath(path), mTextureID(0)
{
	reload(ResourceManager::getInstance());
}

Font::~Font()
{
	deinit();
}

void Font::reload(std::shared_ptr<ResourceManager>& rm)
{
	init(rm->getFileData(mPath));
}

void Font::unload(std::shared_ptr<ResourceManager>& rm)
{
	deinit();
}

std::shared_ptr<Font> Font::get(int size, const std::string& path)
{
	const std::string canonicalPath = getCanonicalPath(path);

	std::pair<std::string, int> def(canonicalPath.empty() ? getDefaultPath() : canonicalPath, size);
	auto foundFont = sFontMap.find(def);
	if(foundFont != sFontMap.end())
	{
		if(!foundFont->second.expired())
			return foundFont->second.lock();
	}

	std::shared_ptr<Font> font = std::shared_ptr<Font>(new Font(def.second, def.first));
	sFontMap[def] = std::weak_ptr<Font>(font);
	ResourceManager::getInstance()->addReloadable(font);
	return font;
}

void Font::init(ResourceData data)
{
	if(sLibrary == NULL)
		initLibrary();

	deinit();

	mMaxGlyphHeight = 0;

	buildAtlas(data);
}

void Font::deinit()
{
	if(mTextureID)
	{
		glDeleteTextures(1, &mTextureID);
		mTextureID = 0;
	}
}

void Font::buildAtlas(ResourceData data)
{	
	assert(mSize > 0);

	FT_Face face;
	if(FT_New_Memory_Face(sLibrary, data.ptr.get(), data.length, 0, &face))
	{
		LOG(LogError) << "Error creating font face! (mPath: " << mPath << ", data.length: " << data.length << ")";
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, mSize);

	// hardcoded texture size right now
	mTextureWidth = 2048;
	mTextureHeight = 512;

	// create the texture
	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, mTextureWidth, mTextureHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);

	//copy the glyphs into the texture
	int x = 0;
	int y = 0;
	int maxHeight = 0;
	FT_GlyphSlot g = face->glyph;
	for(int i = 32; i < 128; i++)
	{
		if(FT_Load_Char(face, i, FT_LOAD_RENDER))
			continue;

		if(x + g->bitmap.width >= mTextureWidth)
		{
			x = 0;
			y += maxHeight + 1; //leave one pixel of space between glyphs
			maxHeight = 0;
		}

		if(g->bitmap.rows > maxHeight)
			maxHeight = g->bitmap.rows;

		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);


		mCharData[i].texX = x;
		mCharData[i].texY = y;
		mCharData[i].texW = g->bitmap.width;
		mCharData[i].texH = g->bitmap.rows;
		mCharData[i].advX = (float)g->metrics.horiAdvance / 64.0f;
		mCharData[i].advY = (float)g->metrics.vertAdvance / 64.0f;
		mCharData[i].bearingX = (float)g->metrics.horiBearingX / 64.0f;
		mCharData[i].bearingY = (float)g->metrics.horiBearingY / 64.0f;

		if(mCharData[i].texH > mMaxGlyphHeight)
			mMaxGlyphHeight = mCharData[i].texH;

		x += g->bitmap.width + 1; //leave one pixel of space between glyphs
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face);

	if((y + maxHeight) >= mTextureHeight)
	{
		//failed to create a proper font texture
		LOG(LogWarning) << "Font \"" << mPath << "\" with size " << mSize << " exceeded max texture size! Trying again...";
		//try a 3/4th smaller size and redo initialization
		mFontScale *= 1.25f;
		mSize = (int)(mSize * (1.0f / mFontScale));
		deinit();
		init(data);
	}
}

void Font::renderTextCache(TextCache* cache)
{
	if(!mTextureID)
	{
		LOG(LogError) << "Error - tried to draw with Font that has no texture loaded!";
		return;
	}

	if(cache == NULL)
	{
		LOG(LogError) << "Attempted to draw NULL TextCache!";
		return;
	}

	glBindTexture(GL_TEXTURE_2D, mTextureID);
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

Eigen::Vector2f Font::sizeText(std::string text, float lineSpacing) const
{
	float lineWidth = 0.0f;
	float highestWidth = 0.0f;

	float y = getHeight(lineSpacing);

	for(unsigned int i = 0; i < text.length(); i++)
	{
		unsigned char letter = text[i];

		if(letter == '\n')
		{
			if(lineWidth > highestWidth)
				highestWidth = lineWidth;

			lineWidth = 0.0f;
			y += getHeight(lineSpacing);
		}

		if(letter < 32 || letter >= 128)
			letter = 127;

		lineWidth += mCharData[letter].advX * mFontScale;
	}

	if(lineWidth > highestWidth)
		highestWidth = lineWidth;

	return Eigen::Vector2f(highestWidth, y);
}

float Font::getHeight(float lineSpacing) const
{
	return mMaxGlyphHeight * lineSpacing * mFontScale;
}

float Font::getLetterHeight() const
{
	return mCharData['S'].texH * mFontScale;
}

//the worst algorithm ever written
//breaks up a normal string with newlines to make it fit xLen
std::string Font::wrapText(std::string text, float xLen) const
{
	std::string out;

	std::string line, word, temp;
	size_t space;

	Eigen::Vector2f textSize;

	while(text.length() > 0) //while there's text or we still have text to render
	{
		space = text.find_first_of(" \t\n");
		if(space == std::string::npos)
			space = text.length() - 1;

		word = text.substr(0, space + 1);
		text.erase(0, space + 1);

		temp = line + word;

		textSize = sizeText(temp);

		// if the word will fit on the line, add it to our line, and continue
		if(textSize.x() <= xLen)
		{
			line = temp;
			continue;
		}else{
			// the next word won't fit, so break here
			out += line + '\n';
			line = word;
		}
	}

	// whatever's left should fit
	out += line;

	return out;
}

Eigen::Vector2f Font::sizeWrappedText(std::string text, float xLen, float lineSpacing) const
{
	text = wrapText(text, xLen);
	return sizeText(text, lineSpacing);
}

Eigen::Vector2f Font::getWrappedTextCursorOffset(std::string text, float xLen, int cursor, float lineSpacing) const
{
	std::string wrappedText = wrapText(text, xLen);

	float lineWidth = 0.0f;
	float y = 0.0f;

	unsigned int stop = (unsigned int)cursor;
	unsigned int wrapOffset = 0;
	for(unsigned int i = 0; i < stop; i++)
	{
		unsigned char wrappedLetter = wrappedText[i + wrapOffset];
		unsigned char letter = text[i];

		if(wrappedLetter == '\n' && letter != '\n')
		{
			//this is where the wordwrap inserted a newline
			//reset lineWidth and increment y, but don't consume a cursor character
			lineWidth = 0.0f;
			y += getHeight(lineSpacing);

			wrapOffset++;
			i--;
			continue;
		}

		if(letter == '\n')
		{
			lineWidth = 0.0f;
			y += getHeight(lineSpacing);
			continue;
		}

		if(letter < 32 || letter >= 128)
			letter = 127;

		lineWidth += mCharData[letter].advX * mFontScale;
	}

	return Eigen::Vector2f(lineWidth, y);
}

//=============================================================================================================
//TextCache
//=============================================================================================================

float Font::getNewlineStartOffset(const std::string& text, const unsigned int& charStart, const float& xLen, const Alignment& alignment)
{
	switch(alignment)
	{
	case ALIGN_LEFT:
		return 0;
	case ALIGN_CENTER:
		{
			unsigned int endChar = text.find('\n', charStart);
			return (xLen - sizeText(text.substr(charStart, endChar != std::string::npos ? endChar - charStart : endChar)).x()) / 2.0f;
		}
	case ALIGN_RIGHT:
		{
			unsigned int endChar = text.find('\n', charStart);
			return xLen - (sizeText(text.substr(charStart, endChar != std::string::npos ? endChar - charStart : endChar)).x());
		}
	default:
		return 0;
	}
}

TextCache* Font::buildTextCache(const std::string& text, Eigen::Vector2f offset, unsigned int color, float xLen, Alignment alignment, float lineSpacing)
{
	if(!mTextureID)
	{
		LOG(LogError) << "Error - tried to build TextCache with Font that has no texture loaded!";
		return NULL;
	}

	const unsigned int vertCount = text.length() * 2 * 3; // 2 triangles of 3 vertices per character
	TextCache::Vertex* vert = new TextCache::Vertex[vertCount];
	GLubyte* colors = new GLubyte[vertCount * 4];

	//texture atlas width/height
	float tw = (float)mTextureWidth;
	float th = (float)mTextureHeight;

	float x = offset[0] + (xLen != 0 ? getNewlineStartOffset(text, 0, xLen, alignment) : 0);
	
	float yTop = mCharData['S'].bearingY * mFontScale;
	float yBot = getHeight(lineSpacing);
	float y = offset[1] + (yBot + yTop)/2.0f;

	for(unsigned int i = 0, charNum = 0; i < vertCount; i += 6, charNum++)
	{
		unsigned char letter = text[charNum];

		if(letter == '\n')
		{
			y += getHeight(lineSpacing);
			x = offset[0] + (xLen != 0 ? getNewlineStartOffset(text, charNum+1, xLen, alignment) : 0);
			memset(&vert[i], 0, 6 * sizeof(TextCache::Vertex));
			continue;
		}

		if(letter < 32 || letter >= 128)
			letter = 127; //print [X] if character is not standard ASCII

		//the glyph might not start at the cursor position, but needs to be shifted a bit
		const float glyphStartX = x + mCharData[letter].bearingX * mFontScale;
		//order is bottom left, top right, top left
		vert[i + 0].pos << glyphStartX, y + (mCharData[letter].texH - mCharData[letter].bearingY) * mFontScale;
		vert[i + 1].pos << glyphStartX + mCharData[letter].texW * mFontScale, y - mCharData[letter].bearingY * mFontScale;
		vert[i + 2].pos << glyphStartX, vert[i + 1].pos.y();

		Eigen::Vector2i charTexCoord(mCharData[letter].texX, mCharData[letter].texY);
		Eigen::Vector2i charTexSize(mCharData[letter].texW, mCharData[letter].texH);

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

		// round to fix some weird "cut off" text bugs
		for(unsigned int j = i; j < i + 6; j++)
		{
			vert[j].pos[0] = round(vert[j].pos[0]);
			vert[j].pos[1] = round(vert[j].pos[1]);
		}

		x += mCharData[letter].advX * mFontScale;
	}

	TextCache::CacheMetrics metrics = { sizeText(text, lineSpacing) };
	TextCache* cache = new TextCache(vertCount, vert, colors, metrics);
	if(color != 0x00000000)
		cache->setColor(color);

	return cache;
}

TextCache* Font::buildTextCache(const std::string& text, float offsetX, float offsetY, unsigned int color)
{
	return buildTextCache(text, Eigen::Vector2f(offsetX, offsetY), color, 0.0f);
}

TextCache::TextCache(int verts, Vertex* v, GLubyte* c, const CacheMetrics& m) : vertCount(verts), verts(v), colors(c), metrics(m)
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

std::shared_ptr<Font> Font::getFromTheme(const ThemeData::ThemeElement* elem, unsigned int properties, const std::shared_ptr<Font>& orig)
{
	using namespace ThemeFlags;
	if(!(properties & FONT_PATH) && !(properties & FONT_SIZE))
		return orig;
	
	std::shared_ptr<Font> font;
	int size = (orig ? orig->mSize : FONT_SIZE_MEDIUM);
	std::string path = (orig ? orig->mPath : getDefaultPath());

	float sh = (float)Renderer::getScreenHeight();
	if(properties & FONT_SIZE && elem->has("fontSize")) 
		size = (int)(sh * elem->get<float>("fontSize"));
	if(properties & FONT_PATH && elem->has("fontPath"))
		path = elem->get<std::string>("fontPath");

	return get(size, path);
}
