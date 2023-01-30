//
// Created by young on 2023/1/30.
//
#include "AnnexBReader.h"

AnnexBReader::AnnexBReader(const std::string& filePath)
{
	this->filePath = filePath;
	this->f = nullptr;
	this->isEnd = false;
	this->buffer = nullptr;
	this->bufferLen = 0;
}

int AnnexBReader::Open()
{
	this->f = fopen(this->filePath.c_str(), "rb");
	if (this->f == nullptr)
		return -1;
	return 0;
}

int AnnexBReader::Close()
{
	if (this->f != nullptr)
		fclose(this->f);

	return 0;
}
/*
 * |------------|------------|------------|------------|
 * |	0001	| NALU		 |	0001	  | NALU	   |
 * |			| 			 |			  | 		   |
*/
int AnnexBReader::ReadNalu(Nalu& nalu)
{
	while (true)
	{
		if (this->bufferLen == 0)
		{
			int num = ReadFromFile();
			if (num <= 0)
			{
				return -1;
			}
		}

		int startCodeLen = 0;
		bool isStartCodeBegin = CheckStartCode(startCodeLen, this->buffer, this->bufferLen);
		if (!isStartCodeBegin)
			break;

		int endPos = -1;
		//偏移两位，避免读到开头的startCode
		for (int i = 2; i < this->bufferLen; ++i)
		{
			isStartCodeBegin = CheckStartCode(startCodeLen, this->buffer + i, this->bufferLen - i);
			if (isStartCodeBegin)
			{
				endPos = i;
				break;
			}
		}

		//找到下一个NALU的startCode,把pos位置前的数据写入nalu。一个完整的nalu构建完毕。
		if (endPos > 0)
		{
			nalu.SetBuf(this->buffer, endPos);
			uint8_t* newBuf = (uint8_t*)malloc(this->bufferLen - endPos);
			memcpy(newBuf, this->buffer + endPos, this->bufferLen - endPos);
			free(this->buffer);
			this->buffer = newBuf;
			this->bufferLen = this->bufferLen - endPos;


			return 0;
		}
		else
		{
			//没找到，需要继续读取
			int num = ReadFromFile();
			if (num <= 0)
			{
				this->isEnd = true;
				nalu.SetBuf(this->buffer, this->bufferLen);
				this->bufferLen = 0;
				return 0;
			}
		}
	}

	return -1;
}
int AnnexBReader::ReadFromFile()
{
	static int num = 0;
	int bufLen = 1024;
	uint8_t* buf = (uint8_t*)malloc(bufLen);
	size_t n = fread(buf, 1, bufLen, this->f);
	if (n > 0)
	{
		//追加到缓存中
		uint8_t* newBuf = (uint8_t*)malloc(this->bufferLen + n);
		memcpy(newBuf, this->buffer, this->bufferLen);
		memcpy(newBuf + this->bufferLen, buf, n);
		this->bufferLen += n;

		if (this->buffer != nullptr)
			free(this->buffer);

		this->buffer = newBuf;
	}

	free(buf);
	printf("%d\n", ++num);
	return n;
}

// 001/0001
bool AnnexBReader::CheckStartCode(int& startCodeLen, uint8_t* buf, int bufLen)
{
	if (bufLen < 3)
		return false;

	if (bufLen == 3)
	{
		if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
		{
			startCodeLen = 3;
			return true;
		}
		return false;
	}

	if (buf[0] == 0 && buf[1] == 0)
	{
		if (buf[2] == 1)
		{
			startCodeLen = 3;
			return true;
		}
		if (buf[2] == 0 && buf[3] == 1)
		{
			startCodeLen = 4;
			return true;
		}
	}

	return false;
}
AnnexBReader::~AnnexBReader()
{
	if (this->buffer != nullptr)
		free(this->buffer);
}
