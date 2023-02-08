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

int Nalu::GetEBSP(EBSP& ebsp) const
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
int Nalu::ParseRBSP()
{
	EBSP ebsp;
	this->GetEBSP(ebsp);

	return ebsp.GetRBSP(this->rbsp);
}
int Nalu::ParseRBSP()
{
	EBSP ebsp;
	this->GetEBSP(ebsp);
	return ebsp.GetRBSP(this->rbsp);
}
int Nalu::Parse()
{
	return 0;
}
Nalu::Nalu(const Nalu& nalu)
{
	this->len = nalu.len;
	this->buf = (uint8_t*)malloc(this->len);
	memcpy(this->buf, nalu.buf, this->len);
	this->startCodeLen = nalu.startCodeLen;

	this->rbsp.len = nalu.rbsp.len;
	this->rbsp.buf = (uint8_t*)malloc(this->rbsp.len);
	memcpy(this->rbsp.buf, nalu.rbsp.buf, this->rbsp.len);

	this->forbidden_bit = nalu.forbidden_bit;
	this->nal_ref_idc = nalu.nal_ref_idc;
	this->nal_unit_type = nalu.nal_unit_type;
}
Nalu& Nalu::operator=(const Nalu& nalu)
{
	if (&nalu == this)
		return *this;

	if (this->buf != nullptr)
	{
		free(this->rbsp.buf);
		free(this->buf);
		this->buf = nullptr;
	}

	this->len = nalu.len;
	this->buf = (uint8_t*)malloc(this->len);
	memcpy(this->buf, nalu.buf, this->len);
	this->startCodeLen = nalu.startCodeLen;

	this->rbsp.len = nalu.rbsp.len;
	this->rbsp.buf = (uint8_t*)malloc(this->rbsp.len);
	memcpy(this->rbsp.buf, nalu.rbsp.buf, this->rbsp.len);

	this->forbidden_bit = nalu.forbidden_bit;
	this->nal_ref_idc = nalu.nal_ref_idc;
	this->nal_unit_type = nalu.nal_unit_type;

	return *this;
}
