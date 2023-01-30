//
// Created by young on 2023/1/30.
//
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

void simplest_pcm16le_split(const char* url)
{
	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("output_l.pcm", "wb+");
	FILE* fp2 = fopen("output_r.pcm", "wb+");

	unsigned char* sample = malloc(4);
	while (!feof(fp))
	{
		fread(sample, 1, 4, fp);
		fwrite(sample, 1, 2, fp1);
		fwrite(sample + 2, 1, 2, fp2);
	}

	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);

}

void simplest_pcm16le_halfvolumeleft(const char* url)
{
	FILE* pcm = fopen(url, "rb+");
	unsigned char* sample = malloc(4);

	FILE* pcml = fopen("output_halfleft.pcm", "wb+");

	while (!feof(pcm))
	{
		short* left = NULL;
		fread(sample, 1, 4, pcm);
		left = (short*)sample;    //左声道
		*left = *left / 2;
		fwrite(left, 1, 2, pcml);
		fwrite(sample + 2, 1, 2, pcml);
	}

	free(sample);
	fclose(pcml);
	fclose(pcm);

}
void simplest_pcm16le_doublespeed(const char* url)
{
	FILE* pcm = fopen(url, "rb+");
	unsigned char* sample = malloc(4);

	FILE* pcml = fopen("output_doublespeed.pcm", "wb+");

	int i = 0;
	while (!feof(pcm))
	{
		fread(sample, 1, 4, pcm);

		if (i % 2 == 0)
		{
			fwrite(sample, 1, 2, pcml);
			fwrite(sample + 2, 1, 2, pcml);
		}
		i++;
	}

	free(sample);
	fclose(pcml);
	fclose(pcm);

}
void simplest_pcm16le_to_pcm8(const char* url)
{
	FILE* fp = fopen(url, "rb+");
	unsigned char* sample = malloc(4);
	FILE* fp1 = fopen("output_8.pcm", "wb+");

	while (!feof(fp))
	{
		short* samplenum16 = NULL;
		char samplenum8 = 0;
		unsigned char samplenum8_u = 0;

		fread(sample, 1, 4, fp);

		//左右声道都取一半
		samplenum16 = (short*)sample;
		//pcm存储的都是无符号 short左移8位为char， +128为0~256
		samplenum8 = (*samplenum16) >> 8;
		samplenum8_u = samplenum8 + 128;
		fwrite(&samplenum8_u, 1, 1, fp1);

		samplenum16 = (short*)(sample + 2);
		samplenum8 = (*samplenum16) >> 8;
		samplenum8_u = samplenum8 + 128;
		fwrite(&samplenum8_u, 1, 1, fp1);
	}
	free(sample);
	fclose(fp1);
	fclose(fp);
}

void simplest_pcm16le_cut_singlechannel(const char* url, int start_num, int dur_num)
{
	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("output_cut.pcm", "wb+");
	//文本保存采样值，用大端序显示。
	FILE* fp_stat = fopen("output_cut.txt", "wb+");

	unsigned char* sample = malloc(2);

	int cnt = 0;
	while (!feof(fp))
	{
		fread(sample, 1, 2, fp);

		if (cnt >= start_num && cnt <= (start_num + dur_num))
		{
			fwrite(sample, 1, 2, fp1);

			//输出采样率
			short samplenum = sample[1];
			samplenum = samplenum << 8;
			samplenum = samplenum + sample[0];

			fprintf(fp_stat, "%6d", samplenum);
			if (cnt % 10 == 0)
			{
				fprintf(fp_stat, "\n");
			}
		}
		cnt++;
	}
	free(sample);
	fclose(fp1);
	fclose(fp);
}

/*
 *  __________________________
| riff wave chunk    |
|   groupid  = 'riff'      |
|   rifftype = 'wave'      |
|    __________________    |
|   | format chunk     |   |
|   | ckid = 'fmt '  |   |
|   |__________________|   |
|    __________________    |
|   | sound data chunk |   |
|   | ckid = 'data'  |   |
|   |__________________|   |
|__________________________|
————————————————
 *
 * */
