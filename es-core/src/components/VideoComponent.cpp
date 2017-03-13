#include "components/VideoComponent.h"
#include "Renderer.h"
#include "ThemeData.h"
#include "Util.h"
#ifdef WIN32
#include <codecvt>
#endif

#define FADE_TIME_MS	200

libvlc_instance_t*		VideoComponent::mVLC = NULL;

// VLC prepares to render a video frame.
static void *lock(void *data, void **p_pixels) {
    struct VideoContext *c = (struct VideoContext *)data;
    SDL_LockMutex(c->mutex);
    SDL_LockSurface(c->surface);
	*p_pixels = c->surface->pixels;
    return NULL; // Picture identifier, not needed here.
}

// VLC just rendered a video frame.
static void unlock(void *data, void *id, void *const *p_pixels) {
    struct VideoContext *c = (struct VideoContext *)data;
    SDL_UnlockSurface(c->surface);
    SDL_UnlockMutex(c->mutex);
}

// VLC wants to display a video frame.
static void display(void *data, void *id) {
    //Data to be displayed
}

VideoComponent::VideoComponent(Window* window) :
	GuiComponent(window),
	mStaticImage(window),
	mMediaPlayer(nullptr),
	mVideoHeight(0),
	mVideoWidth(0),
	mStartDelayed(false),
	mIsPlaying(false),
	mShowing(false),
	mTargetIsMax(false),
	mOrigin(0, 0),
	mTargetSize(0, 0)
{
	memset(&mContext, 0, sizeof(mContext));

	// Setup the default configuration
	mConfig.showSnapshotDelay 		= false;
	mConfig.showSnapshotNoVideo		= false;
	mConfig.startDelay				= 0;

	// Get an empty texture for rendering the video
	mTexture = TextureResource::get("");

	// Make sure VLC has been initialised
	setupVLC();
}

VideoComponent::~VideoComponent()
{
	// Stop any currently running video
	stopVideo();
}

void VideoComponent::setOrigin(float originX, float originY)
{
	mOrigin << originX, originY;

	// Update the embeded static image
	mStaticImage.setOrigin(originX, originY);
}

void VideoComponent::setResize(float width, float height)
{
	mTargetSize << width, height;
	mTargetIsMax = false;
	mStaticImage.setResize(width, height);
	resize();
}

void VideoComponent::setMaxSize(float width, float height)
{
	mTargetSize << width, height;
	mTargetIsMax = true;
	mStaticImage.setMaxSize(width, height);
	resize();
}

Eigen::Vector2f VideoComponent::getCenter() const
{
	return Eigen::Vector2f(mPosition.x() - (getSize().x() * mOrigin.x()) + getSize().x() / 2,
		mPosition.y() - (getSize().y() * mOrigin.y()) + getSize().y() / 2);
}

void VideoComponent::resize()
{
	if(!mTexture)
		return;

	const Eigen::Vector2f textureSize(mVideoWidth, mVideoHeight);

	if(textureSize.isZero())
		return;

		// SVG rasterization is determined by height (see SVGResource.cpp), and rasterization is done in terms of pixels
		// if rounding is off enough in the rasterization step (for images with extreme aspect ratios), it can cause cutoff when the aspect ratio breaks
		// so, we always make sure the resultant height is an integer to make sure cutoff doesn't happen, and scale width from that
		// (you'll see this scattered throughout the function)
		// this is probably not the best way, so if you're familiar with this problem and have a better solution, please make a pull request!

		if(mTargetIsMax)
		{

			mSize = textureSize;

			Eigen::Vector2f resizeScale((mTargetSize.x() / mSize.x()), (mTargetSize.y() / mSize.y()));

			if(resizeScale.x() < resizeScale.y())
			{
				mSize[0] *= resizeScale.x();
				mSize[1] *= resizeScale.x();
			}else{
				mSize[0] *= resizeScale.y();
				mSize[1] *= resizeScale.y();
			}

			// for SVG rasterization, always calculate width from rounded height (see comment above)
			mSize[1] = round(mSize[1]);
			mSize[0] = (mSize[1] / textureSize.y()) * textureSize.x();

		}else{
			// if both components are set, we just stretch
			// if no components are set, we don't resize at all
			mSize = mTargetSize.isZero() ? textureSize : mTargetSize;

			// if only one component is set, we resize in a way that maintains aspect ratio
			// for SVG rasterization, we always calculate width from rounded height (see comment above)
			if(!mTargetSize.x() && mTargetSize.y())
			{
				mSize[1] = round(mTargetSize.y());
				mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
			}else if(mTargetSize.x() && !mTargetSize.y())
			{
				mSize[1] = round((mTargetSize.x() / textureSize.x()) * textureSize.y());
				mSize[0] = (mSize.y() / textureSize.y()) * textureSize.x();
			}
		}

	// mSize.y() should already be rounded
	mTexture->rasterizeAt((int)round(mSize.x()), (int)round(mSize.y()));

	onSizeChanged();
}


