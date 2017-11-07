#include "SystemScreenSaver.h"

#ifdef _RPI_
#include "components/VideoPlayerComponent.h"
#endif
#include "components/VideoVlcComponent.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "FileData.h"
#include "FileFilterIndex.h"
#include "Log.h"
#include "PowerSaver.h"
#include "Renderer.h"
#include "Sound.h"
#include "SystemData.h"
#include "Util.h"
#include <boost/filesystem/operations.hpp>
#include <unordered_map>

#define FADE_TIME 			300

SystemScreenSaver::SystemScreenSaver(Window* window) :
	mVideoScreensaver(NULL),
	mImageScreensaver(NULL),
	mWindow(window),
	mVideosCounted(false),
	mVideoCount(0),
	mImagesCounted(false),
	mImageCount(0),
	mState(STATE_INACTIVE),
	mOpacity(0.0f),
	mTimer(0),
	mSystemName(""),
	mGameName(""),
	mCurrentGame(NULL),
	mStopBackgroundAudio(true)
{
	mWindow->setScreenSaver(this);
	std::string path = getTitleFolder();
	if(!boost::filesystem::exists(path))
		boost::filesystem::create_directory(path);
	srand((unsigned int)time(NULL));
	mVideoChangeTime = 30000;
}

SystemScreenSaver::~SystemScreenSaver()
{
	// Delete subtitle file, if existing
	remove(getTitlePath().c_str());
	mCurrentGame = NULL;
	delete mVideoScreensaver;
	delete mImageScreensaver;
}

bool SystemScreenSaver::allowSleep()
{
	//return false;
	return ((mVideoScreensaver == NULL) && (mImageScreensaver == NULL));
}

bool SystemScreenSaver::isScreenSaverActive()
{
	return (mState != STATE_INACTIVE);
}

void SystemScreenSaver::startScreenSaver()
{
	std::string screensaver_behavior = Settings::getInstance()->getString("ScreenSaverBehavior");
	if (!mVideoScreensaver && (screensaver_behavior == "random video"))
	{
		// Configure to fade out the windows, Skip Fading if Instant mode
		mState =  PowerSaver::getMode() == PowerSaver::INSTANT
					? STATE_SCREENSAVER_ACTIVE
					: STATE_FADE_OUT_WINDOW;
		mVideoChangeTime = Settings::getInstance()->getInt("ScreenSaverSwapVideoTimeout");
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
#ifdef _RPI_
			// Create the correct type of video component
			if (Settings::getInstance()->getBool("ScreenSaverOmxPlayer"))
				mVideoScreensaver = new VideoPlayerComponent(mWindow, getTitlePath());
			else
				mVideoScreensaver = new VideoVlcComponent(mWindow, getTitlePath());
#else
			mVideoScreensaver = new VideoVlcComponent(mWindow, getTitlePath());
#endif

			mVideoScreensaver->topWindow(true);
			mVideoScreensaver->setOrigin(0.5f, 0.5f);
			mVideoScreensaver->setPosition(Renderer::getScreenWidth() / 2.0f, Renderer::getScreenHeight() / 2.0f);

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
			PowerSaver::runningScreenSaver(true);
			mTimer = 0;
			return;
		}
	}
	else if (screensaver_behavior == "slideshow")
	{
		// Configure to fade out the windows, Skip Fading if Instant mode
		mState =  PowerSaver::getMode() == PowerSaver::INSTANT
					? STATE_SCREENSAVER_ACTIVE
					: STATE_FADE_OUT_WINDOW;
		mVideoChangeTime = Settings::getInstance()->getInt("ScreenSaverSwapImageTimeout");
		mOpacity = 0.0f;

		// Load a random image
		std::string path = "";
		if (Settings::getInstance()->getBool("SlideshowScreenSaverCustomImageSource"))
		{
			pickRandomCustomImage(path);
			// Custom images are not tied to the game list
			mCurrentGame = NULL;
		}
		else
		{
			pickRandomGameListImage(path);
		}

		if (!mImageScreensaver)
		{
			mImageScreensaver = new ImageComponent(mWindow, false, false);
		}

		mTimer = 0;

		mImageScreensaver->setImage(path);
		mImageScreensaver->setOrigin(0.5f, 0.5f);
		mImageScreensaver->setPosition(Renderer::getScreenWidth() / 2.0f, Renderer::getScreenHeight() / 2.0f);

		if (Settings::getInstance()->getBool("SlideshowScreenSaverStretch"))
		{
			mImageScreensaver->setResize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
		}
		else
		{
			mImageScreensaver->setMaxSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
		}

		std::string bg_audio_file = Settings::getInstance()->getString("SlideshowScreenSaverBackgroundAudioFile");
		if ((!mBackgroundAudio) && (bg_audio_file != ""))
		{
			if (boost::filesystem::exists(bg_audio_file))
			{
				// paused PS so that the background audio keeps playing
				PowerSaver::pause();
				mBackgroundAudio = Sound::get(bg_audio_file);
				mBackgroundAudio->play();
			}
		}

		PowerSaver::runningScreenSaver(true);
		mTimer = 0;
		return;
	}
	// No videos. Just use a standard screensaver
	mState = STATE_SCREENSAVER_ACTIVE;
	mCurrentGame = NULL;
}

