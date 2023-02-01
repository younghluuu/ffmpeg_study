//
// Created by young on 2023/1/31.
//

#include <cstdlib>
#include <cstring>
#include "RBSP.h"
RBSP::RBSP()
{
	this->len = 0;
	this->buf = nullptr;
}
RBSP::~RBSP()
{
	this->len = 0;
	if (this->buf != nullptr)
		free(this->buf);

	this->buf = nullptr;
}
RBSP& RBSP::operator=(const RBSP& rbsp)
{
	if (this == &rbsp)
		return *this;

	if (this->buf != nullptr)
	{
		free(this->buf);
		this->buf = nullptr;
	}

	this->len = rbsp.len;
	this->buf = (uint8_t*)malloc(this->len);
	memcpy(this->buf, rbsp.buf, this->len);
	return *this;
}
