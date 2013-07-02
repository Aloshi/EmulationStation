#ifndef _IMAGECOMPONENT_H_
#define _IMAGECOMPONENT_H_

#include "../platform.h"
#include GLHEADER

#include "../GuiComponent.h"
#include <string>
#include <FreeImage.h>


class ImageComponent : public GuiComponent
{
public:
	//Creates a new GuiImage at the given location. If given an image, it will be loaded. If maxWidth and/or maxHeight are nonzero, the image will be
	//resized to fit. If only one axis is specified, the other will be set in accordance with the image's aspect ratio. If allowUpscale is false,
	//the image will only be downscaled, never upscaled (the image's size must surpass at least one nonzero bound).
	ImageComponent(Window* window, int offsetX = 0, int offsetY = 0, std::string path = "", unsigned int maxWidth = 0, unsigned int maxHeight = 0, bool allowUpscale = false);
	virtual ~ImageComponent();

	//Copy the entire screen into a texture for us to use.
	void copyScreen();
	void setImage(std::string path); //Loads the image at the given filepath.
	void setOrigin(float originX, float originY); //Sets the origin as a percentage of this image (e.g. (0, 0) is top left, (0.5, 0.5) is the center)
	void setTiling(bool tile); //Enables or disables tiling. Must be called before loading an image or resizing will be weird.
	void setResize(unsigned int width, unsigned int height, bool allowUpscale);

	void setFlipX(bool flip);
	void setFlipY(bool flip);

	//You can get the rendered size of the ImageComponent with getSize().
	Vector2u getTextureSize();


	bool hasImage();

	//Image textures will be deleted on renderer deinitialization, and recreated on reinitialization (if mPath is not empty).
	void init();
	void deinit();

protected:
	void onRender();

private:
	Vector2u mTargetSize;
	Vector2u mTextureSize;
	Vector2f mOrigin;

	bool mAllowUpscale, mTiled, mFlipX, mFlipY;

	void loadImage(std::string path);
	void resize();
	void buildImageArray(int x, int y, GLfloat* points, GLfloat* texs, float percentageX = 1, float percentageY = 1); //writes 12 GLfloat points and 12 GLfloat texture coordinates to a given array at a given position
	void drawImageArray(GLfloat* points, GLfloat* texs, GLubyte* colors, unsigned int count = 6); //draws the given set of points and texture coordinates, number of coordinate pairs may be specified (default 6)
	void unloadImage();

	std::string mPath;

	GLuint mTextureID;
};

#endif
