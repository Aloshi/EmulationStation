#ifndef _VIDEOVLCCOMPONENT_H_
#define _VIDEOVLCCOMPONENT_H_

#include "platform.h"
#include GLHEADER

#include "VideoComponent.h"
#include <vlc/vlc.h>
#include <vlc/libvlc_media.h>
#include "resources/TextureResource.h"

struct VideoContext {
	SDL_Surface*		surface;
	SDL_mutex*			mutex;
	bool				valid;
};

class VideoVlcComponent : public VideoComponent
{
	// Structure that groups together the configuration of the video component
	struct Configuration
	{
		unsigned						startDelay;
		bool							showSnapshotNoVideo;
		bool							showSnapshotDelay;
		std::string						defaultVideoPath;
	};

public:
	static void setupVLC(std::string subtitles);

	VideoVlcComponent(Window* window, std::string subtitles);
	virtual ~VideoVlcComponent();

	void render(const Eigen::Affine3f& parentTrans) override;


	// Resize the video to fit this size. If one axis is zero, scale that axis to maintain aspect ratio.
	// If both are non-zero, potentially break the aspect ratio.  If both are zero, no resizing.
	// Can be set before or after a video is loaded.
	// setMaxSize() and setResize() are mutually exclusive.
	void setResize(float width, float height);

	// Resize the video to be as large as possible but fit within a box of this size.
	// Can be set before or after a video is loaded.
	// Never breaks the aspect ratio. setMaxSize() and setResize() are mutually exclusive.
	void setMaxSize(float width, float height);

private:
	// Calculates the correct mSize from our resizing information (set by setResize/setMaxSize).
	// Used internally whenever the resizing parameters or texture change.
	void resize();
	// Start the video Immediately
	virtual void startVideo();
	// Stop the video
	virtual void stopVideo();
	// Handle looping the video. Must be called periodically
	virtual void handleLooping();

	void setupContext();
	void freeContext();

private:
	static libvlc_instance_t*		mVLC;
	libvlc_media_t*					mMedia;
	libvlc_media_player_t*			mMediaPlayer;
	VideoContext					mContext;
	std::shared_ptr<TextureResource> mTexture;
};

#endif
