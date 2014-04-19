#include "../GuiComponent.h"

class ImageComponent;

struct AnimationFrame
{
	const char* path;
	int time;
};

struct AnimationDef
{
	AnimationFrame* frames;
	size_t frameCount;
	bool loop;
};

class AnimatedImageComponent : public GuiComponent
{
public:
	AnimatedImageComponent(Window* window);
	
	void load(const AnimationDef* def); // no reference to def is kept after loading is complete

	void reset(); // set to frame 0

	void update(int deltaTime) override;
	void render(const Eigen::Affine3f& trans) override;

	void onSizeChanged() override;

private:
	typedef std::pair<std::unique_ptr<ImageComponent>, int> ImageFrame;

	std::vector<ImageFrame> mFrames;

	bool mLoop;
	bool mEnabled;
	int mFrameAccumulator;
	int mCurrentFrame;
};


// animation declarations because there's only one right now and I'm too lazy to make another file
extern const AnimationDef BUSY_ANIMATION_DEF;
