#include "SystemScreenSaver.h"

#ifdef _OMX_
#include "components/VideoPlayerComponent.h"
#endif
#include "components/VideoVlcComponent.h"
#include "CollectionSystemManager.h"
#include "utils/FileSystemUtil.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "FileData.h"
#include "FileFilterIndex.h"
#include "Log.h"
#include "PowerSaver.h"
#include "Scripting.h"
#include "Sound.h"
#include "SystemData.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <time.h>
#include <unordered_map>

#define FADE_TIME 			300

static int lastIndex = 0;

SystemScreenSaver::SystemScreenSaver(Window* window) :
	mVideoScreensaver(NULL),
	mImageScreensaver(NULL),
	mWindow(window),
	mState(STATE_INACTIVE),
	mOpacity(0.0f),
	mTimer(0),
	mCurrentGame(NULL),
	mStopBackgroundAudio(true),
	mSystem(NULL)
{
	mWindow->setScreenSaver(this);
	std::string path = getTitleFolder();
	if(!Utils::FileSystem::exists(path))
		Utils::FileSystem::createDirectory(path);
	mSwapTimeout = 30000;
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

void SystemScreenSaver::setVideoScreensaver(std::string& path)
{
#ifdef _OMX_
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
}

void SystemScreenSaver::setImageScreensaver(std::string& path)
{
		if (!mImageScreensaver)
		{
			mImageScreensaver = new ImageComponent(mWindow, false, false);
		}

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

		PowerSaver::runningScreenSaver(true);
		mTimer = 0;
}

bool SystemScreenSaver::isFileVideo(std::string& path)
{
	if (path.empty()) {
		return false;
	}
	std::string pathFilter = Settings::getInstance()->getString("SlideshowScreenSaverVideoFilter");
	std::string pathExtension = path.substr(path.find_last_of("."));
	return pathFilter.find(pathExtension) != std::string::npos;
}

void SystemScreenSaver::startScreenSaver(SystemData* system)
{
	mSystem = system;
	// if set to index files in background, start thread
	if (Settings::getInstance()->getBool("BackgroundIndexing"))
	{
		mExit = false;
		mThread = new std::thread(&SystemScreenSaver::backgroundIndexing, this);
	}

	std::string screensaver_behavior = Settings::getInstance()->getString("ScreenSaverBehavior");
	if (!mVideoScreensaver && (screensaver_behavior == "random video"))
	{
		// Configure to fade out the windows, Skip Fading if Instant mode
		mState =  PowerSaver::getMode() == PowerSaver::INSTANT
					? STATE_SCREENSAVER_ACTIVE
					: STATE_FADE_OUT_WINDOW;
		mSwapTimeout = Settings::getInstance()->getInt("ScreenSaverSwapVideoTimeout");
		mOpacity = 0.0f;

		// Load a random video
		std::string path = "";
		pickRandomVideo(path);

		int retry = 200;
		while(retry > 0 && ((path.empty() || !Utils::FileSystem::exists(path)) || mCurrentGame == NULL))
		{
			retry--;
			pickRandomVideo(path);
		}

		if (!path.empty() && Utils::FileSystem::exists(path))
		{
			setVideoScreensaver(path);
			if (mCurrentGame != NULL)
			{
				Scripting::fireEvent("screensaver-game-select", mCurrentGame->getSystem()->getName(), mCurrentGame->getPath(), mCurrentGame->getName(), "randomvideo");
			}
			return;
		}
	}
	else if (screensaver_behavior == "slideshow")
	{
		// Configure to fade out the windows, Skip Fading if Instant mode
		mState =  PowerSaver::getMode() == PowerSaver::INSTANT
					? STATE_SCREENSAVER_ACTIVE
					: STATE_FADE_OUT_WINDOW;
		mSwapTimeout = Settings::getInstance()->getInt("ScreenSaverSwapMediaTimeout");
		mOpacity = 0.0f;

		// Load a random media
		std::string path = "";
		if (Settings::getInstance()->getBool("SlideshowScreenSaverCustomMediaSource"))
		{
			pickRandomCustomMedia(path);
			// Custom media are not tied to the game list
			mCurrentGame = NULL;
		}
		else
		{
			pickRandomGameListImage(path);
		}

		if (isFileVideo(path))
		{
			setVideoScreensaver(path);
		}
		else
		{
			setImageScreensaver(path);
		}

		std::string bg_audio_file = Settings::getInstance()->getString("SlideshowScreenSaverBackgroundAudioFile");
		if (!mBackgroundAudio && bg_audio_file != "" && Utils::FileSystem::exists(bg_audio_file))
		{
			// paused PS so that the background audio keeps playing
			PowerSaver::pause();
			mBackgroundAudio = Sound::get(bg_audio_file);
			mBackgroundAudio->play();
		}

		if (mCurrentGame != NULL)
		{
			Scripting::fireEvent("screensaver-game-select", mCurrentGame->getSystem()->getName(), mCurrentGame->getFileName(), mCurrentGame->getName(), "slideshow");
		}
		return;
	}
	// No videos. Just use a standard screensaver
	mState = STATE_SCREENSAVER_ACTIVE;
	mCurrentGame = NULL;
}

void SystemScreenSaver::stopScreenSaver(bool toResume)
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

	// if we're not changing videos or images, let's delete the random list
	if (!toResume) {
		mAllFiles.clear();
		mSystem = NULL;
	}

	// Exit the indexing thread in case it's running. Check if thread still exists.
	if (Settings::getInstance()->getBool("BackgroundIndexing") && mThread)
	{
		mExit = true;
		mThread->join();
		delete mThread;
		mThread = NULL;
	}

	// we need this to loop through different videos
	mState = STATE_INACTIVE;
	PowerSaver::runningScreenSaver(false);
}

