//
// Created by young on 2023/2/1.
//

#ifndef H264DECODE_H264DECODER_NALUSPS_H_
#define H264DECODE_H264DECODER_NALUSPS_H_

#include "Nalu.h"
#include "BitStream.h"
class HRD
{
 public:
	int cpb_cnt_minus1;
	int bit_rate_scale;
	int cpb_size_scale;
	//E.2.2 HRD参数语义 值应在 0 到 31 的范围内
	int bit_rate_value_minus1[32];
	int cpb_size_value_minus1[32];
	int cbr_flag[32];

	int initial_cpb_removal_delay_length_minus1;
	int cpb_removal_delay_length_minus1;
	int dpb_output_delay_length_minus1;
	int time_offset_length;
};

class SPSVUI
{
 public:
	int aspect_ratio_info_present_flag;
	int aspect_ratio_idc;
	int sar_width = 0;
	int sar_height = 0;

	int overscan_info_present_flag = 0;
	int overscan_appropriate_flag = 0;

	int video_signal_type_present_flag = 0;
	int video_format;
	int video_full_range_flag;
	int colour_description_present_flag;
	int colour_primaries;
	int transfer_characteristics;
	int matrix_coefficients;

	int chroma_loc_info_present_flag;
	int chroma_sample_loc_type_top_field;
	int chroma_sample_loc_type_bottom_field;

	int timing_info_present_flag;
	int num_units_in_tick;
	int time_scale;
	int fixed_frame_rate_flag;

	int nal_hrd_parameters_present_flag;
	HRD hrd_nal;
	int vcl_hrd_parameters_present_flag;
	HRD hrd_vcl;

	int low_delay_hrd_flag;

	int pic_struct_present_flag;
	int bitstream_restriction_flag;
	int motion_vectors_over_pic_boundaries_flag;
	int max_bytes_per_pic_denom;
	int max_bits_per_mb_denom;
	int log2_max_mv_length_horizontal;
	int log2_max_mv_length_vertical;
	int num_reorder_frames;
	int max_dec_frame_buffering;

};

class NaluSPS : public Nalu
{
 public:
	NaluSPS();
	explicit NaluSPS(Nalu& nalu);
	int Parse() override;

	int ReadScalingList(BitStream& bs, int* scalingList, int sizeOfScalingList, int* useDefaultScalingMatrixFlag);
	int ReadVuiParameters(BitStream& bs);
	int ReadHrdParameters(HRD& hrd, BitStream& bs);
	int GetWidthHeight(int* w, int* h);
 public:
	int profile_idc = 0;
	int constraint_set0_flag = 0;
	int constraint_set1_flag = 0;
	int constraint_set2_flag = 0;
	int constraint_set3_flag = 0;
	int reserved_zero_2bits = 0;
	int level_idc = 0;
	int seq_parameter_set_id = 0;

	//当chroma_format_idc不存在时，应该推断其值为1 即4：2：0
	int chroma_format_idc = 1;
	int residual_colour_transform_flag = 0;
	int bit_depth_luma_minus8 = 0;
	int bit_depth_chroma_minus8 = 0;
	int qpprime_y_zero_transform_bypass_flag = 0;
	int seq_scaling_matrix_present_flag = 0;
	//05里是8
	int seq_scaling_list_present_flag[12];
	int ScalingList4x4[6][16];
	int UseDefaultScalingMatrix4x4Flag[6];
	int ScalingList8x8[6][64];
	int UseDefaultScalingMatrix8x8Flag[6];

	int log2_max_frame_num_minus4 = 0;
	int pic_order_cnt_type = 0;

	int log2_max_pic_order_cnt_lsb_minus4 = 0;
	int delta_pic_order_always_zero_flag = 0;
	int offset_for_non_ref_pic = 0;
	int offset_for_top_to_bottom_field = 0;
	//num_ref_frames_in_pic_order_cnt_cycle 的取值范围是 0 到 255（包括 0 和 255）
	int num_ref_frames_in_pic_order_cnt_cycle = 0;
	int offset_for_ref_frame[256];

	/*
	 *  width = (pic_width_in_mbs_minus1 + 1) * 16;
		height = (pic_height_in_map_units_minus1 + 1) * 16;
	 */
	int num_ref_frames = 0;
	int gaps_in_frame_num_value_allowed_flag = 0;
	//横向宏块的个数减 1
	int pic_width_in_mbs_minus1 = 0;
	//纵向宏块的个数减 1
	int pic_height_in_map_units_minus1 = 0;
	//场编码相关信息，frame_mbs_only_flag 等于 1 的时候，表示都是帧编码，
	// frame_mbs_only_flag 等于 0 的时候，表示有可能存在场编码
	int frame_mbs_only_flag = 0;
	int mb_adaptive_frame_field_flag = 0;

	/*
	 * 上下左右的偏移,宽高不可能都是16的倍数，不足16时，补充的数据=》crop
	 * 对于 YUV 420 来说，4 个 Y 共用一组 UV，你去掉一排 Y，那么就会让剩下的数据不完整。
	 * 所以，对于 YUV 420 来说，只能减去偶数个像素。
	 * 而对于 422 来说，竖直方向上可以减去任意个像素，但是水平方向上只能减去偶数个像素
	 */
	int direct_8x8_inference_flag = 0;
	int frame_cropping_flag = 0;
	int frame_crop_left_offset = 0;
	int frame_crop_right_offset = 0;
	int frame_crop_top_offset = 0;
	int frame_crop_bottom_offset = 0;

	int vui_parameters_present_flag = 0;
	SPSVUI vui;
};

#endif //H264DECODE_H264DECODER_NALUSPS_H_
