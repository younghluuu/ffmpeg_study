//
// Created by young on 2023/2/1.
//

#ifndef H264DECODE_H264DECODER_NALUPPS_H_
#define H264DECODE_H264DECODER_NALUPPS_H_

#include "Nalu.h"
class NaluPPS : public Nalu
{
 public:
	explicit NaluPPS(Nalu& nalu);
	int Parse() override;

 public:
	int pic_parameter_set_id = 0;
	int seq_parameter_set_id = 0;
	int entropy_coding_mode_flag = 0;
	int pic_order_present_flag = 0;
	int num_slice_groups_minus1 = 0;
	int slice_group_map_type = 0;
	int run_length_minus1[8];
	int top_left[8];
	int bottom_right[8];
	int slice_group_change_direction_flag = 0;
	int slice_group_change_rate_minus1 = 0;
	int pic_size_in_map_units_minus1 = 0;
	//²»ÖªµÀ·¶Î§
	int* slice_group_id;

	int num_ref_idx_l0_active_minus1;
	int num_ref_idx_l1_active_minus1;
	int weighted_pred_flag;
	int weighted_bipred_idc;
	int pic_init_qp_minus26;
	int pic_init_qs_minus26;
	int chroma_qp_index_offset;
	int deblocking_filter_control_present_flag;
	int constrained_intra_pred_flag;
	int redundant_pic_cnt_present_flag;
};

#endif //H264DECODE_H264DECODER_NALUPPS_H_
