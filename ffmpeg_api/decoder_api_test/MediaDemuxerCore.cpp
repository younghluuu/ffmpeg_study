//
// Created by young on 2023/2/20.
//

#include "MediaDemuxerCore.h"

MediaDemuxerCore::~MediaDemuxerCore()
{

}
int MediaDemuxerCore::Demuxing(const std::string& media_path,
	const std::string& output_video,
	const std::string& output_audio)
{
	fmt_ctx = avformat_alloc_context();
	pkt = av_packet_alloc();
	int cnt = 0;
	int ret = 0;

	ret = avformat_open_input(&fmt_ctx, media_path.c_str(), nullptr, nullptr);
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
	av_dump_format(fmt_ctx, audio_stream_idx, media_path.c_str(), 0);
	h264_out = fopen(output_video.c_str(), "wb");
	if (h264_out == nullptr)
	{
		perror("open outputVideo fail");
		return -1;
	}
	audio_out = fopen(output_audio.c_str(), "wb");
	if (audio_out == nullptr)
	{
		perror("open output_audio fail");
		return -1;
	}

	while (true)
	{
		ret = av_read_frame(fmt_ctx, pkt);
		if (ret < 0)
		{
			printf("read over\n");
			break;
		}

		if (pkt->stream_index == video_stream_idx)
		{
			printf("num:[%d], write h264 size:[%d]\n", cnt++, pkt->size);
			video_dec_packet(pkt);
		}
		else if (pkt->stream_index == audio_stream_idx)
		{
			audio_dec_packet(pkt);
		}

		//减少引用计数
		av_packet_unref(pkt);
	}

	//flush
	video_dec_packet(nullptr);
	audio_dec_packet(nullptr);

	fclose(h264_out);
	av_packet_free(&pkt);
	av_bsf_free(&bsf_ctx);
	avformat_close_input(&fmt_ctx);
	return 0;
}

void MediaDemuxerCore::init_h264_mp4toannexb(AVCodecParameters* avCodecParameters)
{
	if (bsf_ctx == nullptr)
	{
		const AVBitStreamFilter* avBitStreamFilter = av_bsf_get_by_name("h264_mp4toannexb");
		//分配上下文
		int ret = av_bsf_alloc(avBitStreamFilter, &bsf_ctx);
		if (ret < 0)
		{
			perror("Error alloc bsf_ctx ");
			return;
		}
		//复制解码器属性
		ret = avcodec_parameters_copy(bsf_ctx->par_in, avCodecParameters);
		if (ret < 0)
		{
			perror("Error copy codecpar");
			return;
		}
		av_bsf_init(bsf_ctx);
	}
}

int MediaDemuxerCore::video_dec_packet(AVPacket* avPacket)
{
	int ret = 0;
	char errMsg[AV_ERROR_MAX_STRING_SIZE];
	//添加start code
	init_h264_mp4toannexb(fmt_ctx->streams[video_stream_idx]->codecpar);

	ret = av_bsf_send_packet(bsf_ctx, avPacket);
	if (ret < 0)
	{
		av_strerror(ret, errMsg, AV_ERROR_MAX_STRING_SIZE);
		fprintf(stderr, "Error submitting a packet to bsf decoding (%s)\n", errMsg);
		//return -1;
		return ret;    //annexb 可能需要更多的包
	}

	while (true)
	{
		ret = av_bsf_receive_packet(bsf_ctx, avPacket);
		if (ret < 0)
		{
			//正常结束
			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
				return 0;

			//错误
			av_strerror(ret, errMsg, AV_ERROR_MAX_STRING_SIZE);
			fprintf(stderr, "Error submitting a packet to bsf decoding (%s)\n", errMsg);
			return ret;
		}
		fwrite(avPacket->data, 1, avPacket->size, h264_out);
	}
}

int MediaDemuxerCore::audio_dec_packet(AVPacket* avPacket)
{
	int ret = 0;
	char errMsg[AV_ERROR_MAX_STRING_SIZE];

	//拼接ADTS头
	char adts_header_buf[7] = { 0 };

	return 0;
}
