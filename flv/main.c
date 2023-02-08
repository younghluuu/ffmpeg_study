//
// Created by young on 2023/2/3.
//
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TAG_TYPE_SCRIPT 18
#define TAG_TYPE_AUDIO  8
#define TAG_TYPE_VIDEO  9

typedef unsigned char byte;
typedef unsigned int uint;

typedef struct
{
	char Signature[3];
	char Version;
	char Flags;
	char HeaderSize[4];
} FLV_HEADER;

typedef struct
{
	char tagType;
	char DataSize[3];
	char Timestamp[3];
	char Reserved;
	char StreamId[3];
} TAG_HEADER;

int BigEndianToLE(char* p, int c)
{
	int r = 0;
	int i;
	for (i = 0; i < c; i++)
	{
		unsigned char n = *(p + i);
		int tmp = ((n << ((c - 1 - i) * 8)));
		r = r | tmp;
	}

	return r;

}

int simplest_flv_parse(char* url)
{
	int output_a = 1;
	int output_v = 1;
	FILE* ifh, * vfh, * afh;
	FILE* myout = stdout;

	ifh = fopen(url, "rb+");
	if (ifh == NULL)
		return -1;

	FLV_HEADER header;
	TAG_HEADER tagHeader;
	fread(&header, 1, sizeof(FLV_HEADER), ifh);
	fprintf(myout, "============== FLV Header ==============\n");
	fprintf(myout, "Signature:  0x %c %c %c\n", header.Signature[0], header.Signature[1], header.Signature[2]);
	fprintf(myout, "Version:    0x %X\n", header.Version);
	fprintf(myout, "Flags  :    0x %X\n", header.Flags);
	fprintf(myout, "HeaderSize: 0x %X\n", BigEndianToLE(header.HeaderSize, sizeof(header.HeaderSize)));
	fprintf(myout, "========================================\n");
	fprintf(myout, "=TYPE===dataSize==timestamp==============\n");
	int previoustagsize = 0;
	do
	{
		int tmp = _getw(ifh);
		previoustagsize = BigEndianToLE(&tmp, 4);
		fread(&tagHeader, 1, sizeof(TAG_HEADER), ifh);

		int tagHeader_dataSize = BigEndianToLE(tagHeader.DataSize, 3);
		int tagHeader_timestamp = BigEndianToLE(tagHeader.Timestamp, 3);

		char tagType_str[10];
		switch (tagHeader.tagType)
		{
		case 0x08:
			sprintf(tagType_str, "audio");
			break;
		case 0x09:
			sprintf(tagType_str, "video");
			break;
		case 0x12:
			sprintf(tagType_str, "script");
			break;
		}

		fprintf(myout, "[%6s] %6d %6d |", tagType_str, tagHeader_dataSize, tagHeader_timestamp);

		if (feof(ifh))
			break;

		char audiotag_str[100] = { 0 };
		char videotag_str[100] = { 0 };
		char firstByte = 0;
		int previousTagSize_w = 0;
		int frameType = 0;
		switch (tagHeader.tagType)
		{
		case TAG_TYPE_AUDIO:
			strcat(audiotag_str, "| ");
			firstByte = fgetc(ifh);
			int audioEncode = firstByte >> 4;
			switch (audioEncode)
			{
			case 0:
				strcat(audiotag_str, "Linear PCM, platform endian");
				break;
			case 1:
				strcat(audiotag_str, "ADPCM");
				break;
			case 2:
				strcat(audiotag_str, "MP3");
				break;
			case 3:
				strcat(audiotag_str, "Linear PCM, little endian");
				break;
			case 4:
				strcat(audiotag_str, "Nellymoser 16-kHz mono");
				break;
			case 5:
				strcat(audiotag_str, "Nellymoser 8-kHz mono");
				break;
			case 6:
				strcat(audiotag_str, "Nellymoser");
				break;
			case 7:
				strcat(audiotag_str, "G.711 A-law logarithmic PCM");
				break;
			case 8:
				strcat(audiotag_str, "G.711 mu-law logarithmic PCM");
				break;
			case 9:
				strcat(audiotag_str, "reserved");
				break;
			case 10:
				strcat(audiotag_str, "AAC");
				break;
			case 11:
				strcat(audiotag_str, "Speex");
				break;
			case 14:
				strcat(audiotag_str, "MP3 8-Khz");
				break;
			case 15:
				strcat(audiotag_str, "Device-specific sound");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}

			strcat(audiotag_str, "| ");
			int rate = (firstByte & 0x0c) >> 2;
			switch (rate)
			{
			case 0:
				strcat(audiotag_str, "5.5-kHz");
				break;
			case 1:
				strcat(audiotag_str, "1-kHz");
				break;
			case 2:
				strcat(audiotag_str, "22-kHz");
				break;
			case 3:
				strcat(audiotag_str, "44-kHz");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}

			strcat(audiotag_str, "| ");
			int bits = (firstByte & 0x02) >> 1;
			switch (bits)
			{
			case 0:
				strcat(audiotag_str, "8Bit");
				break;
			case 1:
				strcat(audiotag_str, "16Bit");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}

			strcat(audiotag_str, "| ");
			int channel = firstByte & 0x01;
			switch (channel)
			{
			case 0:
				strcat(audiotag_str, "Mono");
				break;
			case 1:
				strcat(audiotag_str, "Stereo");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}

			fprintf(myout, "%s", audiotag_str);

			//去掉读取tagBody中的的第一个字节
			if (output_a != 0)
			{
				if (afh == NULL)
					afh = fopen("output.mp3", "wb");

				for (int i = 0; i < tagHeader_dataSize - 1; i++)
					fputc(fgetc(ifh), afh);
			}
			else
			{
				fseek(ifh, tagHeader_dataSize - 1, SEEK_CUR);
			}
			break;
		case TAG_TYPE_VIDEO:
			strcat(videotag_str, "| ");
			firstByte = fgetc(ifh);
			frameType = firstByte >> 4;
			switch (frameType)
			{
			case 1:
				strcat(videotag_str, "key frame  ");
				break;
			case 2:
				strcat(videotag_str, "inter frame");
				break;
			case 3:
				strcat(videotag_str, "disposable inter frame");
				break;
			case 4:
				strcat(videotag_str, "generated keyframe");
				break;
			case 5:
				strcat(videotag_str, "video info/command frame");
				break;
			default:
				strcat(videotag_str, "UNKNOWN");
				break;
			}

			strcat(videotag_str, "| ");
			int videoEncode = firstByte & 0x0F;
			switch (videoEncode)
			{
			case 1:
				strcat(videotag_str, "JPEG (currently unused)");
				break;
			case 2:
				strcat(videotag_str, "Sorenson H.263");
				break;
			case 3:
				strcat(videotag_str, "Screen video");
				break;
			case 4:
				strcat(videotag_str, "On2 VP6");
				break;
			case 5:
				strcat(videotag_str, "On2 VP6 with alpha channel");
				break;
			case 6:
				strcat(videotag_str, "Screen video version 2");
				break;
			case 7:
				strcat(videotag_str, "AVC");
				break;
			default:
				strcat(videotag_str, "UNKNOWN");
				break;
			}
			fprintf(myout, "%s", videotag_str);

			//去掉读取tagBody中的的第一个字节
			if (output_v != 0 && vfh == NULL)
			{
				vfh = fopen("output.flv", "wb");
				fwrite((char*)&header, 1, sizeof(header), vfh);
				fwrite((char*)&previousTagSize_w, 1, sizeof(previousTagSize_w), vfh);
			}

			if (vfh != NULL)
			{
				fwrite((char*)&tagHeader, 1, sizeof(tagHeader), vfh);
				//fwrite(&firstByte, 1, 1, vfh);
				fseek(ifh, -1, SEEK_CUR);
				//for (int i = 0; i < tagHeader_dataSize - 1; i++)
				for (int i = 0; i < tagHeader_dataSize + 4; i++)
					fputc(fgetc(ifh), vfh);

				previousTagSize_w = tagHeader_dataSize + (int)sizeof(tagHeader);
			}
			else
			{
				fseek(ifh, -4, SEEK_CUR);
			}

			fseek(ifh, -4, SEEK_CUR);

			break;
		case TAG_TYPE_SCRIPT:
			//skip the data of this tag
			fseek(ifh, tagHeader_dataSize, SEEK_CUR);

			/*int type = fgetc(ifh);
			if (type != 0x02)
			{
				perror("AMF1 type not string");
				break;
			}
			char stringLen[2];
			fread(&stringLen, 1, sizeof(stringLen), ifh);
			int len = BigEndianToLE(stringLen, 2);
			fprintf(myout, " len:[%d] ", len);*/
			break;
		}

		printf("\n");
	} while (!feof(ifh));

	fclose(ifh);
	fclose(vfh);
	fclose(afh);
	return 0;
}