void simplest_pcm16le_to_wave(const char* url, int channels, int sample_rate, const char* wavepath)
{
	typedef struct WAVE_HEADER
	{
		char fccID[4];    //RIFF  Resources Interchange File Format
		unsigned long dwSize;    //文件大小
		char fccType[4];    //WAVE
	} WAVE_HEADER;

	typedef struct WAVE_FMT
	{
		char fccID[4];    // “fmt ”
		unsigned long dwSize;        //不包含fccID和dwsSize的结构体大小 即为16
		unsigned short wFormatTag;    //标识是否有压缩的。在无压缩的情况下，wformattag的值是1，不等于1的其他情况下的处理方式要看微软网站
		unsigned short wChannels;        //声道数量
		unsigned long dwSamplesPerSec;        //采样频率、每秒采样次数。单位赫兹
		unsigned long dwAvgBytesPerSec;        //每秒采样数据的空间大小，也就是正常播放的话，每秒要读多少数据
		unsigned short wBlockAlign;        //块对齐位，值从以下公式得出。wblockalign = round(wchannels * (wbitspersample % 8))
		unsigned short uiBitsPerSample;        //采样位深
	} WAVE_FMT;

	typedef struct WAVE_DATA
	{
		char fccID[4];    //data
		unsigned long dwSize;
	} WAVE_DATA;

	FILE* fp = fopen(url, "rb+");
	FILE* fpout = fopen(wavepath, "wb+");

	WAVE_HEADER pcmHEADER;
	WAVE_FMT pcmFMT;
	WAVE_DATA pcmDATA;

	memcpy(pcmHEADER.fccID, "RIFF", strlen("RIFF"));
	memcpy(pcmHEADER.fccType, "WAVE", strlen("WAVE"));
	fseek(fpout, sizeof(WAVE_HEADER), SEEK_SET);

	int bits = 16;
	memcpy(pcmFMT.fccID, "fmt ", strlen("fmt "));
	pcmFMT.dwSize = 16;
	pcmFMT.wFormatTag = 1;
	pcmFMT.wChannels = channels;
	pcmFMT.dwSamplesPerSec = sample_rate;
	//pcmFMT.dwAvgBytesPerSec = sample_rate * bits;
	pcmFMT.dwAvgBytesPerSec = channels * sample_rate * bits / 8;
	pcmFMT.wBlockAlign = round(channels * (bits / 8));
	pcmFMT.uiBitsPerSample = bits;

	fwrite(&pcmFMT, 1, sizeof(pcmFMT), fpout);

	//WAVE_DATA
	memcpy(&pcmDATA, "data", strlen("data"));

	fseek(fpout, sizeof(pcmDATA), SEEK_CUR);

	unsigned short m_pcmData;
	while (!feof(fp))
	{
		fread(&m_pcmData, 1, sizeof(m_pcmData), fp);
		fwrite(&m_pcmData, 1, sizeof(m_pcmData), fpout);
		pcmDATA.dwSize += 2;
	}
	pcmHEADER.dwSize = pcmDATA.dwSize + sizeof(pcmHEADER) + sizeof(pcmFMT) + sizeof(pcmDATA);

	rewind(fpout);
	fwrite(&pcmHEADER, 1, sizeof(pcmHEADER), fpout);
	fseek(fpout, sizeof(pcmFMT), SEEK_CUR);
	fwrite(&pcmDATA, 1, sizeof(pcmDATA), fpout);

	fclose(fp);
	fclose(fpout);

}

int main()
{
	printf("hello world\n");
	simplest_pcm16le_halfvolumeleft("../audio/NocturneNo2inEflat_44.1k_s16le.pcm");
	simplest_pcm16le_doublespeed("../audio/NocturneNo2inEflat_44.1k_s16le.pcm");
	simplest_pcm16le_to_pcm8("../audio/NocturneNo2inEflat_44.1k_s16le.pcm");
	simplest_pcm16le_cut_singlechannel("../audio/drum.pcm", 2360, 120);
	simplest_pcm16le_to_wave("../audio/NocturneNo2inEflat_44.1k_s16le.pcm", 2, 44100, "output_nocturne.wav");

	simplest_pcm16le_split("../audio/NocturneNo2inEflat_44.1k_s16le.pcm");
}