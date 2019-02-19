#include "VideoResource.h"
#include "Log.h"

#include <assert.h>

// This file is heavily based on this tutorial:
// http://blog.tomaka17.com/2012/03/libavcodeclibavformat-tutorial/

bool VideoResource::sInitializedLibrary = false;

void ffmpeg_log(void* ptr, int level, const char* fmt, va_list vl)
{
	const size_t BUFF_SIZE = 1024 * 2;
	char line[BUFF_SIZE];
	int printBuffer = 1;
	av_log_format_line(ptr, level, fmt, vl, line, BUFF_SIZE, &printBuffer);
	LOG(LogInfo) << "(ffmpeg " << level << "): " << line;
}

void LogFFmpegError(int errorCode)
{
	static char err[1024];
	LOG(LogError) << "error processing frame?! " << av_make_error_string(err, 1024, errorCode);
}

void VideoResource::loadLibrary()
{
	assert(!sInitializedLibrary);
	av_register_all();

	av_log_set_callback(&ffmpeg_log);

	sInitializedLibrary = true;
}

VideoResource::VideoResource()
{
	if(!sInitializedLibrary)
		loadLibrary();

	mFormatCtx = NULL;
	mVideoStream = NULL;
	mCodec = NULL;
	mTexture = NULL;
	mTimeAccumulator = 0;

	av_init_packet(&mPacket);
	mPacket.data = NULL;
	mPacketDataOffset = 0;
}

void VideoResource::open(const std::string& path)
{
	close();

	int err;
	err = avformat_open_input(&mFormatCtx, path.c_str(), NULL, NULL); // expects mFormatCtx to be NULL, will allocate it
	assert(err == 0);
	err = avformat_find_stream_info(mFormatCtx, NULL);
	assert(err >= 0);

	LOG(LogInfo) << "Opening video file " << path << ", which has " << mFormatCtx->nb_streams << " streams";
	for(unsigned int i = 0; i < mFormatCtx->nb_streams; i++)
	{
		AVStream* stream = mFormatCtx->streams[i];
		if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			LOG(LogInfo) << "Stream " << i << " has a type of video, so decoding this one";
			mVideoStream = stream;
			break;
		}
	}

	assert(mVideoStream);

	// find the decoder
	AVCodecContext* codecContext = mVideoStream->codec;
	mCodec = avcodec_find_decoder(codecContext->codec_id);
	assert(mCodec != NULL); // could not find decoder
	err = avcodec_open2(codecContext, mCodec, NULL);
	assert(err >= 0);

	// generate OpenGL texture based on the resolution of the video
	mTextureSize << mVideoStream->codec->width, mVideoStream->codec->height;

	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mTextureSize.x(), mTextureSize.y(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// get the first frame
	readNextFrame();
}

// This function is kind of ugly.
// avcodec_decode_video returns how many bytes of the packet were processed.
// This can be less than the actual size of the packet, in which case we need
// to send the remaining bytes in the packet into the next decode_video call.
void VideoResource::readNextFrame()
{
	AVFrame* frame = av_frame_alloc();

	// read packets until we have the next video frame decoded
	while(true)
	{
		// while the current packet is exhausted OR
		// the current packet does not pertain to a stream that we care about
		while(mPacketDataOffset >= mPacket.size || mPacket.stream_index != mVideoStream->index)
		{
			// done with this packet
			av_free_packet(&mPacket);
			mPacketDataOffset = 0;

			// get the next one
			int err = av_read_frame(mFormatCtx, &mPacket);
			if(err < 0)
			{
				LOG(LogInfo) << "av_read_frame returned " << err << ", EOF?";
				av_frame_free(&frame);
				return;
			}
		}

		// we've got new data in our packet,
		// try to decode a frame of video from it
		mPacket.data += mPacketDataOffset;
		mPacket.size -= mPacketDataOffset;
		int frameAvailable = 0;
		const int processedLength = avcodec_decode_video2(mVideoStream->codec, frame, &frameAvailable, &mPacket);
		mPacket.data -= mPacketDataOffset;
		mPacket.size += mPacketDataOffset;

		if(processedLength < 0)
		{
			LogFFmpegError(processedLength);
			break;
		}

		if(processedLength == 0)
			LOG(LogInfo) << "no frame decoded...";

		mPacketDataOffset += processedLength;

		if(frameAvailable)
		{
			// convert frame to RGB
			AVPicture pic;
			avpicture_alloc(&pic, PIX_FMT_RGB24, frame->width, frame->height);

			SwsContext* swsCtx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
				frame->width, frame->height, PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
			assert(swsCtx);

			sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, pic.data, pic.linesize);

			// update texture
			glBindTexture(GL_TEXTURE_2D, mTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_RGB, GL_UNSIGNED_BYTE, pic.data[0]);
			// for(int y = 0; y < frame->height; y++)
			//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, frame->width, 1, GL_RGB, GL_UNSIGNED_BYTE, frame->data[0] + (y * pic.linesize[0]));

			sws_freeContext(swsCtx);
			avpicture_free(&pic);
			break;
		}
	}

	av_frame_free(&frame);
}

int64_t VideoResource::msToNextFrame() const
{
	return av_rescale(mVideoStream->codec->ticks_per_frame * 1000, mVideoStream->codec->time_base.num, mVideoStream->codec->time_base.den);
}

void VideoResource::advance(unsigned int ms)
{
	mTimeAccumulator += ms;
	
	int framesRead = 0;
	while(msToNextFrame() <= mTimeAccumulator && framesRead < 8)
	{
		mTimeAccumulator -= msToNextFrame();
		readNextFrame();
		framesRead++;
	}

	if(framesRead > 1)
		LOG(LogWarning) << "Skipping " << (framesRead - 1) << " frames!";
}

GLuint VideoResource::getFrameTexture()
{
	return mTexture;
}

void VideoResource::close()
{
	if(mFormatCtx)
	{
		// this function also sets mFormatCtx to NULL
		avformat_close_input(&mFormatCtx);
	}

	if(mTexture)
		glDeleteTextures(1, &mTexture);
	mTexture = NULL;
}

VideoResource::~VideoResource()
{
	close();
}
