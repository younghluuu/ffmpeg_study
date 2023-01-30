//
// Created by young on 2023/1/30.
//

#ifndef H264DECODE_H264DECODER_ANNEXBREADER_H_
#define H264DECODE_H264DECODER_ANNEXBREADER_H_
#include "Nalu.h"
#include <string>
#include <cstring>

class AnnexBReader
{
 public:
	explicit AnnexBReader(const std::string& filePath);
	~AnnexBReader();

	int Open();
	int Close();
	int ReadNalu(Nalu& nalu);

 private:
	bool CheckStartCode(int& startCodeLen, uint8_t* buf, int bufLen);
	int ReadFromFile();
	std::string filePath;
	FILE* f;
	bool isEnd;
	uint8_t* buffer;
	int bufferLen;
};

#endif //H264DECODE_H264DECODER_ANNEXBREADER_H_
