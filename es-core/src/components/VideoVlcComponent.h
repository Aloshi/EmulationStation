#ifndef _VIDEOVLCCOMPONENT_H_
#define _VIDEOVLCCOMPONENT_H_

#include "platform.h"
#include GLHEADER

#include "VideoComponent.h"
#include <vlc/vlc.h>
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
	static void setupVLC();

	VideoVlcComponent(Window* window);
	virtual ~VideoVlcComponent();

	void render(const Eigen::Affine3f& parentTrans) override;

private:
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