void SystemScreenSaver::renderScreenSaver()
{
	std::string screensaver_behavior = Settings::getInstance()->getString("ScreenSaverBehavior");
	if (mVideoScreensaver && (screensaver_behavior == "random video" || screensaver_behavior == "slideshow"))
	{
		setBackground();

		// Only render the video if the state requires it
		if ((int)mState >= STATE_FADE_IN_VIDEO)
		{
			Transform4x4f transform = Transform4x4f::Identity();
			mVideoScreensaver->render(transform);
		}

		// Check if slideshow then loop background music
		if (screensaver_behavior == "slideshow" && mBackgroundAudio && !mBackgroundAudio->isPlaying())
		{
			mBackgroundAudio->play();
		}

	}
	else if (mImageScreensaver && screensaver_behavior == "slideshow")
	{
		setBackground();

		// Only render the image if the state requires it
		if ((int)mState >= STATE_FADE_IN_VIDEO && mImageScreensaver->hasImage())
		{
			mImageScreensaver->setOpacity(255- (unsigned char) (mOpacity * 255));

			Transform4x4f transform = Transform4x4f::Identity();
			mImageScreensaver->render(transform);
		}

		// Check if we need to restart the background audio
		if (mBackgroundAudio && !mBackgroundAudio->isPlaying())
		{
			mBackgroundAudio->play();
		}
	}
	else if (mState != STATE_INACTIVE)
	{
		Renderer::setMatrix(Transform4x4f::Identity());
		unsigned char color = screensaver_behavior == "dim" ? 0x000000A0 : 0x000000FF;
		Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(), Renderer::getScreenHeight(), color, color);
	}
}

void SystemScreenSaver::backgroundIndexing()
{
	LOG(LogDebug) << "Background indexing starting.";

	// get the list of all games
	SystemData* all = CollectionSystemManager::get()->getAllGamesCollection();
	std::vector<FileData*> files = all->getRootFolder()->getFilesRecursive(GAME);

	const auto startTs = std::chrono::system_clock::now();
	for ( ; lastIndex < files.size(); lastIndex++)
	{
		if(mExit)
			break;
		Utils::FileSystem::exists(files.at(lastIndex)->getVideoPath());
		Utils::FileSystem::exists(files.at(lastIndex)->getMarqueePath());
		Utils::FileSystem::exists(files.at(lastIndex)->getThumbnailPath());
		Utils::FileSystem::exists(files.at(lastIndex)->getImagePath());
	}
	auto endTs = std::chrono::system_clock::now();
	LOG(LogDebug) << "Indexed a total of " << lastIndex << " entries in " << std::chrono::duration_cast<std::chrono::milliseconds>(endTs - startTs).count() << " ms. Stopping.";
}

void SystemScreenSaver::getAllGamelistNodesForSystem(SystemData* system) {
	std::vector<FileData*> subsysFiles {};

	FileData* rootFileData = system->getRootFolder();
	subsysFiles = rootFileData->getFilesRecursive(FileType::GAME, true);
	mAllFiles.insert(mAllFiles.end(), subsysFiles.begin(), subsysFiles.end());
}

void SystemScreenSaver::getAllGamelistNodes()
{
	std::vector<FileData*> subsysFiles {};
	for (std::vector<SystemData*>::const_iterator it = SystemData::sSystemVector.cbegin(); it != SystemData::sSystemVector.cend(); ++it)
	{
		// We only want nodes from game systems that are not collections
		if (!(*it)->isGameSystem() || (*it)->isCollection())
			continue;

		getAllGamelistNodesForSystem(*it);
	}
}


