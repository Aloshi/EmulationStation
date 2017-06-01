#include "SystemScreenSaver.h"
#ifdef _RPI_
#include "components/VideoPlayerComponent.h"
#endif
#include "components/VideoVlcComponent.h"
#include "platform.h"
#include "Renderer.h"
#include "Settings.h"
#include "SystemData.h"
#include "Util.h"
#include "Log.h"
#include "views/ViewController.h"
#include "views/gamelist/IGameListView.h"
#include <stdio.h>

#define FADE_TIME 			300
#define SWAP_VIDEO_TIMEOUT	30000

SystemScreenSaver::SystemScreenSaver(Window* window) :
	mVideoScreensaver(NULL),
	mWindow(window),
	mCounted(false),
	mVideoCount(0),
	mState(STATE_INACTIVE),
	mOpacity(0.0f),
	mTimer(0),
	mSystemName(""),
	mGameName(""),
	mCurrentGame(NULL)
{
	mWindow->setScreenSaver(this);
	std::string path = getTitleFolder();
	if(!boost::filesystem::exists(path))
		boost::filesystem::create_directory(path);
	srand((unsigned int)time(NULL));
}

SystemScreenSaver::~SystemScreenSaver()
{
	// Delete subtitle file, if existing
	remove(getTitlePath().c_str());
	mCurrentGame = NULL;
	delete mVideoScreensaver;
}

bool SystemScreenSaver::allowSleep()
{
	//return false;
	return (mVideoScreensaver == NULL);
}

bool SystemScreenSaver::isScreenSaverActive()
{
	return (mState != STATE_INACTIVE);
}

void SystemScreenSaver::startScreenSaver()
{
	if (!mVideoScreensaver && (Settings::getInstance()->getString("ScreenSaverBehavior") == "random video"))
	{
		// Configure to fade out the windows
		mState = STATE_FADE_OUT_WINDOW;
		mOpacity = 0.0f;

		// Load a random video
		std::string path = "";
		pickRandomVideo(path);

		int retry = 200;
		while(retry > 0 && ((path.empty() || !boost::filesystem::exists(path)) || mCurrentGame == NULL))
		{
			retry--;
			pickRandomVideo(path);
		}

		if (!path.empty() && boost::filesystem::exists(path))
		{
		// Create the correct type of video component

#ifdef _RPI_
			if (Settings::getInstance()->getBool("ScreenSaverOmxPlayer"))
				mVideoScreensaver = new VideoPlayerComponent(mWindow, getTitlePath());
			else
				mVideoScreensaver = new VideoVlcComponent(mWindow, getTitlePath());
#else
			mVideoScreensaver = new VideoVlcComponent(mWindow, getTitlePath());
#endif

			mVideoScreensaver->setOrigin(0.5f, 0.5f);
			mVideoScreensaver->setPosition(Renderer::getScreenWidth()/2, Renderer::getScreenHeight()/2);

			if (Settings::getInstance()->getBool("StretchVideoOnScreenSaver"))
			{
				mVideoScreensaver->setResize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
			}
			else
			{
				mVideoScreensaver->setMaxSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
			}
			mVideoScreensaver->setVideo(path);
			mVideoScreensaver->setScreensaverMode(true);
			mVideoScreensaver->onShow();
			mTimer = 0;
			return;
		}
	}
	// No videos. Just use a standard screensaver
	mState = STATE_SCREENSAVER_ACTIVE;
	mCurrentGame = NULL;
}

void SystemScreenSaver::stopScreenSaver()
{
	delete mVideoScreensaver;
	mVideoScreensaver = NULL;
	mState = STATE_INACTIVE;
}

void SystemScreenSaver::renderScreenSaver()
{
	if (mVideoScreensaver && Settings::getInstance()->getString("ScreenSaverBehavior") == "random video")
	{
		// Render black background
		Renderer::setMatrix(Eigen::Affine3f::Identity());
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), (unsigned char)(255));

		// Only render the video if the state requires it
		if ((int)mState >= STATE_FADE_IN_VIDEO)
		{
			Eigen::Affine3f transform = Eigen::Affine3f::Identity();
			mVideoScreensaver->render(transform);
		}
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
	mCurrentGame = NULL;
	if (mVideoCount > 0)
	{
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
							mSystemName = (*it)->getFullName();
							mGameName = fileNode.child("name").text().get();

							// getting corresponding FileData

							// try the easy way. Should work for the majority of cases, unless in subfolders
							FileData* rootFileData = (*it)->getRootFolder();
							std::string gamePath = resolvePath(fileNode.child("path").text().get(), (*it)->getStartPath(), false).string();

							std::string shortPath = gamePath;
							shortPath = shortPath.replace(0, (*it)->getStartPath().length()+1, "");

							const std::unordered_map<std::string, FileData*>& children = rootFileData->getChildrenByFilename();
							std::unordered_map<std::string, FileData*>::const_iterator screenSaverGame = children.find(shortPath);

							if (screenSaverGame != children.end())
							{
								// Found the corresponding FileData
								mCurrentGame = screenSaverGame->second;
							}
							else
							{
								// Couldn't find FileData. Going for the full iteration.
								// iterate on children
								FileType type = GAME;
								std::vector<FileData*> allFiles = rootFileData->getFilesRecursive(type);
								std::vector<FileData*>::iterator itf;  // declare an iterator to a vector of strings

								int i = 0;
								for(itf=allFiles.begin() ; itf < allFiles.end(); itf++,i++ ) {
									if ((*itf)->getPath() == gamePath)
									{
										mCurrentGame = (*itf);
										break;
									}
								}
							}

							// end of getting FileData
							if (Settings::getInstance()->getString("ScreenSaverGameInfo") != "never")
								writeSubtitle(mGameName.c_str(), mSystemName.c_str(),
									(Settings::getInstance()->getString("ScreenSaverGameInfo") == "always"));
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
			nextVideo();
		}
	}

	// If we have a loaded video then update it
	if (mVideoScreensaver)
		mVideoScreensaver->update(deltaTime);
}

void SystemScreenSaver::nextVideo() {
	stopScreenSaver();
	startScreenSaver();
	mState = STATE_SCREENSAVER_ACTIVE;
}

FileData* SystemScreenSaver::getCurrentGame()
{
	return mCurrentGame;
}

void SystemScreenSaver::launchGame()
{
	// launching Game
	ViewController::get()->goToGameList(mCurrentGame->getSystem());
	IGameListView* view = ViewController::get()->getGameListView(mCurrentGame->getSystem()).get();
 	view->setCursor(mCurrentGame);
 	if (Settings::getInstance()->getBool("ScreenSaverControls"))
 	{
 		view->launch(mCurrentGame);
 	}
}