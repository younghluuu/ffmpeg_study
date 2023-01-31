//
// Created by young on 2023/1/30.
//

#ifndef H264_H264DECODER_NALU_HPP_
#define H264_H264DECODER_NALU_HPP_
#include <cstdint>
#include "EBSP.h"

class Nalu
{
 public:
	Nalu();
	~Nalu();

	int SetBuf(uint8_t* buf, int len);
	int len;
	int startCodeLen;

	int GetEBSP(EBSP& ebsp);

	int ParseHeader();
	RBSP rbsp;
	int forbidden_bit;
	int nal_ref_idc;
	int nal_unit_type;
 private:
	uint8_t* buf;
};

#endif //H264_H264DECODER_NALU_HPP_
