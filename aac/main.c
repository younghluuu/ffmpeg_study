//
// Created by young on 2023/2/2.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ADTS
{
	unsigned syncword: 12; //同步头代表着1个ADTS帧的开始，所有bit置1，即 0xFFF
	unsigned ID: 1;//MPEG标识符，0标识MPEG-4，1标识MPEG-2
	unsigned Layer: 2;// 直接置00
	unsigned protection_absent: 1;//表示是否误码校验。1 no CRC, 0 has CRC
	unsigned profile: 2;//AAC 编码级别, 0: Main Profile, 1:LC(最常用), 2: SSR, 3: reserved.
	unsigned sampling_frequency_index: 4;//采样率标识
	unsigned private_bit: 1;//直接置0，解码时忽略这个参数
	unsigned channel_configuration: 3;// 声道数标识
	unsigned original_copy: 1;// 直接置0，解码时忽略这个参数
	unsigned home: 1;// 直接置0，解码时忽略这个参数

	unsigned copyright_identification_bit: 1;//直接置0，解码时忽略这个参数
	unsigned copyright_identification_start: 1;// 直接置0，解码时忽略这个参数
	unsigned aac_frame_lenght: 13;// 当前音频帧的字节数，编码元数据字节数 + 文件头字节数(0 == protection_absent ? 7: 9)
	unsigned adts_buffer_fullness: 11;// 当设置为0x7FF时表示时可变码率
	unsigned number_of_raw_data_blocks_in_frames: 2;// 当前音频包里面包含的音频编码帧数， 置为 aac_nums - 1, 即只有一帧音频时置0

} ADTS;

int getADTSframe(unsigned char* buffer, int buf_size, unsigned char* data, int* data_size, int* skipLen)
{
	int size = 0;
	int skip = 0;
	if (!buffer || !data || !data_size)
	{
		return -1;
	}

	while (1)
	{
		if (buf_size < 7)
		{
			return -1;
		}
		//Sync words
		if ((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0))
		{
			size |= ((buffer[3] & 0x03) << 11);     //high 2 bit
			size |= buffer[4] << 3;                //middle 8 bit
			size |= ((buffer[5] & 0xe0) >> 5);        //low 3bit
			break;
		}
		++skip;
		--buf_size;
		++buffer;
	}

	if (buf_size < size)
	{
		return 1;
	}

	memcpy(data, buffer, size);
	*data_size = size;
	*skipLen = skip;
	return 0;
}

int simplest_aac_parser(char* url)
{
	int data_size = 0;
	int size = 0;
	int cnt = 0;
	int offset = 0;

	//FILE *myout=fopen("output_log.txt","wb+");
	FILE* myout = stdout;

	unsigned char* aacframe = (unsigned char*)malloc(1024);
	unsigned char* aacbuffer = (unsigned char*)malloc(1024 * 1024);

	FILE* ifile = fopen(url, "rb");
	if (!ifile)
	{
		printf("Open file error");
		return -1;
	}

	printf("-----+- ADTS Frame Table -+------+\n");
	printf(" NUM | Profile | Frequency| Size |\n");
	printf("-----+---------+----------+------+\n");

	while (!feof(ifile))
	{
		data_size = fread(aacbuffer + offset, 1, 1024 * 64, ifile);
		if (data_size < 0)
		{
			perror("<0");
		}
		unsigned char* input_data = aacbuffer;

		while (1)
		{
			int skipLen = 0;
			int ret = getADTSframe(input_data, data_size, aacframe, &size, &skipLen);
			if (ret == -1)
			{
				break;
			}
			else if (ret == 1)
			{
				memcpy(aacbuffer, input_data + skipLen, data_size - skipLen);
				offset = data_size - skipLen;
				break;
			}

			char profile_str[10] = { 0 };
			char frequence_str[10] = { 0 };

			unsigned char profile = aacframe[2] & 0xC0;
			profile = profile >> 6;
			switch (profile)
			{
			case 0:
				sprintf(profile_str, "Main");
				break;
			case 1:
				sprintf(profile_str, "LC");
				break;
			case 2:
				sprintf(profile_str, "SSR");
				break;
			default:
				sprintf(profile_str, "unknown");
				break;
			}

			unsigned char sampling_frequency_index = aacframe[2] & 0x3C;
			sampling_frequency_index = sampling_frequency_index >> 2;
			switch (sampling_frequency_index)
			{
			case 0:
				sprintf(frequence_str, "96000Hz");
				break;
			case 1:
				sprintf(frequence_str, "88200Hz");
				break;
			case 2:
				sprintf(frequence_str, "64000Hz");
				break;
			case 3:
				sprintf(frequence_str, "48000Hz");
				break;
			case 4:
				sprintf(frequence_str, "44100Hz");
				break;
			case 5:
				sprintf(frequence_str, "32000Hz");
				break;
			case 6:
				sprintf(frequence_str, "24000Hz");
				break;
			case 7:
				sprintf(frequence_str, "22050Hz");
				break;
			case 8:
				sprintf(frequence_str, "16000Hz");
				break;
			case 9:
				sprintf(frequence_str, "12000Hz");
				break;
			case 10:
				sprintf(frequence_str, "11025Hz");
				break;
			case 11:
				sprintf(frequence_str, "8000Hz");
				break;
			default:
				sprintf(frequence_str, "unknown");
				break;
			}

			fprintf(myout, "%5d| %8s|  %8s| %5d|\n", cnt, profile_str, frequence_str, size);
			data_size -= skipLen;
			data_size -= size;
			input_data += skipLen;
			input_data += size;

			cnt++;
		}

	}
	fclose(ifile);
	free(aacbuffer);
	free(aacframe);

	return 0;
}

int main()
{
	simplest_aac_parser("../aac/nocturne.aac");
}