void VideoComponent::onSizeChanged()
{
	// Update the embeded static image
	mStaticImage.onSizeChanged();
}

bool VideoComponent::setVideo(std::string path)
{
	// Convert the path into a format VLC can understand
	boost::filesystem::path fullPath = getCanonicalPath(path);
	fullPath.make_preferred().native();

	// Check that it's changed
	if (fullPath == mVideoPath)
		return !path.empty();

	// Store the path
	mVideoPath = fullPath;

	// If the file exists then set the new video
	if (!fullPath.empty() && ResourceManager::getInstance()->fileExists(fullPath.generic_string()))
	{
		// Return true to show that we are going to attempt to play a video
		return true;
	}
	// Return false to show that no video will be displayed
	return false;
}

void VideoComponent::setImage(std::string path)
{
	// Check that the image has changed
	if (path == mStaticImagePath)
		return;
	
	mStaticImage.setImage(path);
	mFadeIn = 0.0f;
	mStaticImagePath = path;
}

void VideoComponent::setDefaultVideo()
{
	setVideo(mConfig.defaultVideoPath);
}

void VideoComponent::setOpacity(unsigned char opacity)
{
	mOpacity = opacity;
	// Update the embeded static image
	mStaticImage.setOpacity(opacity);
}

void VideoComponent::render(const Eigen::Affine3f& parentTrans)
{
	float x, y;

	Eigen::Affine3f trans = parentTrans * getTransform();
	GuiComponent::renderChildren(trans);

	Renderer::setMatrix(trans);
	
	// Handle the case where the video is delayed
	handleStartDelay();

	// Handle looping of the video
	handleLooping();

	if (mIsPlaying && mContext.valid)
	{
		float tex_offs_x = 0.0f;
		float tex_offs_y = 0.0f;
		float x2;
		float y2;

		x = -(float)mSize.x() * mOrigin.x();
		y = -(float)mSize.y() * mOrigin.y();
		x2 = x+mSize.x();
		y2 = y+mSize.y();

		// Define a structure to contain the data for each vertex
		struct Vertex
		{
			Eigen::Vector2f pos;
			Eigen::Vector2f tex;
			Eigen::Vector4f colour;
		} vertices[6];

		// We need two triangles to cover the rectangular area
		vertices[0].pos[0] = x; 			vertices[0].pos[1] = y;
		vertices[1].pos[0] = x; 			vertices[1].pos[1] = y2;
		vertices[2].pos[0] = x2;			vertices[2].pos[1] = y;

		vertices[3].pos[0] = x2;			vertices[3].pos[1] = y;
		vertices[4].pos[0] = x; 			vertices[4].pos[1] = y2;
		vertices[5].pos[0] = x2;			vertices[5].pos[1] = y2;

		// Texture coordinates
		vertices[0].tex[0] = -tex_offs_x; 			vertices[0].tex[1] = -tex_offs_y;
		vertices[1].tex[0] = -tex_offs_x; 			vertices[1].tex[1] = 1.0f + tex_offs_y;
		vertices[2].tex[0] = 1.0f + tex_offs_x;		vertices[2].tex[1] = -tex_offs_y;

		vertices[3].tex[0] = 1.0f + tex_offs_x;		vertices[3].tex[1] = -tex_offs_y;
		vertices[4].tex[0] = -tex_offs_x;			vertices[4].tex[1] = 1.0f + tex_offs_y;
		vertices[5].tex[0] = 1.0f + tex_offs_x;		vertices[5].tex[1] = 1.0f + tex_offs_y;

		// Colours - use this to fade the video in and out
		for (int i = 0; i < (4 * 6); ++i) {
			if ((i%4) < 3)
				vertices[i / 4].colour[i % 4] = mFadeIn;
			else
				vertices[i / 4].colour[i % 4] = 1.0f;
		}

		glEnable(GL_TEXTURE_2D);

		// Build a texture for the video frame
		mTexture->initFromPixels((unsigned char*)mContext.surface->pixels, mContext.surface->w, mContext.surface->h);
		mTexture->bind();

		// Render it
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glColorPointer(4, GL_FLOAT, sizeof(Vertex), &vertices[0].colour);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &vertices[0].tex);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		// This is the case where the video is not currently being displayed. Work out
		// if we need to display a static image
		if ((mConfig.showSnapshotNoVideo && mVideoPath.empty()) || (mStartDelayed && mConfig.showSnapshotDelay))
		{
			// Display the static image instead
			mStaticImage.setOpacity((unsigned char)(mFadeIn * 255.0f));
			mStaticImage.render(parentTrans);
		}
	}

}

