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
	Nalu(const Nalu & nalu);
	~Nalu();

	Nalu& operator=(const Nalu& nalu);

	int SetBuf(uint8_t* buf, int len);

	int GetEBSP(EBSP& ebsp) const;
	int ParseRBSP();
	int ParseHeader();

	virtual int Parse();
 public:
	int len;
	uint8_t* buf;
	int startCodeLen;

	int GetEBSP(EBSP& ebsp);

	int ParseHeader();
	RBSP rbsp;
	int forbidden_bit;
	int nal_ref_idc;
	int nal_unit_type;


};

#endif //H264_H264DECODER_NALU_HPP_
