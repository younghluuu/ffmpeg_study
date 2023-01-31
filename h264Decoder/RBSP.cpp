//
// Created by young on 2023/1/31.
//

#include <cstdlib>
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
