#include <stdio.h>
#include <malloc.h>
#include <string.h>

//采样位数为8bit,1个像素的采样值为1Byte，所以yuv空间中y的大小为width*height， 因为是yuv420，2个y共享一个uv，所以uv分别大小为width*height*0.25
// 256x256 => u 128x128
int simple_yuv420_split(char* url, int width, int height, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* y = fopen("output_420_y.y", "wb+");
	FILE* u = fopen("output_420_u.u", "wb+");
	FILE* v = fopen("output_420_v.v", "wb+");

	unsigned char* yuv = malloc(width * height * 1.5);
	for (int i = 0; i < nFrames; ++i)
	{
		memset(yuv, 0, width * height * 1.5);

		fread(yuv, 1, width * height * 1.5, pic);
		fwrite(yuv, 1, width * height, y);
		fwrite(yuv + width * height, 1, width * height * 0.25, u);
		fwrite(yuv + width * height * 5 / 4, 1, width * height * 0.25, v);
	}

	free(yuv);
	fclose(v);
	fclose(u);
	fclose(y);
	fclose(pic);
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

int simple_yuv444p_split(char* url, int width, int height, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* y = fopen("output_444_y.y", "wb+");
	FILE* u = fopen("output_444_u.u", "wb+");
	FILE* v = fopen("output_444_v.v", "wb+");

	unsigned char* yuv = malloc(width * height * 3);
	for (int i = 0; i < nFrames; ++i)
	{
		memset(yuv, 0, width * height * 3);

		fread(yuv, 1, width * height * 3, pic);
		fwrite(yuv, 1, width * height, y);
		fwrite(yuv + width * height, 1, width * height, u);
		fwrite(yuv + width * height * 2, 1, width * height, v);
	}

	free(yuv);
	fclose(v);
	fclose(u);
	fclose(y);
	fclose(pic);
	return 0;

}

//yuv y=>luma亮度，uv=>chroma色度
//灰度处理，yCbCr在色度分量偏置前取值范围是-128~127，无色是0度，偏置后范围是0~256，无色是128.
int simple_yuv420p_gray(char* url, int width, int height, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* gray = fopen("output_420_gray.yuv", "wb+");
	unsigned char* yuv = malloc(width * height * 1.5);
	for (int i = 0; i < nFrames; ++i)
	{
		memset(yuv, 0, width * height * 1.5);
		fread(yuv, 1, width * height * 1.5, pic);
		memset(yuv + width * height, 128, width * height / 2);
		fwrite(yuv, 1, width * height * 1.5, gray);
	}

	free(yuv);
	fclose(gray);
	fclose(pic);
	return 0;
}

//亮度减半
int simple_yuv420p_halfY(char* url, int width, int height, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* halfY = fopen("output_420_halfY.yuv", "wb+");
	unsigned char* yuv = malloc(width * height * 1.5);
	for (int i = 0; i < nFrames; ++i)
	{
		memset(yuv, 0, width * height * 1.5);
		fread(yuv, 1, width * height * 1.5, pic);
		for (int j = 0; j < width * height; ++j)
		{
			yuv[j] = yuv[j] / 2;
		}
		fwrite(yuv, 1, width * height * 1.5, halfY);
	}

	free(yuv);
	fclose(halfY);
	fclose(pic);
	return 0;
}

//加边框		一维数组表示二维数组，即一行一行顺序写到数组中，高度依次增加
int simple_yuv420p_border(char* url, int width, int height, int border, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* halfY = fopen("output_420_border.yuv", "wb+");
	unsigned char* yuv = malloc(width * height * 1.5);
	for (int i = 0; i < nFrames; ++i)
	{
		memset(yuv, 0, width * height * 1.5);
		fread(yuv, 1, width * height * 1.5, pic);
		for (int w = 0; w < width; ++w)
		{
			for (int h = 0; h < height; ++h)
			{
				if (w < border || w > width - border || h < border || h > height - border)
					yuv[w * height + h] = 255;
			}
		}
/*		for (int j = 0; j < height; ++j)
		{
			for (int k = 0; k < width; ++k)
			{
				if (k < border || k > (width - border) || j < border || j > (height - border))
				{
					yuv[j * width + k] = 255;
				}
			}
		}*/
		fwrite(yuv, 1, width * height * 1.5, halfY);
	}

	free(yuv);
	fclose(halfY);
	fclose(pic);
	return 0;
}

//生成灰阶测试图
int simple_yuv420p_grayBar(char* url, int width, int height, int minY, int maxY, int barNum)
{
	FILE* grayBar = fopen(url, "wb+");
	if (grayBar == NULL)
		return -1;
	float luma_inc = (float)(maxY - minY) / (float)(barNum - 1);
	float barWidth = (float)width / (float)barNum;

	unsigned char* y = malloc(width * height);
	memset(y, 255, width * height);
	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			y[h * width + w] = minY + (unsigned char)((float)w / barWidth) * luma_inc;
		}
	}

	fwrite(y, 1, width * height, grayBar);

	char* uv = malloc(width * height * 0.5);
	memset(uv, 128, width * height * 0.5);
	fwrite(uv, 1, width * height * 0.5, grayBar);

	free(uv);
	free(y);
	fclose(grayBar);
	return 0;
}