//reverse_bytes - turn a BigEndian byte array into a LittleEndian integer
uint reverse_bytes(byte* p, char c)
{
	int r = 0;
	int i;
	for (i = 0; i < c; i++)
		r |= (*(p + i) << (((c - 1) * 8) - 8 * i));
	return r;
}

/**
 * Analysis FLV file
 * @param url    Location of input FLV file.
 */

int simplest_flv_parser(char* url)
{

	//whether output audio/video stream
	int output_a = 1;
	int output_v = 1;
	//-------------
	FILE* ifh = NULL, * vfh = NULL, * afh = NULL;

	//FILE *myout=fopen("output_log.txt","wb+");
	FILE* myout = stdout;

	FLV_HEADER flv;
	TAG_HEADER tagheader;
	uint previoustagsize, previoustagsize_z = 0;
	uint ts = 0, ts_new = 0;

	ifh = fopen(url, "rb+");
	if (ifh == NULL)
	{
		printf("Failed to open files!");
		return -1;
	}

	//FLV file header
	fread((char*)&flv, 1, sizeof(FLV_HEADER), ifh);

	fprintf(myout, "============== FLV Header ==============\n");
	fprintf(myout, "Signature:  0x %c %c %c\n", flv.Signature[0], flv.Signature[1], flv.Signature[2]);
	fprintf(myout, "Version:    0x %X\n", flv.Version);
	fprintf(myout, "Flags  :    0x %X\n", flv.Flags);
	fprintf(myout, "HeaderSize: 0x %X\n", reverse_bytes((byte*)&flv.HeaderSize, sizeof(flv.HeaderSize)));
	fprintf(myout, "========================================\n");

	//move the file pointer to the end of the header
	fseek(ifh, reverse_bytes((byte*)&flv.HeaderSize, sizeof(flv.HeaderSize)), SEEK_SET);

	//process each tag
	do
	{

		previoustagsize = _getw(ifh);

		fread((void*)&tagheader, sizeof(TAG_HEADER), 1, ifh);

		//int temp_datasize1=reverse_bytes((byte *)&tagheader.DataSize, sizeof(tagheader.DataSize));
		int tagheader_datasize = tagheader.DataSize[0] * 65536 + tagheader.DataSize[1] * 256 + tagheader.DataSize[2];
		int tagheader_timestamp =
			tagheader.Timestamp[0] * 65536 + tagheader.Timestamp[1] * 256 + tagheader.Timestamp[2];

		char tagtype_str[10];
		switch (tagheader.tagType)
		{
		case TAG_TYPE_AUDIO:
			sprintf(tagtype_str, "AUDIO");
			break;
		case TAG_TYPE_VIDEO:
			sprintf(tagtype_str, "VIDEO");
			break;
		case TAG_TYPE_SCRIPT:
			sprintf(tagtype_str, "SCRIPT");
			break;
		default:
			sprintf(tagtype_str, "UNKNOWN");
			break;
		}
		fprintf(myout, "[%6s] %6d %6d |", tagtype_str, tagheader_datasize, tagheader_timestamp);

		//if we are not past the end of file, process the tag
		if (feof(ifh))
		{
			break;
		}

		//process tag by type
		switch (tagheader.tagType)
		{

		case TAG_TYPE_AUDIO:
		{
			char audiotag_str[100] = { 0 };
			strcat(audiotag_str, "| ");
			char tagdata_first_byte;
			tagdata_first_byte = fgetc(ifh);
			int x = tagdata_first_byte & 0xF0;
			x = x >> 4;
			switch (x)
			{
			case 0:
				strcat(audiotag_str, "Linear PCM, platform endian");
				break;
			case 1:
				strcat(audiotag_str, "ADPCM");
				break;
			case 2:
				strcat(audiotag_str, "MP3");
				break;
			case 3:
				strcat(audiotag_str, "Linear PCM, little endian");
				break;
			case 4:
				strcat(audiotag_str, "Nellymoser 16-kHz mono");
				break;
			case 5:
				strcat(audiotag_str, "Nellymoser 8-kHz mono");
				break;
			case 6:
				strcat(audiotag_str, "Nellymoser");
				break;
			case 7:
				strcat(audiotag_str, "G.711 A-law logarithmic PCM");
				break;
			case 8:
				strcat(audiotag_str, "G.711 mu-law logarithmic PCM");
				break;
			case 9:
				strcat(audiotag_str, "reserved");
				break;
			case 10:
				strcat(audiotag_str, "AAC");
				break;
			case 11:
				strcat(audiotag_str, "Speex");
				break;
			case 14:
				strcat(audiotag_str, "MP3 8-Khz");
				break;
			case 15:
				strcat(audiotag_str, "Device-specific sound");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}
			strcat(audiotag_str, "| ");
			x = tagdata_first_byte & 0x0C;
			x = x >> 2;
			switch (x)
			{
			case 0:
				strcat(audiotag_str, "5.5-kHz");
				break;
			case 1:
				strcat(audiotag_str, "1-kHz");
				break;
			case 2:
				strcat(audiotag_str, "22-kHz");
				break;
			case 3:
				strcat(audiotag_str, "44-kHz");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}
			strcat(audiotag_str, "| ");
			x = tagdata_first_byte & 0x02;
			x = x >> 1;
			switch (x)
			{
			case 0:
				strcat(audiotag_str, "8Bit");
				break;
			case 1:
				strcat(audiotag_str, "16Bit");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}
			strcat(audiotag_str, "| ");
			x = tagdata_first_byte & 0x01;
			switch (x)
			{
			case 0:
				strcat(audiotag_str, "Mono");
				break;
			case 1:
				strcat(audiotag_str, "Stereo");
				break;
			default:
				strcat(audiotag_str, "UNKNOWN");
				break;
			}
			fprintf(myout, "%s", audiotag_str);

			//if the output file hasn't been opened, open it.
			if (output_a != 0 && afh == NULL)
			{
				afh = fopen("output.mp3", "wb");
			}

			//TagData - First Byte Data
			int data_size = reverse_bytes((byte*)&tagheader.DataSize, sizeof(tagheader.DataSize)) - 1;
			if (output_a != 0)
			{
				//TagData+1
				for (int i = 0; i < data_size; i++)
					fputc(fgetc(ifh), afh);

			}
			else
			{
				for (int i = 0; i < data_size; i++)
					fgetc(ifh);
			}
			break;
		}
		case TAG_TYPE_VIDEO:
		{
			char videotag_str[100] = { 0 };
			strcat(videotag_str, "| ");
			char tagdata_first_byte;
			tagdata_first_byte = fgetc(ifh);
			int x = tagdata_first_byte & 0xF0;
			x = x >> 4;
			switch (x)
			{
			case 1:
				strcat(videotag_str, "key frame  ");
				break;
			case 2:
				strcat(videotag_str, "inter frame");
				break;
			case 3:
				strcat(videotag_str, "disposable inter frame");
				break;
			case 4:
				strcat(videotag_str, "generated keyframe");
				break;
			case 5:
				strcat(videotag_str, "video info/command frame");
				break;
			default:
				strcat(videotag_str, "UNKNOWN");
				break;
			}
			strcat(videotag_str, "| ");
			x = tagdata_first_byte & 0x0F;
			switch (x)
			{
			case 1:
				strcat(videotag_str, "JPEG (currently unused)");
				break;
			case 2:
				strcat(videotag_str, "Sorenson H.263");
				break;
			case 3:
				strcat(videotag_str, "Screen video");
				break;
			case 4:
				strcat(videotag_str, "On2 VP6");
				break;
			case 5:
				strcat(videotag_str, "On2 VP6 with alpha channel");
				break;
			case 6:
				strcat(videotag_str, "Screen video version 2");
				break;
			case 7:
				strcat(videotag_str, "AVC");
				break;
			default:
				strcat(videotag_str, "UNKNOWN");
				break;
			}
			fprintf(myout, "%s", videotag_str);

			fseek(ifh, -1, SEEK_CUR);
			//if the output file hasn't been opened, open it.
			if (vfh == NULL && output_v != 0)
			{
				//write the flv header (reuse the original file's hdr) and first previoustagsize
				vfh = fopen("output.flv", "wb");
				fwrite((char*)&flv, 1, sizeof(flv), vfh);
				fwrite((char*)&previoustagsize_z, 1, sizeof(previoustagsize_z), vfh);
			}
#if 0
			//Change Timestamp
			//Get Timestamp
			ts = reverse_bytes((byte *)&tagheader.Timestamp, sizeof(tagheader.Timestamp));
			ts=ts*2;
			//Writeback Timestamp
			ts_new = reverse_bytes((byte *)&ts, sizeof(ts));
			memcpy(&tagheader.Timestamp, ((char *)&ts_new) + 1, sizeof(tagheader.Timestamp));
#endif


			//TagData + Previous Tag Size
			int data_size = reverse_bytes((byte*)&tagheader.DataSize, sizeof(tagheader.DataSize)) + 4;
			if (output_v != 0)
			{
				//TagHeader
				fwrite((char*)&tagheader, 1, sizeof(tagheader), vfh);
				//TagData
				for (int i = 0; i < data_size; i++)
					fputc(fgetc(ifh), vfh);
			}
			else
			{
				for (int i = 0; i < data_size; i++)
					fgetc(ifh);
			}
			//rewind 4 bytes, because we need to read the previoustagsize again for the loop's sake
			fseek(ifh, -4, SEEK_CUR);

			break;
		}
		default:

			//skip the data of this tag
			fseek(ifh, reverse_bytes((byte*)&tagheader.DataSize, sizeof(tagheader.DataSize)), SEEK_CUR);
		}

		fprintf(myout, "\n");

	} while (!feof(ifh));

	_fcloseall();

	return 0;
}

int main()
{
	simplest_flv_parse("../flv/cuc_ieschool.flv");
	//simplest_flv_parser("../flv/cuc_ieschool.flv");
}