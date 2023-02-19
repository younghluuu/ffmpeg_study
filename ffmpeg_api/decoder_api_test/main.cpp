//
// Created by young on 2023/2/18.
//
#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#ifdef __cplusplus
};
#endif
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

int main()
{
	const char* outputTxt = "output.txt";
	const char* outputVideo = "output.h264";
	int video_dst_bufsize;
	uint8_t* video_dst_data[4] = { nullptr };
	int video_dst_linesize[4];
	const char* in_filepath = "../video/Titanic.ts";
	AVFormatContext* fmt_ctx = nullptr;
	int video_stream_idx = -1;
	AVCodecParameters* video_codecpar = nullptr;
	AVCodec* video_dec = nullptr;
	AVCodecContext* video_dec_ctx = nullptr;
	AVFrame* pFrame = nullptr;
	AVPacket* pkt = nullptr;
	fmt_ctx = avformat_alloc_context();
	//网络功能
	//avformat_network_init();
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
	//找到video stream index
	for (int i = 0; i < fmt_ctx->nb_streams; ++i)
	{
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_stream_idx = i;
			video_codecpar = fmt_ctx->streams[i]->codecpar;
			break;
		}
	}
	if (video_stream_idx == -1)
	{
		perror("Didn't find video stream");
		return -1;
	}

	//根据解码器ID找解码器
	video_dec = avcodec_find_decoder(video_codecpar->codec_id);
	if (!video_dec)
	{
		perror("video codec not found");
		return -1;
	}
	//分配上下文对象  必须调用
	video_dec_ctx = avcodec_alloc_context3(video_dec);
	if (!video_dec_ctx)
	{
		perror("Could not allocate video codec context");
		return -1;
	}
	ret = avcodec_parameters_to_context(video_dec_ctx, video_codecpar);
	if (ret < 0)
	{
		perror("avcodec_parameters_to_context");
		return -1;
	}
	//根据编解码器初始化上下文
	ret = avcodec_open2(video_dec_ctx, video_dec, nullptr);
	if (ret < 0)
	{
		perror("Could not open codec");
		return -1;
	}

	//文件信息
	FILE* txt = fopen(outputTxt, "w");
	fprintf(txt, "duration:[%lld],bit_rate:[%lld]\n", fmt_ctx->duration, fmt_ctx->bit_rate);
	fprintf(txt, "format:[%s],long format:[%s]\n", fmt_ctx->iformat->name, fmt_ctx->iformat->long_name);

	fprintf(txt, "codec format:[%s],long format:[%s]\n", video_dec->name, video_dec->long_name);
	fprintf(txt, "video width:[%d],height:[%d]\n", video_codecpar->width, video_codecpar->height);
	fprintf(txt,
		"pix format:[%d], pic width:[%d],height:[%d]\n",
		video_dec_ctx->pix_fmt,
		video_dec_ctx->width,
		video_dec_ctx->height);
	fclose(txt);


	//视频文件
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
	//转储信息到
	av_dump_format(fmt_ctx, 0, in_filepath, 0);

	pFrame = av_frame_alloc();
	if (!pFrame)
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
	printf("Demuxing video from file '%s' into '%s'\n", in_filepath, outputVideo);

	while (av_read_frame(fmt_ctx, pkt) > 0)
	{
		if (pkt->stream_index == video_stream_idx)
		{

		}
	}

	av_freep(video_dst_data);
	av_frame_free(&pFrame);
	avcodec_free_context(&video_dec_ctx);
	avformat_close_input(&fmt_ctx);
	return 0;
}