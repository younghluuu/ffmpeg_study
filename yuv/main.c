#include <stdio.h>
#include <malloc.h>
#include <string.h>

int simple_yuv420p_split(const char* url, int w, int h, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* y = fopen("output_420_y.y", "wb+");
	FILE* u = fopen("output_420_u.u", "wb+");
	FILE* v = fopen("output_420_v.v", "wb+");

	//±£´æyuvÊý¾Ý
	char* yuv = malloc(w * h * 3 / 2);
	for (int i = 0; i < nFrames; ++i)
	{
		memset(yuv, 0, w * h * 3 / 2);
		fread(yuv, 1, w * h * 3 / 2, pic);
		fwrite(yuv, 1, w * h, y);
		fwrite(yuv + w * h, 1, w * h / 4, u);
		fwrite(yuv + w * h * 5 / 4, 1, w * h / 4, v);
	}

	free(yuv);
	fclose(pic);
	fclose(y);
	fclose(u);
	fclose(v);
	return 0;
}

/**
 * Split Y, U, V planes in YUV420P file.
 * @param url  Location of Input YUV file.
 * @param w    Width of Input YUV file.
 * @param h    Height of Input YUV file.
 * @param num  Number of frames to process.
 *
 */
int simplest_yuv420_split(char* url, int w, int h, int num)
{
	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("output_420_y.y", "wb+");
	FILE* fp2 = fopen("output_420_u.y", "wb+");
	FILE* fp3 = fopen("output_420_v.y", "wb+");

	unsigned char* pic = (unsigned char*)malloc(w * h * 3 / 2);

	for (int i = 0; i < num; i++)
	{

		fread(pic, 1, w * h * 3 / 2, fp);
		//Y
		fwrite(pic, 1, w * h, fp1);
		//U
		fwrite(pic + w * h, 1, w * h / 4, fp2);
		//V
		fwrite(pic + w * h * 5 / 4, 1, w * h / 4, fp3);
	}

	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	return 0;
}

int simplest_yuv444_split(char* url, int w, int h, int num)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;

	FILE* y = fopen("output_444_y.y", "wb+");
	FILE* u = fopen("output_444_u.u", "wb+");
	FILE* v = fopen("output_444_v.u", "wb+");

	char* yuv = malloc(w * h * 1.5);
	for (int i = 0; i < num; ++i)
	{
		memset(yuv, 0, w * h * 1.5);
		fread(yuv, 1, w * h * 1.5, pic);
		fwrite(yuv, 1, w * h, y);
		fwrite(yuv + w * h, 1, w * h * 0.25, u);
		fwrite(yuv + w * h * 5 / 4, 1, w * h * 0.25, v);
	}

	free(yuv);
	fclose(v);
	fclose(u);
	fclose(y);
	fclose(pic);
	return 0;
}

int main()
{
	printf("Hello, World!\n");
	//simplest_yuv420_split("lena_256x256_yuv420p.yuv",256,256,1);
	int ret = simple_yuv420p_split("lena_256x256_yuv420p.yuv", 256, 256, 1);
	if (ret < 0)
		perror("simple_yuv420p_split error");

	ret = simple_yuv420p_split("lena_256x256_yuv444p.yuv", 256, 256, 1);
	if (ret < 0)
		perror("lena_256x256_yuv444p error");
	return 0;
}