void VideoComponent::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	using namespace ThemeFlags;

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "video");
	if(!elem)
	{
		return;
	}

	Eigen::Vector2f scale = getParent() ? getParent()->getSize() : Eigen::Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	if ((properties & POSITION) && elem->has("pos"))
	{
		Eigen::Vector2f denormalized = elem->get<Eigen::Vector2f>("pos").cwiseProduct(scale);
		setPosition(Eigen::Vector3f(denormalized.x(), denormalized.y(), 0));
		mStaticImage.setPosition(Eigen::Vector3f(denormalized.x(), denormalized.y(), 0));
	}

	if(properties & ThemeFlags::SIZE)
	{
		if(elem->has("size"))
			setResize(elem->get<Eigen::Vector2f>("size").cwiseProduct(scale));
		else if(elem->has("maxSize"))
			setMaxSize(elem->get<Eigen::Vector2f>("maxSize").cwiseProduct(scale));
	}

	// position + size also implies origin
	if (((properties & ORIGIN) || ((properties & POSITION) && (properties & ThemeFlags::SIZE))) && elem->has("origin"))
		setOrigin(elem->get<Eigen::Vector2f>("origin"));

	if(elem->has("default"))
		mConfig.defaultVideoPath = elem->get<std::string>("default");

	if((properties & ThemeFlags::DELAY) && elem->has("delay"))
		mConfig.startDelay = (unsigned)(elem->get<float>("delay") * 1000.0f);

	if (elem->has("showSnapshotNoVideo"))
		mConfig.showSnapshotNoVideo = elem->get<bool>("showSnapshotNoVideo");

	if (elem->has("showSnapshotDelay"))
		mConfig.showSnapshotDelay = elem->get<bool>("showSnapshotDelay");
}

std::vector<HelpPrompt> VideoComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> ret;
	ret.push_back(HelpPrompt("a", "select"));
	return ret;
}

void VideoComponent::setupContext()
{
	if (!mContext.valid)
	{
		// Create an RGBA surface to render the video into
		mContext.surface = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)mVideoWidth, (int)mVideoHeight, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
		mContext.mutex = SDL_CreateMutex();
		mContext.valid = true;
		resize();
	}
}

void VideoComponent::freeContext()
{
	if (mContext.valid)
	{
		SDL_FreeSurface(mContext.surface);
		SDL_DestroyMutex(mContext.mutex);
		mContext.valid = false;
	}
}

void VideoComponent::setupVLC()
{
	// If VLC hasn't been initialised yet then do it now
	if (!mVLC)
	{
		const char* args[] = { "--quiet" };
		mVLC = libvlc_new(sizeof(args) / sizeof(args[0]), args);
	}
}

void VideoComponent::handleStartDelay()
{
	// Only play if any delay has timed out
	if (mStartDelayed)
	{
		if (mStartTime > SDL_GetTicks())
		{
			// Timeout not yet completed
			return;
		}
		// Completed
		mStartDelayed = false;
		// Clear the playing flag so startVideo works
		mIsPlaying = false;
		startVideo();
	}
}

void VideoComponent::handleLooping()
{
	if (mIsPlaying && mMediaPlayer)
	{
		libvlc_state_t state = libvlc_media_player_get_state(mMediaPlayer);
		if (state == libvlc_Ended)
		{
			//libvlc_media_player_set_position(mMediaPlayer, 0.0f);
			libvlc_media_player_set_media(mMediaPlayer, mMedia);
			libvlc_media_player_play(mMediaPlayer);
		}
	}
}

