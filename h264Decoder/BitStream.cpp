//
// Created by young on 2023/2/1.
//

#include "BitStream.h"
BitStream::BitStream(uint8_t* buf, int size)
{
	this->size = size;
	this->bits_left = 8;
	this->start = (uint8_t*)malloc(size);
	memcpy(this->start, buf, size);
	this->cur = start;
}
BitStream::~BitStream()
{
	if (this->start != nullptr)
		free(this->start);

	this->start = nullptr;
	this->cur = nullptr;
	this->size = 0;
	this->bits_left = 0;
}
int BitStream::ReadU1()
{
	this->bits_left--;
	int ret = ((*(this->cur)) >> this->bits_left) & 1;
	if (bits_left == 0)
	{
		this->cur++;
		this->bits_left = 8;
	}
	return ret;
}
int BitStream::ReadU(int n)
{
	int ret = 0;
	for (int i = 0; i < n; ++i)
	{
		ret |= (ReadU1() << (n - i - 1));
	}
	return ret;
}
int BitStream::ReadUE()
{
	int ret = 0;
	int i = 0;
	while (ReadU1() == 0 && i < 32)
		i++;

	//在判断是不是0的时候，已经多读走1个1了，所以再次读i次就够了
	ret = ReadU(i);
	//前面补个已经读掉的1
	ret += (1 << i);
	ret--;
	return ret;
}
int BitStream::ReadSE()
{
	int r = ReadUE();
	if (r & 0x01)
	{
		r = (r + 1) / 2;    //为什么+1,UE读取时会-1,因此+1
	}
	else
	{
		r = -(r / 2);    //为什么这里不需要，好像是在误差范围内
	}
	return r;

}