void SystemScreenSaver::stopScreenSaver()
{
	if ((mBackgroundAudio) && (mStopBackgroundAudio))
	{
		mBackgroundAudio->stop();
		mBackgroundAudio.reset();
		// if we were playing audio, we paused PS
		PowerSaver::resume();
	}

	// so that we stop the background audio next time, unless we're restarting the screensaver
	mStopBackgroundAudio = true;

	delete mVideoScreensaver;
	mVideoScreensaver = NULL;
	delete mImageScreensaver;
	mImageScreensaver = NULL;

	// we need this to loop through different videos
	mState = STATE_INACTIVE;
	PowerSaver::runningScreenSaver(false);
}

void SystemScreenSaver::renderScreenSaver()
{
	std::string screensaver_behavior = Settings::getInstance()->getString("ScreenSaverBehavior");
	if (mVideoScreensaver && screensaver_behavior == "random video")
	{
		// Render black background
		Renderer::setMatrix(Transform4x4f::Identity());
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), (unsigned char)(255));

		// Only render the video if the state requires it
		if ((int)mState >= STATE_FADE_IN_VIDEO)
		{
			Transform4x4f transform = Transform4x4f::Identity();
			mVideoScreensaver->render(transform);
		}
	}
	else if (mImageScreensaver && screensaver_behavior == "slideshow")
	{
		// Render black background
		Renderer::setMatrix(Transform4x4f::Identity());
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), (unsigned char)(255));

		// Only render the video if the state requires it
		if ((int)mState >= STATE_FADE_IN_VIDEO)
		{
			if (mImageScreensaver->hasImage())
			{
				mImageScreensaver->setOpacity(255- (unsigned char) (mOpacity * 255));

				Transform4x4f transform = Transform4x4f::Identity();
				mImageScreensaver->render(transform);
			}
		}

		// Check if we need to restart the background audio
		if ((mBackgroundAudio) && (Settings::getInstance()->getString("SlideshowScreenSaverBackgroundAudioFile") != ""))
		{
			if (!mBackgroundAudio->isPlaying())
			{
				mBackgroundAudio->play();
			}
		}
	}
	else if (mState != STATE_INACTIVE)
	{
		Renderer::setMatrix(Transform4x4f::Identity());
		unsigned char opacity = screensaver_behavior == "dim" ? 0xA0 : 0xFF;
		Renderer::drawRect(0, 0, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x00000000 | opacity);
	}
}

unsigned long SystemScreenSaver::countGameListNodes(const char *nodeName)
{
	unsigned long nodeCount = 0;
	std::vector<SystemData*>:: iterator it;
	for (it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); ++it)
	{
		// We only want images and videos from game systems that are not collections
		if (!(*it)->isCollection() && (*it)->isGameSystem())
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
					pugi::xml_node node = fileNode.child(nodeName);
					if (node)
						++nodeCount;
				}
			}
		}
	}
	return nodeCount;
}

void SystemScreenSaver::countVideos()
{
	if (!mVideosCounted)
	{
		mVideoCount = countGameListNodes("video");
		mVideosCounted = true;
	}
}

