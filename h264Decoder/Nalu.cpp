//
// Created by young on 2023/1/31.
//
#include "Nalu.h"

int Nalu::SetBuf(uint8_t* buf, int len)
{
	this->buf = (uint8_t*)malloc(len);
	memcpy(this->buf, buf, len);
	this->len = len;
	return 0;
}
Nalu::Nalu()
{
	this->buf = nullptr;
	this->len = 0;
	this->startCodeLen = 0;
}
Nalu::~Nalu()
{
	if (this->buf != nullptr)
		free(this->buf);
}
