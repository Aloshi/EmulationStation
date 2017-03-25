#ifdef _RPI_
#include "components/VideoPlayerComponent.h"
#include "Renderer.h"
#include "ThemeData.h"
#include "Settings.h"
#include "Util.h"
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

VideoPlayerComponent::VideoPlayerComponent(Window* window) :
	VideoComponent(window),
	mPlayerPid(-1)
{
}

VideoPlayerComponent::~VideoPlayerComponent()
{
}

void VideoPlayerComponent::render(const Eigen::Affine3f& parentTrans)
{
	VideoComponent::render(parentTrans);
}

void VideoPlayerComponent::setResize(float width, float height)
{
	setSize(width, height);
	mTargetSize << width, height;
	mTargetIsMax = false;
	mStaticImage.setSize(width, height);
	onSizeChanged();
}

void VideoPlayerComponent::setMaxSize(float width, float height)
{
	setSize(width, height);
	mTargetSize << width, height;
	mTargetIsMax = true;
	mStaticImage.setMaxSize(width, height);
	onSizeChanged();
}

void VideoPlayerComponent::startVideo()
{
	if (!mIsPlaying) {
		mVideoWidth = 0;
		mVideoHeight = 0;

		std::string path(mVideoPath.c_str());

		// Make sure we have a video path
		if ((path.size() > 0) && (mPlayerPid == -1))
		{
			// Set the video that we are going to be playing so we don't attempt to restart it
			mPlayingVideoPath = mVideoPath;

			// Start the player process
			pid_t pid = fork();
			if (pid == -1)
			{
				// Failed
				mPlayingVideoPath = "";
			}
			else if (pid > 0)
			{
				mPlayerPid = pid;
				// Update the playing state
				signal(SIGCHLD, catch_child);
				mIsPlaying = true;
				mFadeIn = 0.0f;
			}
			else
			{

				// Find out the pixel position of the video view and build a command line for
				// omxplayer to position it in the right place
				char buf[32];
				float x = mPosition.x() - (mOrigin.x() * mSize.x());
				float y = mPosition.y() - (mOrigin.y() * mSize.y());
				sprintf(buf, "%d,%d,%d,%d", (int)x, (int)y, (int)(x + mSize.x()), (int)(y + mSize.y()));
				// We need to specify the layer of 10000 or above to ensure the video is displayed on top
				// of our SDL display

				const char* argv[] = { "", "--layer", "10010", "--loop", "--no-osd", "--aspect-mode", "letterbox", "--vol", "0", "--win", buf, "-b", "", "", "", "", NULL };

				// check if we want to mute the audio
				if (!Settings::getInstance()->getBool("VideoAudio"))
				{
					argv[8] = "-1000000";
				}

				// if we are rendering a video gamelist
				if (!mTargetIsMax)
				{
					argv[6] = "stretch";
				}

				argv[11] = mPlayingVideoPath.c_str();

				const char* env[] = { "LD_LIBRARY_PATH=/opt/vc/libs:/usr/lib/omxplayer", NULL };

				// Redirect stdout
				int fdin = open("/dev/null", O_RDONLY);
				int fdout = open("/dev/null", O_WRONLY);
				dup2(fdin, 0);
				dup2(fdout, 1);
				// Run the omxplayer binary
				execve("/usr/bin/omxplayer.bin", (char**)argv, (char**)env);

				_exit(EXIT_FAILURE);
			}
		}
	}
}

void catch_child(int sig_num)
{
    /* when we get here, we know there's a zombie child waiting */
    int child_status;
    wait(&child_status);
}

void VideoPlayerComponent::stopVideo()
{
	mIsPlaying = false;
	mStartDelayed = false;

	// Stop the player process
	if (mPlayerPid != -1)
	{
		int status;
		kill(mPlayerPid, SIGKILL);
		waitpid(mPlayerPid, &status, WNOHANG);
		mPlayerPid = -1;
	}
}

#endif

