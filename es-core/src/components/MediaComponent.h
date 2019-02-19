#pragma once

#include "platform.h"
#include GLHEADER

#include "GuiComponent.h"
#include <string>
#include <memory>

class MediaComponent : public GuiComponent
{
public:
	MediaComponent(Window* window);
	virtual ~MediaComponent();

	virtual void onSizeChanged() override;
	virtual void setOpacity(unsigned char opacity) override;

	// Sets the origin as a percentage of this image (e.g. (0, 0) is top left, (0.5, 0.5) is the center).
	// Image position is relative to this origin value.
	void setOrigin(Eigen::Vector2f origin);
	inline void setOrigin(float x, float y) { setOrigin(Eigen::Vector2f(x, y)); }

	// Resize the image to fit this size. If one axis is zero, scale that axis to maintain aspect ratio.
	// If both are non-zero, potentially break the aspect ratio.  If both are zero, no resizing.
	// Can be set before or after an image is loaded.
	// setMaxSize() and setResize() are mutually exclusive.
	void setResize(Eigen::Vector2f size);
	inline void setResize(float x, float y) { setResize(Eigen::Vector2f(x, y)); }

	// Resize the image to be as large as possible but fit within a box of this size.
	// Can be set before or after an image is loaded.
	// Never breaks the aspect ratio. setMaxSize() and setResize() are mutually exclusive.
	void setMaxSize(Eigen::Vector2f maxSize);
	inline void setMaxSize(float x, float y) { setMaxSize(Eigen::Vector2f(x, y)); }

	// Mirror on the X/Y axes.
	void setFlip(bool flipX, bool flipY);

	// Multiply all pixels in the image by this color when rendering.
	void setColorShift(unsigned int color);

	Eigen::Vector2f getCenter() const;

	virtual bool hasImage();

	virtual void render(const Eigen::Affine3f& parentTrans) override;

	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;

protected:
	virtual Eigen::Vector2i getTextureSize() = 0;
	virtual Eigen::Vector2f getSourceTextureSize();
	virtual bool isTextureTiled() = 0;
	virtual GLuint getTexture() = 0;

	void onTextureChanged();

private:
	enum ResizeType
	{
		RESIZE_TARGET,
		RESIZE_MAX
	};

	ResizeType mResizeType;

	Eigen::Vector2f mTargetSize;
	Eigen::Vector2f mOrigin;
	Eigen::Vector2i mFlip;

	// Calculates the correct mSize from our resizing information (set by setResize/setMaxSize).
	// Used internally whenever the resizing parameters or texture change.
	void resize();

	struct Vertex
	{
		Eigen::Vector2f pos;
		Eigen::Vector2f tex;
	} mVertices[6];

	GLubyte mColors[6 * 4];

	void updateVertices();
	void updateColors();

	unsigned int mColorShift;
};