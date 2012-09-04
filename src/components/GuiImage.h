#ifndef _GUIIMAGE_H_
#define _GUIIMAGE_H_

#include "../GuiComponent.h"
#include <string>
#include <FreeImage.h>
#include <GLES/gl.h>

class GuiImage : public GuiComponent
{
public:
	GuiImage(int offsetX = 0, int offsetY = 0, std::string path = "", unsigned int maxWidth = 0, unsigned int maxHeight = 0, bool resizeExact = false);
	~GuiImage();

	void setImage(std::string path);
	void setOrigin(float originX, float originY);
	void setTiling(bool tile);

	int getWidth();
	int getHeight();

	void onRender();

	void onInit();
	void onDeinit();

private:
	unsigned int mResizeWidth, mResizeHeight;
	float mOriginX, mOriginY;
	bool mResizeExact, mTiled;

	void loadImage(std::string path);
	void drawImage(int x, int y);
	void unloadImage();

	std::string mPath;

	int mOffsetX, mOffsetY;
	unsigned int mWidth, mHeight;

	GLuint mTextureID;
};

#endif
