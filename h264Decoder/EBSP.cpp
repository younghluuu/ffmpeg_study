//
// Created by young on 2023/1/31.
//

#include "EBSP.h"
EBSP::EBSP()
{
	this->len = 0;
	this->buf = nullptr;
}
EBSP::~EBSP()
{
	this->len = 0;
	if (this->buf != nullptr)
		free(this->buf);
	this->buf = nullptr;
}

/*
	0 0 0 => 0 0 3 0
	0 0 1 => 0 0 3 1
	0 0 2 => 0 0 3 2
	0 0 3 => 0 0 3 3
 */
int EBSP::GetRBSP(RBSP& rbsp)
{
	rbsp.len = this->len;
	rbsp.buf = (uint8_t*)malloc(rbsp.len);

	int index = 0;
	for (int i = 0; i < this->len; ++i)
	{
		//小心越界，跳过2个，最后减1个
		if (i > 2 && i < this->len - 1)
		{
			if (this->buf[i] == 0x03 && this->buf[i - 1] == 0x00 && this->buf[i - 2] == 0x00)
			{
				if (this->buf[i + 1] == 0x00 || this->buf[i + 1] == 0x01 || this->buf[i + 1] == 0x02
					|| this->buf[i + 1] == 0x03)
				{
					uint8_t* tmp1 = &this->buf[i - 1];
					uint8_t* tmp2 = &this->buf[i - 2];
					uint8_t* tmp3 = &this->buf[i];
					uint8_t* tmp4 = &this->buf[i + 1];
					//去除防竞争字符
					rbsp.len--;
					continue;
				}
			}
		}
		//不能用i，因为i的长度是this->len，
		rbsp.buf[index] = this->buf[i];
		index++;
	}
	if (this->len != rbsp.len)
	{
		printf("ebsp len:[%d] rbsp len:[%d]\n", this->len, rbsp.len);
	}
	return 0;
}
