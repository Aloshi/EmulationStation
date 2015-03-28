#pragma once

#include "resources/ResourceManager.h"

#include <Eigen/Dense>
#include <string>
#include "platform.h"
#include GLHEADER

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
}

class VideoResource /*: public IReloadable */
{
public:
	VideoResource();
	virtual ~VideoResource();

	void open(const std::string& path);
	void advance(unsigned int ms);

	inline const Eigen::Vector2i& getTextureSize() { return mTextureSize; }
	GLuint getFrameTexture();

	void close();

private:
	static bool sInitializedLibrary;
	static void loadLibrary();

	int64_t msToNextFrame() const;
	void readNextFrame();

	int64_t mTimeAccumulator;
	AVFormatContext* mFormatCtx;
	AVStream* mVideoStream;
	AVCodec* mCodec;

	AVPacket mPacket;
	int mPacketDataOffset;

	GLuint mTexture;
	Eigen::Vector2i mTextureSize;
};