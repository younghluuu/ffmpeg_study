//
// Created by young on 2023/2/20.
//

#ifndef DECODER_API_TEST_FFMPEG_API_DECODER_API_TEST_MEDIADEMUXERCORE_H_
#define DECODER_API_TEST_FFMPEG_API_DECODER_API_TEST_MEDIADEMUXERCORE_H_
extern "C" {
#include "libavformat/avformat.h"
};
#include <cstdio>
#include <string>
#include <iostream>

class MediaDemuxerCore
{
 private:
	AVFormatContext* fmt_ctx = nullptr;
	AVPacket* pkt = nullptr;
	AVFrame* frame = nullptr;
	AVBSFContext* bsf_ctx = nullptr;

	int video_stream_idx = -1;
	int audio_stream_idx = -1;

	FILE* h264_out = nullptr;
 public:
	int Demuxing(const std::string& media_path, const std::string& output_video);
	~MediaDemuxerCore();
 private:
	void init_h264_mp4toannexb(AVCodecParameters* avCodecParameters);
	int dec_paket(AVPacket*);
};

#endif //DECODER_API_TEST_FFMPEG_API_DECODER_API_TEST_MEDIADEMUXERCORE_H_
