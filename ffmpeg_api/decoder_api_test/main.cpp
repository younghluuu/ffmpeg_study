//
// Created by young on 2023/2/18.
//
#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#ifdef __cplusplus
};
#endif
#include <iostream>
#include "MediaDemuxerCore.h"
/*
 * 解码基本流程
 * 			avformat_open_input
 * 					↓
 * 		avformat_find_stream_info
 * 					↓
 * 			avcodec_find_decoder
 * 					↓
 * 			  avcodec_open2
 * 					↓
 * 			  av_read_frame
 * 					↓
 * 				  Packet?
 * 					↓
 * 				 AVPacket
 * 					↓
 * 			avcodec_decode_video2
 * 					↓
 * 				 AVFrame
 * 					↓
 * 			 SHOW ON SCREEN
 */


static const char* in_filepath;
static const char* outputTxt;
static const char* outputVideo;
static const char* outputYUV;
static FILE* YUV_dst_file = nullptr;
static const char* outputAudio;
static const char* outputPCM;
static FILE* PCM_dst_file = nullptr;

static AVFormatContext* fmt_ctx = nullptr;
static int video_stream_idx = -1;
static AVCodecContext* video_dec_ctx = nullptr;
static int width, height;
static AVPixelFormat pix_fmt;

static int audio_stream_idx = -1;
static AVCodecContext* audio_dec_ctx = nullptr;
static AVStream* audio_stream;

static AVFrame* frame = nullptr;
static AVPacket* pkt = nullptr;

static uint8_t* video_dst_data[4] = { nullptr };
static int video_dst_linesize[4];
static int video_dst_bufsize;

static int video_frame_count = 0;
int decode_packet(AVCodecContext* dec_ctx, AVPacket* avPacket)
{
	int ret = 0;
	ret = avcodec_send_packet(dec_ctx, avPacket);
	if (ret < 0)
	{
		char* msg = (char*)malloc(100);
		av_make_error_string(msg, 100, ret);
		sprintf(msg, "Error submitting a packet to decoding (%s)", msg);
		perror(msg);
		return ret;
	}
	while (ret >= 0)
	{
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret < 0)
		{
			//正常结束
			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
				return 0;
			char* msg = (char*)malloc(100);
			av_make_error_string(msg, 100, ret);
			std::cerr << "Error during decoding " << msg << std::endl;
			return ret;
		}

		if (dec_ctx->codec->type == AVMEDIA_TYPE_VIDEO)
		{
			if (frame->width != width || frame->height != height || frame->format != pix_fmt)
			{
				fprintf(stderr,
					"Error: old width:[%d],height:[%d],format:[%s]\n new width:[%d],height:[%d],format:[%s]\n",
					width,
					height,
					av_get_pix_fmt_name(pix_fmt),
					frame->width,
					frame->height,
					av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)));
				return -1;
			}
			printf("video_frame n:%d coded_n:%d\n", ++video_frame_count, frame->coded_picture_number);
			av_image_copy(video_dst_data,
				video_dst_linesize,
				(const uint8_t**)(frame->data),
				frame->linesize,
				pix_fmt,
				width,
				height);

			fwrite(video_dst_data[0], 1, video_dst_bufsize, YUV_dst_file);
		}
		else if (dec_ctx->codec->type == AVMEDIA_TYPE_AUDIO)
		{
			size_t unpadded_linesize =
				frame->nb_samples * av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format));
			fwrite(frame->extended_data[0], 1, unpadded_linesize, PCM_dst_file);
		}

		av_frame_unref(frame);
	}
	return ret;
}

int open_dec_ctx(int* stream_idx, AVCodecContext** dec_ctx, AVMediaType mediaType)
{
	int ret = 0;
	AVStream* stream;
	AVCodec* dec;
	AVDictionary* opts = nullptr;

	ret = av_find_best_stream(fmt_ctx, mediaType, -1, -1, nullptr, 0);
	if (ret < 0)
	{
		fprintf(stderr, "Could not find %s stream\n", av_get_media_type_string(mediaType));
		return ret;
	}
	*stream_idx = ret;
	stream = fmt_ctx->streams[*stream_idx];

	//根据解码器ID找解码器
	dec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!dec)
	{
		fprintf(stderr, "Could not find %s decoder\n", av_get_media_type_string(mediaType));
		return AVERROR(EINVAL);
	}
	//分配上下文对象  必须调用
	*dec_ctx = avcodec_alloc_context3(dec);
	if (!*dec_ctx)
	{
		fprintf(stderr, "Failed to allocate %s context\n", av_get_media_type_string(mediaType));
		return AVERROR(ENOMEM);
	}
	//复制参数
	ret = avcodec_parameters_to_context(*dec_ctx, stream->codecpar);
	if (ret < 0)
	{
		fprintf(stderr, "Failed to copy %s codecpar to decoder context\n", av_get_media_type_string(mediaType));
		return ret;
	}
	//根据编解码器初始化上下文
	ret = avcodec_open2(*dec_ctx, dec, &opts);
	if (ret < 0)
	{
		fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(mediaType));
		return ret;
	}
	return ret;
}