void SystemScreenSaver::countImages()
{
	if (!mImagesCounted)
	{
		mImageCount = countGameListNodes("image");
		mImagesCounted = true;
	}
}

void SystemScreenSaver::pickGameListNode(unsigned long index, const char *nodeName, std::string& path)
{
	std::vector<SystemData*>:: iterator it;
	for (it = SystemData::sSystemVector.begin(); it != SystemData::sSystemVector.end(); ++it)
	{
		pugi::xml_document doc;
		pugi::xml_node root;

		// We only want nodes from game systems that are not collections
		if (!(*it)->isGameSystem() || (*it)->isCollection())
			continue;

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
				pugi::xml_node node = fileNode.child(nodeName);
				if (node)
				{
					// See if this is the desired index
					if (index-- == 0)
					{
						// Yes. Resolve to a full path
						path = resolvePath(node.text().get(), (*it)->getStartPath(), true).generic_string();
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

void SystemScreenSaver::pickRandomVideo(std::string& path)
{
	countVideos();
	mCurrentGame = NULL;
	if (mVideoCount > 0)
	{
		int video = (int)(((float)rand() / float(RAND_MAX)) * (float)mVideoCount);

		pickGameListNode(video, "video", path);
	}
}

void SystemScreenSaver::pickRandomGameListImage(std::string& path)
{
	countImages();
	mCurrentGame = NULL;
	if (mImageCount > 0)
	{
		int image = (int)(((float)rand() / float(RAND_MAX)) * (float)mImageCount);

		pickGameListNode(image, "image", path);
	}
}

void SystemScreenSaver::pickRandomCustomImage(std::string& path)
{
	std::string imageDir = Settings::getInstance()->getString("SlideshowScreenSaverImageDir");
	if ((imageDir != "") && (boost::filesystem::exists(imageDir)))
	{
		std::string imageFilter = Settings::getInstance()->getString("SlideshowScreenSaverImageFilter");

		std::vector<std::string> matchingFiles;

		if (Settings::getInstance()->getBool("SlideshowScreenSaverRecurse"))
		{
			boost::filesystem::recursive_directory_iterator end_iter;
			boost::filesystem::recursive_directory_iterator iter(imageDir);

			// TODO: Figure out how to remove this duplication in the else block
			for (iter; iter != end_iter; ++iter)
			{
				if (boost::filesystem::is_regular_file(iter->status()))
				{
					// If the image filter is empty, or the file extension is in the filter string,
					//  add it to the matching files list
					if ((imageFilter.length() <= 0) ||
						(imageFilter.find(iter->path().extension().string()) != std::string::npos))
					{
						matchingFiles.push_back(iter->path().string());
					}
				}
			}
		}
		else
		{
			boost::filesystem::directory_iterator end_iter;
			boost::filesystem::directory_iterator iter(imageDir);

			for (iter; iter != end_iter; ++iter)
			{
				if (boost::filesystem::is_regular_file(iter->status()))
				{
					// If the image filter is empty, or the file extension is in the filter string,
					//  add it to the matching files list
					if ((imageFilter.length() <= 0) ||
						(imageFilter.find(iter->path().extension().string()) != std::string::npos))
					{
						matchingFiles.push_back(iter->path().string());
					}
				}
			}
		}

		int fileCount = matchingFiles.size();
		if (fileCount > 0)
		{
			// get a random index in the range 0 to fileCount (exclusive)
			int randomIndex = rand() % fileCount;
			path = matchingFiles[randomIndex];
		}
		else
		{
			LOG(LogError) << "Slideshow Screensaver - No image files found\n";
		}
	}
	else
	{
		LOG(LogError) << "Slideshow Screensaver - Image directory does not exist: " << imageDir << "\n";
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
		if (mTimer > mVideoChangeTime)
		{
			nextVideo();
		}
	}

	// If we have a loaded video then update it
	if (mVideoScreensaver)
		mVideoScreensaver->update(deltaTime);
	if (mImageScreensaver)
		mImageScreensaver->update(deltaTime);
}

void SystemScreenSaver::nextVideo() {
	mStopBackgroundAudio = false;
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
	if (mCurrentGame != NULL)
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
}
