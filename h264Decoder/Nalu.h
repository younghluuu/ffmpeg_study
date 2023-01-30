//
// Created by young on 2023/1/30.
//

#ifndef H264_H264DECODER_NALU_HPP_
#define H264_H264DECODER_NALU_HPP_
#include <cstdint>
#include <string>
class Nalu
{
 public:
	Nalu();
	~Nalu();

	int SetBuf(uint8_t* buf, int len);
	int len;
 private:
	uint8_t* buf;
	int startCodeLen;
};

#endif //H264_H264DECODER_NALU_HPP_
