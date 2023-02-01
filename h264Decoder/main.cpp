//
// Created by young on 2023/1/30.
//

#include "Nalu.h"
#include <iostream>
#include "AnnexBReader.h"
#include "BitStream.h"
#include "NaluSPS.h"
#include "NaluPPS.h"
int main()
{
	std::cout << "hello world" << std::endl;

	AnnexBReader reader("../video/demo_video_176x144_baseline.h264");
	int ret = reader.Open();
	if (ret < 0)
	{
		perror("AnnexBReader open error");
		return -1;
	}

	int num = 0;
	while (true)
	{
		++num;
		Nalu nalu;
		ret = reader.ReadNalu(nalu);
		if (ret < 0)
			break;

		printf("num:[%d],NaluSize:[%d] ", ++num, nalu.len);

		nalu.ParseRBSP();
		nalu.ParseHeader();
		printf("header forbidden:[%d] ref_idc:[%d] unit_type:[%d]\n",
			nalu.forbidden_bit,
			nalu.nal_ref_idc,
			nalu.nal_unit_type);

		if (nalu.nal_unit_type == 7)
		{
			NaluSPS sps;
			sps = NaluSPS(nalu);
			sps.Parse();
			int w, h;
			sps.GetWidthHeight(&w, &h);
			printf("w:[%d] h:[%d]\n", w, h);
		}
		if (nalu.nal_unit_type == 8)
		{
			NaluPPS pps = NaluPPS(nalu);
		}
		//EBSP ebsp;
		//ret = nalu.GetEBSP(ebsp);
		//printf("ebsp len:[%d] ", ebsp.len);
		//printf("ebsp begin 4 :[%d %d %d %d] ", ebsp.buf[0], ebsp.buf[1], ebsp.buf[2], ebsp.buf[3]);

		//RBSP rbsp;
		//ebsp.GetRBSP(rbsp);
		//printf("rbsp len:[%d] ", rbsp.len);

		//nalu.rbsp = rbsp;
		//nalu.rbsp.len = rbsp.len;
		//nalu.rbsp.buf = (uint8_t*)malloc(nalu.rbsp.len);
		//memcpy(nalu.rbsp.buf, rbsp.buf, nalu.rbsp.len);
		//nalu.ParseHeader();
	}

	uint8_t temp = 20;
	BitStream bitStream(&temp, 7);
	int r = bitStream.ReadSE();
	std::cout << r << std::endl;

	reader.Close();
	std::cout << "end" << std::endl;
}