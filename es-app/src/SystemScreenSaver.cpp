#include "SystemScreenSaver.h"
#ifdef _RPI_
#include "components/VideoPlayerComponent.h"
#endif
#include "components/VideoVlcComponent.h"
#include "Renderer.h"
#include "Settings.h"
#include "SystemData.h"
#include "Util.h"

#define FADE_TIME 			3000
#define SWAP_VIDEO_TIMEOUT	30000

SystemScreenSaver::SystemScreenSaver(Window* window) :
	mVideoScreensaver(NULL),
	mWindow(window),
	mCounted(false),
	mVideoCount(0),
	mState(STATE_INACTIVE),
	mOpacity(0.0f),
	mTimer(0)
{
	mWindow->setScreenSaver(this);
}

SystemScreenSaver::~SystemScreenSaver()
{
	delete mVideoScreensaver;
}

bool SystemScreenSaver::allowSleep()
{
	return false;
}

void SystemScreenSaver::startScreenSaver()
{
	if (!mVideoScreensaver && (Settings::getInstance()->getString("ScreenSaverBehavior") == "random video"))
	{
		// Configure to fade out the windows
		mState = STATE_FADE_OUT_WINDOW;
		mOpacity = 0.0f;

		// Load a random video
		std::string path;
		pickRandomVideo(path);
		if (!path.empty())
		{
	// Create the correct type of video component
#ifdef _RPI_
			if (Settings::getInstance()->getBool("VideoOmxPlayer"))
				mVideoScreensaver = new VideoPlayerComponent(mWindow);
			else
				mVideoScreensaver = new VideoVlcComponent(mWindow);
#else
			mVideoScreensaver = new VideoVlcComponent(mWindow);
#endif


			mVideoScreensaver->setOrigin(0.0f, 0.0f);
			mVideoScreensaver->setPosition(0.0f, 0.0f);
			mVideoScreensaver->setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
			mVideoScreensaver->setVideo(path);
			mVideoScreensaver->onShow();
			mTimer = 0;
		}
		else
		{
			// No videos. Just use a standard screensaver
			mState = STATE_SCREENSAVER_ACTIVE;
		}
	}
}

void SystemScreenSaver::stopScreenSaver()
{
	delete mVideoScreensaver;
	mVideoScreensaver = NULL;
	mState = STATE_INACTIVE;
}

void SystemScreenSaver::renderScreenSaver()
{
	if (mVideoScreensaver)
	{
		// Only render the video if the state requires it
		if ((int)mState >= STATE_FADE_IN_VIDEO)
		{
			Eigen::Affine3f transform = Eigen::Affine3f::Identity();
			mVideoScreensaver->render(transform);
		}
		// Handle any fade
		Renderer::setMatrix(Eigen::Affine3f::Identity());
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), (unsigned char)(mOpacity * 255));
	}
	else if (mState != STATE_INACTIVE)
	{
		Renderer::setMatrix(Eigen::Affine3f::Identity());
		unsigned char opacity = Settings::getInstance()->getString("ScreenSaverBehavior") == "dim" ? 0xA0 : 0xFF;
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x00000000 | opacity);
	}
}

void SystemScreenSaver::countVideos()
{
	if (!mCounted)
	{
		mVideoCount = 0;
		mCounted = true;
		std::vector<SystemData*>:: iterator it;
		for (it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); ++it)
		{
			pugi::xml_document doc;
			pugi::xml_node root;
			std::string xmlReadPath = (*it)->getGamelistPath(false);

			if(boost::filesystem::exists(xmlReadPath))
			{
				pugi::xml_parse_result result = doc.load_file(xmlReadPath.c_str());
				if (!result)
					continue;
				root = doc.child("gameList");
				if (!root)
					continue;
				for(pugi::xml_node fileNode = root.child("game"); fileNode; fileNode = fileNode.next_sibling("game"))
				{
					pugi::xml_node videoNode = fileNode.child("video");
					if (videoNode)
						++mVideoCount;
				}
			}
		}
	}
}

void SystemScreenSaver::pickRandomVideo(std::string& path)
{
	countVideos();
	if (mVideoCount > 0)
	{
		srand((unsigned int)time(NULL));
		int video = (int)(((float)rand() / float(RAND_MAX)) * (float)mVideoCount);

		std::vector<SystemData*>:: iterator it;
		for (it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); ++it)
		{
			pugi::xml_document doc;
			pugi::xml_node root;
			std::string xmlReadPath = (*it)->getGamelistPath(false);

			if(boost::filesystem::exists(xmlReadPath))
			{
				pugi::xml_parse_result result = doc.load_file(xmlReadPath.c_str());
				if (!result)
					continue;
				root = doc.child("gameList");
				if (!root)
					continue;
				for(pugi::xml_node fileNode = root.child("game"); fileNode; fileNode = fileNode.next_sibling("game"))
				{
					pugi::xml_node videoNode = fileNode.child("video");
					if (videoNode)
					{
						// See if this is the randomly selected video
						if (video-- == 0)
						{
							// Yes. Resolve to a full path
							path = resolvePath(videoNode.text().get(), (*it)->getStartPath(), true).generic_string();
							return;
						}
					}
				}
			}
		}
	}
}

void SystemScreenSaver::update(int deltaTime)
{
	// Use this to update the fade value for the current fade stage
	if (mState == STATE_FADE_OUT_WINDOW)
	{
		mOpacity += (float)deltaTime / FADE_TIME;
		if (mOpacity >= 1.0f)
		{
			mOpacity = 1.0f;

			// Update to the next state
			mState = STATE_FADE_IN_VIDEO;
		}
	}
	else if (mState == STATE_FADE_IN_VIDEO)
	{
		mOpacity -= (float)deltaTime / FADE_TIME;
		if (mOpacity <= 0.0f)
		{
			mOpacity = 0.0f;
			// Update to the next state
			mState = STATE_SCREENSAVER_ACTIVE;
		}
	}
	else if (mState == STATE_SCREENSAVER_ACTIVE)
	{
		// Update the timer that swaps the videos
		mTimer += deltaTime;
		if (mTimer > SWAP_VIDEO_TIMEOUT)
		{
			stopScreenSaver();
			startScreenSaver();
			mState = STATE_SCREENSAVER_ACTIVE;
		}
	}

	// If we have a loaded video then update it
	if (mVideoScreensaver)
		mVideoScreensaver->update(deltaTime);
}
