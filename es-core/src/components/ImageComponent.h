#ifndef _IMAGECOMPONENT_H_
#define _IMAGECOMPONENT_H_

#include "platform.h"
#include GLHEADER

#include "GuiComponent.h"
#include <string>
#include <memory>
#include "resources/TextureResource.h"

class ImageComponent : public GuiComponent
{
public:
	ImageComponent(Window* window);
	virtual ~ImageComponent();

	//Loads the image at the given filepath. Will tile if tile is true (retrieves texture as tiling, creates vertices accordingly).
	void setImage(std::string path, bool tile = false);
	//Loads an image from memory.
	void setImage(const char* image, size_t length, bool tile = false);
	//Use an already existing texture.
	void setImage(const std::shared_ptr<TextureResource>& texture);

	void onSizeChanged() override;
	void setOpacity(unsigned char opacity) override;

	//Sets the origin as a percentage of this image (e.g. (0, 0) is top left, (0.5, 0.5) is the center)
	void setOrigin(float originX, float originY);
	inline void setOrigin(Eigen::Vector2f origin) { setOrigin(origin.x(), origin.y()); }

	// Resize the image to fit this size. If one axis is zero, scale that axis to maintain aspect ratio.
	// If both are non-zero, potentially break the aspect ratio.  If both are zero, no resizing.
	// Can be set before or after an image is loaded.
	// setMaxSize() and setResize() are mutually exclusive.
	void setResize(float width, float height);
	inline void setResize(const Eigen::Vector2f& size) { setResize(size.x(), size.y()); }

	// Resize the image to be as large as possible but fit within a box of this size.
	// Can be set before or after an image is loaded.
	// Never breaks the aspect ratio. setMaxSize() and setResize() are mutually exclusive.
	void setMaxSize(float width, float height);
	inline void setMaxSize(const Eigen::Vector2f& size) { setMaxSize(size.x(), size.y()); }

	// Multiply all pixels in the image by this color when rendering.
	void setColorShift(unsigned int color);

	void setFlipX(bool flip); // Mirror on the X axis.
	void setFlipY(bool flip); // Mirror on the Y axis.

	// Returns the size of the current texture, or (0, 0) if none is loaded.  May be different than drawn size (use getSize() for that).
	Eigen::Vector2i getTextureSize() const;

	// Returns the center point of the image (takes origin into account).
	Eigen::Vector2f getCenter() const;

	bool hasImage();

	void render(const Eigen::Affine3f& parentTrans) override;

	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

	virtual std::vector<HelpPrompt> getHelpPrompts() override;
private:
	Eigen::Vector2f mTargetSize;
	Eigen::Vector2f mOrigin;

	bool mFlipX, mFlipY, mTargetIsMax;

	// Calculates the correct mSize from our resizing information (set by setResize/setMaxSize).
	// Used internally whenever the resizing parameters or texture change.
	void resize();

	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	} mVertices[6];

	GLubyte mColors[6*4];

	void updateVertices();
	void updateColors();

	unsigned int mColorShift;

	std::shared_ptr<TextureResource> mTexture;
};

#endif
