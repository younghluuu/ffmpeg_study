//
// Created by young on 2023/2/20.
//

#include "MediaDemuxerCore.h"
#include "ADTS.h"

MediaDemuxerCore::~MediaDemuxerCore()
{

}
int MediaDemuxerCore::Demuxing(const std::string& media_path,
	const std::string& output_video,
	const std::string& output_audio)
{
	fmt_ctx = avformat_alloc_context();
	pkt = av_packet_alloc();
	int Vcnt = 0;
	int Acnt = 0;
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

	printf("audio time:[%f]\n",
		fmt_ctx->streams[audio_stream_idx]->duration * av_q2d(fmt_ctx->streams[audio_stream_idx]->time_base));
	AVFormatContext* audio_output_fmt = avformat_alloc_context();
	audio_output_fmt->oformat = av_guess_format(nullptr, output_audio.c_str(), nullptr);
	AVStream* aac_stream = avformat_new_stream(audio_output_fmt, nullptr);
	avcodec_parameters_copy(aac_stream->codecpar, fmt_ctx->streams[audio_stream_idx]->codecpar);
	ret = avio_open(&audio_output_fmt->pb, output_audio.c_str(), AVIO_FLAG_WRITE);
	if (ret < 0)
	{
		perror("Error avio_open output audio");
		return -1;
	}
	avformat_write_header(audio_output_fmt, nullptr);

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
			printf("num:[%d], write h264 size:[%d]\n", Vcnt++, pkt->size);
			video_dec_packet(pkt);
		}
		else if (pkt->stream_index == audio_stream_idx)
		{
			printf("num:[%d], write audio size:[%d]\n", Acnt++, pkt->size);
			//audio_dec_packet(pkt);
			audio_dec_packet_stream(pkt, aac_stream, audio_output_fmt);
		}

		//减少引用计数
		av_packet_unref(pkt);
	}

	//flush
	video_dec_packet(nullptr);
	//audio_dec_packet(nullptr);	没有走codec io 不需要排水

	av_write_trailer(audio_output_fmt);
	avformat_free_context(audio_output_fmt);
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
	AVStream* stream = fmt_ctx->streams[audio_stream_idx];
	int adtsId = 0;
	if (stream->codecpar->codec_id == AV_CODEC_ID_MP3)
		adtsId = 1;

	if (strcmp(fmt_ctx->iformat->name, "mpegts") != 0)
	{
		//拼接ADTS头
		ADTS adts(stream->codecpar->profile,
			adtsId,
			stream->codecpar->sample_rate,
			stream->codecpar->channels,
			avPacket->size);
		fwrite(&adts, 1, 7, audio_out);
	}
	fwrite(avPacket->data, 1, avPacket->size, audio_out);
	return 0;
}

int MediaDemuxerCore::audio_dec_packet_stream(AVPacket* avPacket, AVStream* pStream, AVFormatContext* pContext)
{
	avPacket->stream_index = pStream->index;
	//time base
	av_packet_rescale_ts(avPacket, fmt_ctx->streams[audio_stream_idx]->time_base, pStream->time_base);
	int ret = av_write_frame(pContext, avPacket);
	if (ret < 0)
	{
		printf("write audio failed\n");
	}
	return 0;
}
void MediaDemuxerCore::init_muxing_audio()
{

}
