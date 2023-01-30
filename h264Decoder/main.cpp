//
// Created by young on 2023/1/30.
//

#include "Nalu.h"
#include <iostream>
#include "AnnexBReader.h"
int main()
{
	std::cout << "hello world" << std::endl;

	FILE* fp = fopen("../video/demo_video_176x144_baseline.h264", "rb");
	int bufLen = 1024;

	int count = 0;
	while (true)
	{
		uint8_t* buf = (uint8_t*)malloc(bufLen);
		size_t n = fread(buf, 1, bufLen, fp);
		if (n <= 0)
			break;
		count++;
	}
	std::cout << count << std::endl;

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
		Nalu nalu;
		ret = reader.ReadNalu(nalu);
		if (ret < 0)
			break;

		printf("num:[%d],NaluSize:[%d] Nalu Type \n", ++num, nalu.len);

	}
	reader.Close();
	std::cout << "end" << std::endl;
}