int simplest_yuv420_graybar(int width, int height, int ymin, int ymax, int barnum, char* url_out)
{
	int barwidth;
	float lum_inc;
	unsigned char lum_temp;
	int uv_width, uv_height;
	FILE* fp = NULL;
	unsigned char* data_y = NULL;
	unsigned char* data_u = NULL;
	unsigned char* data_v = NULL;
	int t = 0, i = 0, j = 0;

	barwidth = width / barnum;
	lum_inc = ((float)(ymax - ymin)) / ((float)(barnum - 1));
	uv_width = width / 2;
	uv_height = height / 2;

	data_y = (unsigned char*)malloc(width * height);
	data_u = (unsigned char*)malloc(uv_width * uv_height);
	data_v = (unsigned char*)malloc(uv_width * uv_height);

	if ((fp = fopen(url_out, "wb+")) == NULL)
	{
		printf("Error: Cannot create file!");
		return -1;
	}

	//Output Info
	printf("Y, U, V value from picture's left to right:\n");
	for (t = 0; t < (width / barwidth); t++)
	{
		lum_temp = ymin + (char)(t * lum_inc);
		printf("%3d, 128, 128\n", lum_temp);
	}
	//Gen Data
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			t = i / barwidth;
			lum_temp = ymin + (char)(t * lum_inc);
			data_y[j * width + i] = lum_temp;
		}
	}
	for (j = 0; j < uv_height; j++)
	{
		for (i = 0; i < uv_width; i++)
		{
			data_u[j * uv_width + i] = 128;
		}
	}
	for (j = 0; j < uv_height; j++)
	{
		for (i = 0; i < uv_width; i++)
		{
			data_v[j * uv_width + i] = 128;
		}
	}
	fwrite(data_y, width * height, 1, fp);
	fwrite(data_u, uv_width * uv_height, 1, fp);
	fwrite(data_v, uv_width * uv_height, 1, fp);
	fclose(fp);
	free(data_y);
	free(data_u);
	free(data_v);
	return 0;
}

int simple_yuv422p_split(char* url, int width, int height, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* y = fopen("output_422_y.y", "wb+");
	FILE* u = fopen("output_422_u.u", "wb+");
	FILE* v = fopen("output_422_v.v", "wb+");

	unsigned char* yuv = malloc(width * height * 2);
	for (int i = 0; i < nFrames; ++i)
	{
		memset(yuv, 0, width * height * 2);

		fread(yuv, 1, width * height * 2, pic);
		fwrite(yuv, 1, width * height, y);
		fwrite(yuv + width * height, 1, width * height * 0.5, u);
		fwrite(yuv + width * height * 3 / 2, 1, width * height * 0.5, v);
	}

	free(yuv);
	fclose(v);
	fclose(u);
	fclose(y);
	fclose(pic);
	return 0;

}

//rgb采用packed交错存储，一共占用width*height*3 Byte, 提取的r,g,b依然是原来的宽高
int simple_rgb24_split(char* url, int width, int height, int nFrames)
{
	FILE* pic = fopen(url, "rb+");
	if (pic == NULL)
		return -1;
	FILE* r = fopen("output_rgb24_r.y", "wb+");
	FILE* g = fopen("output_rgb24_g.y", "wb+");
	FILE* b = fopen("output_rgb24_b.y", "wb+");

	unsigned char* rgb = malloc(width * height * 3);

	for (int i = 0; i < nFrames; ++i)
	{
		fread(rgb, 1, width * height * 3, pic);
		for (int j = 0; j < width * height * 3; j = j + 3)
		{
			fwrite(rgb + j, 1, 1, r);
			fwrite(rgb + j + 1, 1, 1, g);
			fwrite(rgb + j + 2, 1, 1, b);
		}
	}
	free(rgb);
	fclose(b);
	fclose(g);
	fclose(r);
	fclose(pic);

	return 0;
}

