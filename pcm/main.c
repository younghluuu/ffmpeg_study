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
		left = (short*)sample;    //������
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

		//����������ȡһ��
		samplenum16 = (short*)sample;
		//pcm�洢�Ķ����޷��� short����8λΪchar�� +128Ϊ0~256
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
	//�ı��������ֵ���ô������ʾ��
	FILE* fp_stat = fopen("output_cut.txt", "wb+");

	unsigned char* sample = malloc(2);

	int cnt = 0;
	while (!feof(fp))
	{
		fread(sample, 1, 2, fp);

		if (cnt >= start_num && cnt <= (start_num + dur_num))
		{
			fwrite(sample, 1, 2, fp1);

			//���������
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
��������������������������������
 *
 * */
void simplest_pcm16le_to_wave(const char* url, int channels, int sample_rate, const char* wavepath)
{
	typedef struct WAVE_HEADER
	{
		char fccID[4];    //RIFF  Resources Interchange File Format
		unsigned long dwSize;    //�ļ���С
		char fccType[4];    //WAVE
	} WAVE_HEADER;

	typedef struct WAVE_FMT
	{
		char fccID[4];    // ��fmt ��
		unsigned long dwSize;        //������fccID��dwsSize�Ľṹ���С ��Ϊ16
		unsigned short wFormatTag;    //��ʶ�Ƿ���ѹ���ġ�����ѹ��������£�wformattag��ֵ��1��������1����������µĴ���ʽҪ��΢����վ
		unsigned short wChannels;        //��������
		unsigned long dwSamplesPerSec;        //����Ƶ�ʡ�ÿ�������������λ����
		unsigned long dwAvgBytesPerSec;        //ÿ��������ݵĿռ��С��Ҳ�����������ŵĻ���ÿ��Ҫ����������
		unsigned short wBlockAlign;        //�����λ��ֵ�����¹�ʽ�ó���wblockalign = round(wchannels * (wbitspersample % 8))
		unsigned short uiBitsPerSample;        //����λ��
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