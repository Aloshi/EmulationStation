#pragma once
#ifndef ES_APP_SYSTEM_SCREEN_SAVER_H
#define ES_APP_SYSTEM_SCREEN_SAVER_H

#include "Window.h"
#include <thread>

class ImageComponent;
class Sound;
class VideoComponent;

// Screensaver implementation for main window
class SystemScreenSaver : public Window::ScreenSaver
{
public:
	SystemScreenSaver(Window* window);
	virtual ~SystemScreenSaver();

	virtual void startScreenSaver();
	virtual void stopScreenSaver();
	virtual void nextMediaItem();
	virtual void renderScreenSaver();
	virtual bool allowSleep();
	virtual void update(int deltaTime);
	virtual bool isScreenSaverActive();

	virtual FileData* getCurrentGame();
	virtual void launchGame();

private:
	void pickGameListNode(const char *nodeName, std::string& path);
	void pickRandomVideo(std::string& path);
	void pickRandomGameListImage(std::string& path);
	void pickRandomCustomMedia(std::string& path);
	void setVideoScreensaver(std::string& path);
	void setImageScreensaver(std::string& path);
	bool isFileVideo(std::string& path);
	std::vector<std::string> getCustomMediaFiles(const std::string &mediaDir);
	std::vector<FileData*> getAllGamelistNodes();
	void backgroundIndexing();
	void setBackground();
	void input(InputConfig* config, Input input);

	enum STATE {
		STATE_INACTIVE,
		STATE_FADE_OUT_WINDOW,
		STATE_FADE_IN_VIDEO,
		STATE_SCREENSAVER_ACTIVE
	};

private:
	VideoComponent*		mVideoScreensaver;
	ImageComponent*		mImageScreensaver;
	Window*			mWindow;
	STATE			mState;
	float			mOpacity;
	int			mTimer;
	FileData*		mCurrentGame;
	int			mSwapTimeout;
	std::shared_ptr<Sound>	mBackgroundAudio;
	bool			mStopBackgroundAudio;
	std::vector<FileData*>	mAllFiles;
	std::vector<std::string> mCustomMediaFiles;
	int			mAllFilesSize;
	std::thread*		mThread;
	bool			mExit;
};

#endif // ES_APP_SYSTEM_SCREEN_SAVER_H
