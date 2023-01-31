//
// Created by young on 2023/1/31.
//

#ifndef H264DECODE_H264DECODER_EBSP_H_
#define H264DECODE_H264DECODER_EBSP_H_
#include <cstdint>
#include <string>
#include "RBSP.h"
class EBSP
{
 public:
	EBSP();
	~EBSP();

	int GetRBSP(RBSP& rbsp);

 public:
	int len;
	uint8_t* buf;
};

#endif //H264DECODE_H264DECODER_EBSP_H_
