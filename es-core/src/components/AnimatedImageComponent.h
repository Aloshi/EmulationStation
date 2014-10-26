#include "GuiComponent.h"
#include "ImageComponent.h"

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