int simplest_rgb24_split(char* url, int w, int h, int num)
{
	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("output_r.y", "wb+");
	FILE* fp2 = fopen("output_g.y", "wb+");
	FILE* fp3 = fopen("output_b.y", "wb+");

	unsigned char* pic = (unsigned char*)malloc(w * h * 3);

	for (int i = 0; i < num; i++)
	{

		fread(pic, 1, w * h * 3, fp);

		for (int j = 0; j < w * h * 3; j = j + 3)
		{
			//R
			fwrite(pic + j, 1, 1, fp1);
			//G
			fwrite(pic + j + 1, 1, 1, fp2);
			//B
			fwrite(pic + j + 2, 1, 1, fp3);
		}
	}

	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	return 0;
}

int simple_rgb24_to_bmp(char* url, int width, int height, char* output)
{
	typedef struct
	{
		long imageSize;     //文件大小，以字节为单位
		long blank;
		long startPosition;    //位图文件头到数据的偏移量，以字节为单位
	} BmpHead;

	typedef struct
	{
		long Length;     //该结构大小，字节为单位
		long width;    //图形宽度以象素为单位
		long height;    //图形高度以象素为单位
		unsigned short colorPlane;    //目标设备的级别，必须为1
		unsigned short bitColor;    //颜色深度，每个象素所需要的位数
		long zipFormat;    //位图的压缩类型
		long realSize;    //位图的大小，以字节为单位
		long xPels;    //位图水平分辨率，每米像素数
		long yPels;    //位图垂直分辨率，每米像素数
		long colorUse;    //位图实际使用的颜色表中的颜色数
		long colorImportant;    //位图显示过程中重要的颜色数
	} InfoHead;

	char bfType[2] = { 'B', 'M' };
	int header_size = sizeof(BmpHead) + sizeof(InfoHead) + sizeof(bfType);

	BmpHead m_BMPHeader = { 0 };
	m_BMPHeader.imageSize = width * height * 3 + header_size;
	m_BMPHeader.startPosition = header_size;

	InfoHead m_BMPInfoHeader = { 0 };
	m_BMPInfoHeader.Length = sizeof(InfoHead);
	m_BMPInfoHeader.width = width;
	m_BMPInfoHeader.height = -height;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = 24;
	m_BMPInfoHeader.realSize = width * height * 3;

	FILE* fp_bmp = fopen(output, "wb+");
	fwrite(bfType, 1, sizeof(bfType), fp_bmp);
	fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
	fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);

	FILE* fp = fopen(url, "rb+");
	unsigned char* pic = malloc(width * height * 3);
	fread(pic, 1, width * height * 3, fp);

	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian
	//So we change 'R' and 'B'
	for (int h = 0; h < height; ++h)
	{
		for (int w = 0; w < width; ++w)
		{
			unsigned char tmp = pic[(h * width + w) * 3];
			pic[(h * width + w) * 3] = pic[(h * width + w) * 3 + 2];
			pic[(h * width + w) * 3 + 2] = tmp;
		}
	}

	fwrite(pic, 1, width * height * 3, fp_bmp);
	free(pic);
	fclose(fp_bmp);
	fclose(fp);
	return 0;
}

unsigned char clip_value(unsigned char x, unsigned char min_val, unsigned char max_val)
{
	if (x > max_val)
	{
		return max_val;
	}
	else if (x < min_val)
	{
		return min_val;
	}
	else
	{
		return x;
	}
}

//RGB排列为1：1：1，实际一行大小为width*3
//yuv420，u、v各占0.25,采用每行采样一般，每列采样一半即为1/4.
void RGB24_TO_YUV420(unsigned char* rgbBuf, int width, int height, unsigned char* yuvBuf)
{
	unsigned char* pTmp = rgbBuf;
	unsigned char* pY = yuvBuf;
	unsigned char* pU = yuvBuf + width * height;
	unsigned char* pV = yuvBuf + width * height * 5 / 4;
	unsigned char r, g, b, y, u, v;

	for (int h = 0; h < height; ++h)
	{
		//rgbBuf = rgbBuf + h * width * 3;    //一行实际有width*3个字节。每次循环一行
		for (int w = 0; w < width; ++w)
		{
			r = *(pTmp++);
			g = *(pTmp++);
			b = *(pTmp++);

			//tv range是为了解决滤波（模数转换）后的过冲现象
			y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
			y = clip_value(y, 0, 255);
			*(pY++) = y;

			//每行取一半，每列取一半
			if (w % 2 == 0 && h % 2 == 0)
			{
				u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
				u = clip_value(u, 0, 255);
				*(pU++) = u;
			}
			else if (w % 2 == 1 && h % 2 == 1)
			{
				v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
				v = clip_value(v, 0, 255);
				*(pV++) = v;
			}
		}
	}
}

