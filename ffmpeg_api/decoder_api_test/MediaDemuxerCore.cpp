//
// Created by young on 2023/2/20.
//

#include "MediaDemuxerCore.h"

int MediaDemuxerCore::DemuxingVideo(const std::string& media_path, const std::string& output_video)
{
	fmt_ctx = avformat_alloc_context();
	int ret = avformat_open_input(&fmt_ctx, media_path.c_str(), nullptr, nullptr);
	if (ret < 0)
	{
		perror("Error avformat_open_input");
		return -1;
	}
	ret = avformat_find_stream_info(fmt_ctx, nullptr);
	if (ret < 0)
	{
		perror("avformat_find_stream_info error");
		return -1;
	}
	for (int i = 0; i < fmt_ctx->nb_streams; ++i)
	{
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_stream_idx = i;
		}
		else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_stream_idx = i;
		}
	}
	if (video_stream_idx == -1)
	{
		printf("video not found\n");
		return -1;
	}

	av_dump_format(fmt_ctx, video_stream_idx, media_path.c_str(), 0);
	FILE* h264_out = fopen(output_video.c_str(), "wb");
	if (h264_out == nullptr)
	{
		perror("open outputVideo fail");
		return -1;
	}

	pkt = av_packet_alloc();
	int cnt = 0;
	while (true)
	{
		ret = av_read_frame(fmt_ctx, pkt);
		if (ret < 0)
		{
			printf("video read over\n");
			break;
		}

		if (pkt->stream_index == video_stream_idx)
		{
			printf("num:[%d], write h264 size:[%d]\n", cnt++, pkt->size);
			//添加start code
			const AVBitStreamFilter* bsfilter = av_bsf_get_by_name("h264_mp4toannexb");

		}

		//减少引用计数
		av_packet_unref(pkt);
	}

	avformat_close_input(&fmt_ctx);
	return 0;
}
void MediaDemuxerCore::init_h264_mp4toannexb(AVCodecParameters* avCodecParameters)
{
	if (bsf_ctx == nullptr)
	{
		const AVBitStreamFilter* avBitStreamFilter = av_bsf_get_by_name("h264_mp4toannexb");
	}
}