void VideoComponent::startVideo()
{
	if (!mIsPlaying) {
		mVideoWidth = 0;
		mVideoHeight = 0;

#ifdef WIN32
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wton;
		std::string path = wton.to_bytes(mVideoPath.c_str());
#else
		std::string path(mVideoPath.c_str());
#endif
		// Make sure we have a video path
		if (mVLC && (path.size() > 0))
		{
			// Set the video that we are going to be playing so we don't attempt to restart it
			mPlayingVideoPath = mVideoPath;

			// Open the media
			mMedia = libvlc_media_new_path(mVLC, path.c_str());
			if (mMedia)
			{
				unsigned 	track_count;
				// Get the media metadata so we can find the aspect ratio
				libvlc_media_parse(mMedia);
				libvlc_media_track_t** tracks;
				track_count = libvlc_media_tracks_get(mMedia, &tracks);
				for (unsigned track = 0; track < track_count; ++track)
				{
					if (tracks[track]->i_type == libvlc_track_video)
					{
						mVideoWidth = tracks[track]->video->i_width;
						mVideoHeight = tracks[track]->video->i_height;
						break;
					}
				}
				libvlc_media_tracks_release(tracks, track_count);

				// Make sure we found a valid video track
				if ((mVideoWidth > 0) && (mVideoHeight > 0))
				{
					setupContext();

					// Setup the media player
					mMediaPlayer = libvlc_media_player_new_from_media(mMedia);
					libvlc_media_player_play(mMediaPlayer);
					libvlc_video_set_callbacks(mMediaPlayer, lock, unlock, display, (void*)&mContext);
					libvlc_video_set_format(mMediaPlayer, "RGBA", (int)mVideoWidth, (int)mVideoHeight, (int)mVideoWidth * 4);

					// Update the playing state
					mIsPlaying = true;
					mFadeIn = 0.0f;
				}
			}
		}
	}
}

void VideoComponent::startVideoWithDelay()
{
	// If not playing then either start the video or initiate the delay
	if (!mIsPlaying)
	{
		// Set the video that we are going to be playing so we don't attempt to restart it
		mPlayingVideoPath = mVideoPath;

		if (mConfig.startDelay == 0)
		{
			// No delay. Just start the video
			mStartDelayed = false;
			startVideo();
		}
		else
		{
			// Configure the start delay
			mStartDelayed = true;
			mFadeIn = 0.0f;
			mStartTime = SDL_GetTicks() + mConfig.startDelay;
		}
		mIsPlaying = true;
	}
}

void VideoComponent::stopVideo()
{
	mIsPlaying = false;
	mStartDelayed = false;
	// Release the media player so it stops calling back to us
	if (mMediaPlayer)
	{
		libvlc_media_player_stop(mMediaPlayer);
		libvlc_media_player_release(mMediaPlayer);
		libvlc_media_release(mMedia);
		mMediaPlayer = NULL;
		freeContext();
	}
}

void VideoComponent::update(int deltaTime)
{
	manageState();

	// If the video start is delayed and there is less than the fade time then set the image fade
	// accordingly
	if (mStartDelayed)
	{
		Uint32 ticks = SDL_GetTicks();
		if (mStartTime > ticks) 
		{
			Uint32 diff = mStartTime - ticks;
			if (diff < FADE_TIME_MS) 
			{
				mFadeIn = (float)diff / (float)FADE_TIME_MS;
				return;
			}
		}
	}
	// If the fade in is less than 1 then increment it
	if (mFadeIn < 1.0f)
	{
		mFadeIn += deltaTime / (float)FADE_TIME_MS;
		if (mFadeIn > 1.0f)
			mFadeIn = 1.0f;
	}
	GuiComponent::update(deltaTime);
}

void VideoComponent::manageState()
{
	// We will only show if the component is on display
	bool show = mShowing;

	// See if we're already playing
	if (mIsPlaying)
	{
		// If we are not on display then stop the video from playing
		if (!show)
		{
			stopVideo();
		}
		else
		{
			if (mVideoPath != mPlayingVideoPath)
			{
				// Path changed. Stop the video. We will start it again below because
				// mIsPlaying will be modified by stopVideo to be false
				stopVideo();
			}
		}
	}
	// Need to recheck variable rather than 'else' because it may be modified above
	if (!mIsPlaying)
	{
		// If we are on display then see if we should start the video
		if (show && !mVideoPath.empty())
		{
			startVideoWithDelay();
		}
	}
}

void VideoComponent::onShow()
{
	mShowing = true;
	manageState();
}

void VideoComponent::onHide()
{
	mShowing = false;
	manageState();
}


