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
	{
		free(this->buf);
		this->buf = nullptr;
	}
}

int Nalu::GetEBSP(EBSP& ebsp)
{
	ebsp.len = this->len - this->startCodeLen;
	ebsp.buf = (uint8_t*)malloc(ebsp.len);
	memcpy(ebsp.buf, this->buf + this->startCodeLen, ebsp.len);
	return 0;
}
int Nalu::ParseHeader()
{
	uint8_t header = this->rbsp.buf[0];
	this->forbidden_bit = (header >> 7) & 0x01;
	this->nal_ref_idc = (header >> 5) & 0x03;
	this->nal_unit_type = header & 0x1F;
	return 0;
}
