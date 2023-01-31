//
// Created by young on 2023/1/31.
//

#ifndef H264DECODE_H264DECODER_RBSP_H_
#define H264DECODE_H264DECODER_RBSP_H_
#include <cstdint>

class RBSP
{
 public:
	RBSP();
	~RBSP();


 public:
	int len;
	uint8_t* buf;
};

#endif //H264DECODE_H264DECODER_RBSP_H_
