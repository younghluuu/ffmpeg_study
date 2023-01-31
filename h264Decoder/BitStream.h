//
// Created by young on 2023/2/1.
//

#ifndef H264DECODE_H264DECODER_BITSTREAM_H_
#define H264DECODE_H264DECODER_BITSTREAM_H_
#include <string>
#include <cstdint>
class BitStream
{
 public:
	BitStream(uint8_t* buf, int size);
	~BitStream();

	int ReadU1();
	int ReadU(int n);
	int ReadUE();
	int ReadSE();
 private:
	uint8_t* start;
	int size;
	uint8_t* cur;
	int bits_left;
};

#endif //H264DECODE_H264DECODER_BITSTREAM_H_