void SystemScreenSaver::pickGameListNode(const char *nodeName, std::string& path)
{
	FileData *itf = nullptr;
	bool found =  false;
	int missCtr = 0;
	while (!found) {
		if (mAllFiles.empty()) {
			if (mSystem) {
				getAllGamelistNodesForSystem(mSystem);
			}
			else {
				getAllGamelistNodes();
			}
			if (mAllFiles.empty()) { return; } // no games at all
			mAllFilesSize = mAllFiles.size();
			std::shuffle(std::begin(mAllFiles), std::end(mAllFiles), SystemData::sURNG);
		}
		itf = mAllFiles.back();
		mAllFiles.pop_back();
		if ((strcmp(nodeName, "video") == 0 && itf->getVideoPath() != "") ||
			(strcmp(nodeName, "image") == 0 && itf->getImagePath() != ""))
		{
			found = true;
		} else {
			missCtr++;
			if (missCtr == mAllFilesSize) {
				// avoid looping forever when no candidate exist
				// with image/video path set
				return;
			}

		}
	}

	path = (strcmp(nodeName, "video") == 0) ? itf->getVideoPath() : itf->getImagePath();
	mCurrentGame = itf;

	if (Settings::getInstance()->getString("ScreenSaverGameInfo") != "never")
	{
		auto systemName = mCurrentGame->getSourceFileData()->getSystem()->getFullName();
		writeSubtitle(mCurrentGame->getSourceFileData()->getName().c_str(), systemName.c_str(),
			(Settings::getInstance()->getString("ScreenSaverGameInfo") == "always"));
	}
}

void SystemScreenSaver::pickRandomVideo(std::string& path)
{
	mCurrentGame = NULL;
	pickGameListNode("video", path);
}

void SystemScreenSaver::pickRandomGameListImage(std::string& path)
{
	mCurrentGame = NULL;
	pickGameListNode("image", path);
}

void SystemScreenSaver::pickRandomCustomMedia(std::string& path)
{
	if (mCustomMediaFiles.empty())
	{
		std::string mediaDir = Settings::getInstance()->getString("SlideshowScreenSaverMediaDir");
		if ((mediaDir != "") && (Utils::FileSystem::exists(mediaDir)))
		{
			mCustomMediaFiles = getCustomMediaFiles(mediaDir);
			if (mCustomMediaFiles.empty())
			{
				LOG(LogError) << "Slideshow Screensaver - No media files found\n";
				return;
			}
		}
		else
		{
			LOG(LogError) << "Slideshow Screensaver - Media directory does not exist: " << mediaDir << "\n";
			return;
		}
		std::shuffle(std::begin(mCustomMediaFiles), std::end(mCustomMediaFiles), SystemData::sURNG);
	}
	path = mCustomMediaFiles.back();
	mCustomMediaFiles.pop_back();
}


std::vector<std::string> SystemScreenSaver::getCustomMediaFiles(const std::string &mediaDir) {
	std::string imageFilter = Settings::getInstance()->getString("SlideshowScreenSaverImageFilter");
	std::string videoFilter = Settings::getInstance()->getString("SlideshowScreenSaverVideoFilter");
	std::vector<std::string> matchingFiles;
	Utils::FileSystem::stringList dirContent = Utils::FileSystem::getDirContent(mediaDir, Settings::getInstance()->getBool("SlideshowScreenSaverRecurse"));

	for(Utils::FileSystem::stringList::const_iterator it = dirContent.cbegin(); it != dirContent.cend(); ++it)
	{
		if (Utils::FileSystem::isRegularFile(*it))
		{
			// If the image filter is empty, or the file extension is in the filter string,
			//  add it to the matching files list
			if ((imageFilter.length() <= 0) ||
				(imageFilter.find(Utils::FileSystem::getExtension(*it)) != std::string::npos))
			{
				matchingFiles.push_back(*it);
			}
			// Also add video files
			if ((videoFilter.length() <= 0) ||
				(videoFilter.find(Utils::FileSystem::getExtension(*it)) != std::string::npos))
			{
				matchingFiles.push_back(*it);
			}
		}
	}
	return matchingFiles;
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
		// Update the timer that swaps the videos/images
		mTimer += deltaTime;
		if (mTimer > mSwapTimeout)
		{
			nextMediaItem();
		}
	}

	// If we have a loaded video/image then update it
	if (mVideoScreensaver)
		mVideoScreensaver->update(deltaTime);
	else if (mImageScreensaver)
		mImageScreensaver->update(deltaTime);
}

void SystemScreenSaver::nextMediaItem() {
	mStopBackgroundAudio = false;
	stopScreenSaver(true);
	startScreenSaver(mSystem);
	mState = STATE_SCREENSAVER_ACTIVE;
}

FileData* SystemScreenSaver::getCurrentGame()
{
	return mCurrentGame;
}

void SystemScreenSaver::selectGame(bool launch)
{
	if (mCurrentGame != NULL)
	{
		//Stop screensaver
		mStopBackgroundAudio = true;
		stopScreenSaver();

		// launching Game
		ViewController::get()->goToGameList(mCurrentGame->getSystem());
		IGameListView* view = ViewController::get()->getGameListView(mCurrentGame->getSystem()).get();
		view->setCursor(mCurrentGame);
		if (launch)
			view->launch(mCurrentGame);
	}
}

void SystemScreenSaver::setBackground()
{
		// Render black background
		Renderer::setMatrix(Transform4x4f::Identity());
		Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(), Renderer::getScreenHeight(), 0x000000FF, 0x000000FF);
}