//rgb24t to yuv420p
int simple_rgb24_to_yuv420(char* url, int width, int height, int num, char* output)
{
	FILE* rgb = fopen(url, "rb+");
	if (rgb == NULL)
		return -1;
	unsigned char* rgbBuf = malloc(width * height * 3);
	FILE* yuv = fopen(output, "wb+");
	unsigned char* yuvBuf = malloc(width * height * 1.5);

	for (int i = 0; i < num; ++i)
	{
		fread(rgbBuf, 1, width * height * 3, rgb);
		RGB24_TO_YUV420(rgbBuf, width, height, yuvBuf);
		fwrite(yuvBuf, 1, width * height * 1.5, yuv);
	}

	free(yuvBuf);
	free(rgbBuf);
	fclose(yuv);
	fclose(rgb);
	return 0;
}

void simple_rgb24_colorbar(int width, int height, const char* output)
{
	unsigned char* data = malloc(width * height * 3);
	int barWidth = width / 8;
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			switch (j / barWidth)
			{
			case 0:
				data[(i * width + j) * 3 + 0] = 255;
				data[(i * width + j) * 3 + 1] = 255;
				data[(i * width + j) * 3 + 2] = 255;
				break;
			case 1:
				data[(i * width + j) * 3 + 0] = 255;
				data[(i * width + j) * 3 + 1] = 255;
				data[(i * width + j) * 3 + 2] = 0;
				break;
			case 2:
				data[(i * width + j) * 3 + 0] = 0;
				data[(i * width + j) * 3 + 1] = 255;
				data[(i * width + j) * 3 + 2] = 255;
				break;
			case 3:
				data[(i * width + j) * 3 + 0] = 0;
				data[(i * width + j) * 3 + 1] = 255;
				data[(i * width + j) * 3 + 2] = 0;
				break;
			case 4:
				data[(i * width + j) * 3 + 0] = 255;
				data[(i * width + j) * 3 + 1] = 0;
				data[(i * width + j) * 3 + 2] = 255;
				break;
			case 5:
				data[(i * width + j) * 3 + 0] = 255;
				data[(i * width + j) * 3 + 1] = 0;
				data[(i * width + j) * 3 + 2] = 0;
				break;
			case 6:
				data[(i * width + j) * 3 + 0] = 0;
				data[(i * width + j) * 3 + 1] = 0;
				data[(i * width + j) * 3 + 2] = 255;
				break;
			case 7:
				data[(i * width + j) * 3 + 0] = 0;
				data[(i * width + j) * 3 + 1] = 0;
				data[(i * width + j) * 3 + 2] = 0;
			}

		}
	}

	FILE* pic = fopen(output, "wb+");
	fwrite(data, 1, width * height * 3, pic);

	free(data);
	fclose(pic);
}

int main()
{
	printf("Hello, World!\n");

	int ret = simple_yuv420_split("../picture/lena_256x256_yuv420p.yuv", 256, 256, 1);
	if (ret < 0)
		perror("simple_yuv420_split error");

	ret = simple_yuv444p_split("../picture/lena_256x256_yuv444p.yuv", 256, 256, 1);
	if (ret < 0)
		perror("simple_yuv444p_split error");

	ret = simple_yuv420p_gray("../picture/lena_256x256_yuv420p.yuv", 256, 256, 1);
	if (ret < 0)
		perror("simple_yuv420p_gray error");

	ret = simple_yuv420p_halfY("../picture/lena_256x256_yuv420p.yuv", 256, 256, 1);
	if (ret < 0)
		perror("simple_yuv420p_halfY error");

	ret = simple_yuv420p_border("../picture/lena_256x256_yuv420p.yuv", 256, 256, 20, 1);
	if (ret < 0)
		perror("simple_yuv420p_border error");

	ret = simple_yuv420p_grayBar("output_420_grayBar.yuv", 256, 256, 0, 255, 10);
	if (ret < 0)
		perror("simple_yuv420p_border error");

	simplest_yuv420_graybar(256, 256, 0, 255, 10, "graybar_640x360.yuv");

	ret = simple_yuv422p_split("../picture/lena_256x256_yuv422p.yuv", 256, 256, 1);
	if (ret < 0)
		perror("simple_yuv422p_split error");

	ret = simple_rgb24_split("../picture/cie1931_500x500.rgb", 500, 500, 1);
	if (ret < 0)
		perror("simple_rgb24_split error");

	simplest_rgb24_split("../picture/cie1931_500x500.rgb", 500, 500, 1);

	simple_rgb24_to_bmp("../picture/lena_256x256_rgb24.rgb", 256, 256, "output_lena.bmp");

	simple_rgb24_to_yuv420("../picture/lena_256x256_rgb24.rgb", 256, 256, 1, "output_lena.yuv");

	simple_rgb24_colorbar(256, 256, "colorbar_256x256.rgb");
	return 0;
}
