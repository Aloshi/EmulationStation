#ifndef _GUIIMAGE_H_
#define _GUIIMAGE_H_

#include "../Gui.h"
#include <string>
#include <FreeImage.h>
#include "../platform.h"
#include GLHEADER

class GuiImage : public Gui
{
public:
	//Creates a new GuiImage at the given location. If given an image, it will be loaded. If maxWidth and/or maxHeight are nonzero, the image will be
	//resized to fix. If only one axis is specified, the other will be resized in accordance with the image's aspect ratio. If resizeExact is false,
	//the image will only be downscaled, never upscaled (the image's size must surpass at least one nonzero bound).
	GuiImage(Window* window, int offsetX = 0, int offsetY = 0, std::string path = "", unsigned int maxWidth = 0, unsigned int maxHeight = 0, bool resizeExact = false);
	~GuiImage();

	void setImage(std::string path); //Loads the image at the given filepath.
	void setOrigin(float originX, float originY); //Sets the origin as a percentage of this image (e.g. (0, 0) is top left, (0.5, 0.5) is the center)
	void setTiling(bool tile); //Enables or disables tiling. Must be called before loading an image or resizing will be weird.
	void setResize(unsigned int width, unsigned int height, bool resizeExact);

	void setFlipX(bool flip);
	void setFlipY(bool flip);

	unsigned int getWidth(); //Returns render width in pixels. May be different than actual texture width.
	unsigned int getHeight(); //Returns render height in pixels. May be different than actual texture height.

	bool hasImage();

	void render();

	//Image textures will be deleted on renderer deinitialization, and recreated on reinitialization (if mPath is not empty).
	void onInit();
	void onDeinit();

	int getOffsetX();
	int getOffsetY();
	unsigned char getOpacity();
	void setOpacity(unsigned char opacity);
	void setOffset(int x, int y);
private:
	unsigned int mResizeWidth, mResizeHeight;
	float mOriginX, mOriginY;
	bool mResizeExact, mTiled, mFlipX, mFlipY;

	int mOffsetX, mOffsetY;
	unsigned char mOpacity;

	void loadImage(std::string path);
	void resize();
	void buildImageArray(int x, int y, GLfloat* points, GLfloat* texs, float percentageX = 1, float percentageY = 1); //writes 12 GLfloat points and 12 GLfloat texture coordinates to a given array at a given position
	void drawImageArray(GLfloat* points, GLfloat* texs, GLubyte* colors, unsigned int count = 6); //draws the given set of points and texture coordinates, number of coordinate pairs may be specified (default 6)
	void unloadImage();

	std::string mPath;

	unsigned int mWidth, mHeight; //Our rendered size.
	unsigned int mDrawWidth, mDrawHeight;

	GLuint mTextureID;
};

#endif