int main1()
{
	in_filepath = "file:../video/Titanic.ts";
	outputTxt = "output.txt";
	outputVideo = "output.h264";
	outputYUV = "output.yuv";
	outputAudio = "output.aac";
	outputPCM = "output.pcm";

	//网络功能 avformat_network_init();
	//fmt_ctx = avformat_alloc_context(); 可以不调用这个
	//打开输入流并 allocate AVFormatContext
	int ret = avformat_open_input(&fmt_ctx, in_filepath, nullptr, nullptr);
	if (ret < 0)
	{
		perror("avformat_open_input error");
		return -1;
	}
	//探测函数,检索流信息
	ret = avformat_find_stream_info(fmt_ctx, nullptr);
	if (ret < 0)
	{
		perror("avformat_find_stream_info error");
		return -1;
	}

	//视频 AVCodecContext
	open_dec_ctx(&video_stream_idx, &video_dec_ctx, AVMEDIA_TYPE_VIDEO);
	width = video_dec_ctx->width;
	height = video_dec_ctx->height;
	pix_fmt = video_dec_ctx->pix_fmt;
	//分配图像，以后用来放置解码出来的图像
	video_dst_bufsize = av_image_alloc(video_dst_data,
		video_dst_linesize,
		video_dec_ctx->width,
		video_dec_ctx->height,
		video_dec_ctx->pix_fmt,
		1);
	if (video_dst_bufsize < 0)
	{
		perror("Could not allocate raw video buffer");
		return -1;
	}
	YUV_dst_file = fopen(outputYUV, "wb");

	//音频 AVCodecContext
	open_dec_ctx(&audio_stream_idx, &audio_dec_ctx, AVMEDIA_TYPE_AUDIO);
	PCM_dst_file = fopen(outputPCM, "wb");
	audio_stream = fmt_ctx->streams[audio_stream_idx];

	//转储信息到标准输出
	av_dump_format(fmt_ctx, 0, in_filepath, 0);

	//文件信息
	FILE* txt = fopen(outputTxt, "w");
	fprintf(txt, "duration:[%lld],bit_rate:[%lld]\n", fmt_ctx->duration, fmt_ctx->bit_rate);
	fprintf(txt, "format:[%s],long format:[%s]\n", fmt_ctx->iformat->name, fmt_ctx->iformat->long_name);
	fprintf(txt, "codec format:[%s],long format:[%s]\n", video_dec_ctx->codec->name, video_dec_ctx->codec->long_name);
	fprintf(txt,
		"video width:[%d],height:[%d]\n",
		fmt_ctx->streams[video_stream_idx]->codecpar->width,
		fmt_ctx->streams[video_stream_idx]->codecpar->height);
	fprintf(txt,
		"pix format:[%s], pic width:[%d],height:[%d]\n",
		av_get_pix_fmt_name(video_dec_ctx->pix_fmt),
		video_dec_ctx->width,
		video_dec_ctx->height);
	fclose(txt);

	frame = av_frame_alloc();
	if (!frame)
	{
		perror("Could not allocate video frame");
		return -1;
	}
	pkt = av_packet_alloc();
	if (!pkt)
	{
		perror("Could not allocate packet");
		return -1;
	}
	FILE* video_fp = fopen(outputVideo, "wb");
	if (video_fp == nullptr)
	{
		perror("open outputVideo fail");
		return -1;
	}
	printf("Demuxing video from file '%s' into '%s'\n", in_filepath, outputVideo);

	int i = 0;
	while (true)
	{
		int rect = av_read_frame(fmt_ctx, pkt);
		if (rect < 0)
		{
			printf("read over\n");
			break;
		}
		if (pkt->stream_index == video_stream_idx)
		{
			std::cout << "num:" << ++i << ",写入h264,size:" << pkt->size << std::endl;
			fwrite(pkt->data, 1, pkt->size, video_fp);
			decode_packet(video_dec_ctx, pkt);
		}
		else if (pkt->stream_index == audio_stream_idx)
		{
			decode_packet(audio_dec_ctx, pkt);
		}

		av_packet_unref(pkt);
	}

	//flush
	decode_packet(video_dec_ctx, nullptr);

	av_freep(video_dst_data);
	av_frame_free(&frame);
	avcodec_free_context(&video_dec_ctx);
	avformat_close_input(&fmt_ctx);
	return 0;
}

int main()
{
	MediaDemuxerCore mediaDemuxerCore;
	mediaDemuxerCore.Demuxing("file:../video/Titanic.ts", "output.h264", "output.aac");
}