

#include "stdafx.h"
#include <io.h>  
#include <fcntl.h>
#include <iostream>
#include <Windows.h>
#include <winbase.h>
#include <atlbase.h>
#include <Commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include <xstring>
#include <string>
using std::string;
using std::wstring;
using namespace std;

#include "g711.h"

#pragma warning(disable:4996)
#pragma warning(disable:4267)
#pragma pack(1)

#include "stdafx.h"
#include "g711.h"
// ITU-g711程序源码
// 主要包括：
// 13比特线性码的U律编码解码。
// 13比特线性码的U律编码解码。
// U律和A律8位非线性吗之间的相互转换。

#define	SIGN_BIT	(0x80)		/* A-law 符号位  */
#define	QUANT_MASK	(0xf)		/* 段内量化值域  */
#define	NSEGS		(8)		    /* A-law 段落号. */
#define	SEG_SHIFT	(4)		    /* 段落左移位量  */
#define	SEG_MASK	(0x70)		/* 段落码区域.   */

//A律编码解码预制
static short seg_aend[8] = { 0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF };
//U律编码解码预制表
static short seg_uend[8] = { 0x3F, 0x7F, 0xFF, 0x1FF,0x3FF, 0x7FF, 0xFFF, 0x1FFF };

//U律到A律转化编码表
unsigned char _u2a[128] = {
	1,	1,	2,	2,	3,	3,	4,	4,
	5,	5,	6,	6,	7,	7,	8,	8,
	9,	10,	11,	12,	13,	14,	15,	16,
	17,	18,	19,	20,	21,	22,	23,	24,
	25,	27,	29,	31,	33,	34,	35,	36,
	37,	38,	39,	40,	41,	42,	43,	44,
	46,	48,	49,	50,	51,	52,	53,	54,
	55,	56,	57,	58,	59,	60,	61,	62,
	64,	65,	66,	67,	68,	69,	70,	71,
	72,	73,	74,	75,	76,	77,	78,	79,
	80,	82,	83,	84,	85,	86,	87,	88,
	89,	90,	91,	92,	93,	94,	95,	96,
	97,	98,	99,	100,101,102,103,104,
	105,106,107,108,109,110,111,112,
	113,114,115,116,117,118,119,120,
	121,122,123,124,125,126,127,128 };

//A律到U律转化编码表
unsigned char _a2u[128] = {
	1,	3,	5,	7,	9,	11,	13,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	32,	33,	33,	34,	34,	35,	35,
	36,	37,	38,	39,	40,	41,	42,	43,
	44,	45,	46,	47,	48,	48,	49,	49,
	50,	51,	52,	53,	54,	55,	56,	57,
	58,	59,	60,	61,	62,	63,	64,	64,
	65,	66,	67,	68,	69,	70,	71,	72,
	73,	74,	75,	76,	77,	78,	79,	80,
	80,	81,	82,	83,	84,	85,	86,	87,
	88,	89,	90,	91,	92,	93,	94,	95,
	96,	97,	98,	99,	100,101,102,103,
	104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,
	120,121,122,123,124,125,126,127 };

// 该子程序寻找段落码
static short search(short val, short	*table, short size)
{
	short i;
	for (i = 0; i < size; i++)
	{
		if (val <= *table++)
			return (i);
	}
	return (size);
}

/*  输入与输出码型对应表
*	线性码输入       	   非线性码输出
*	----------------      --------------
*	0000000wxyza			000wxyz
*	0000001wxyza			001wxyz
*	000001wxyzab			010wxyz
*	00001wxyzabc			011wxyz
*	0001wxyzabcd			100wxyz
*	001wxyzabcde			101wxyz
*	01wxyzabcdef			110wxyz
*	1wxyzabcdefg			111wxyz
*/
//16bit 线性码编码为8比特A律非线性码
//输入 pcm_val--16比特线性码
//输出8比特A律非线性码
/* 2's complement (16-bit range) */
unsigned char linear2alaw(short pcm_val)
{
	short		mask;
	short		seg;
	unsigned char	aval;

	pcm_val = pcm_val >> 3;

	if (pcm_val >= 0) {
		mask = 0xD5;		/* sign (7th) bit = 1 */
	}
	else {
		mask = 0x55;		/* sign bit = 0 */
		pcm_val = -pcm_val - 1;
	}
	seg = search(pcm_val, seg_aend, 8);  /* 按照输入绝对值求取段落号. */
										 // 将求取的符号位, 段落号, 段内量化值组成一个8比特数输出/
	if (seg >= 8)  return (unsigned char)(0x7F ^ mask);
	else {
		aval = (unsigned char)seg << SEG_SHIFT;
		if (seg < 2)aval |= (pcm_val >> 1) & QUANT_MASK;
		else
			aval |= (pcm_val >> seg) & QUANT_MASK;
		return (aval ^ mask);
	}
}

//8bitA律非线性码解码为16比特线性码编码
//输入8比特A律非线性码
//输出pcm_val--16比特线性码
short alaw2linear(unsigned char	a_val)
{
	short t;
	short seg;
	a_val ^= 0x55;
	t = (a_val & QUANT_MASK) << 4;
	seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg)
	{
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
		break;
	}

	return ((a_val & SIGN_BIT) ? t : -t);
}

#define	BIAS		(0x84)		//   线性码偏移值
#define CLIP         8159       //   最大量化级数量

/*
*	线性码输入       	   非线性码输出
*	----------------      --------------
*	00000001wxyza			000wxyz
*	0000001wxyzab			001wxyz
*	000001wxyzabc			010wxyz
*	00001wxyzabcd			011wxyz
*	0001wxyzabcde			100wxyz
*	001wxyzabcdef			101wxyz
*	01wxyzabcdefg			110wxyz
*	1wxyzabcdefgh			111wxyz
*/
//16比特线性码
//16bit 线性码编码为8比特u律非线性码
//输入 pcm_val--16比特线性码
//输出8比特u律非线性码
unsigned char linear2ulaw(short pcm_val)
{
	short mask;
	short seg;
	unsigned char uval;

	/* Get the sign and the magnitude of the value. */
	pcm_val = pcm_val >> 2;
	if (pcm_val < 0)
	{
		pcm_val = -pcm_val;
		mask = 0x7F;
	}
	else
	{
		mask = 0xFF;
	}

	if (pcm_val > CLIP) pcm_val = CLIP;		/* 削波 */
	pcm_val += (BIAS >> 2);

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm_val, seg_uend, 8);

	// 将求取的符号位, 段落号, 段内量化值组成一个8比特数输出/
	if (seg >= 8)
	{
		return (unsigned char)(0x7F ^ mask);
	}
	else
	{
		uval = (unsigned char)(seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
		return (uval ^ mask);
	}
}

//8bit u律非线性码解码为16bits 线性码编码
//输入u_val 8比特u律非线性码
//输出pcm_val--16比特线性码
short ulaw2linear(unsigned char	u_val)
{
	short t;
	u_val = ~u_val;
	t = ((u_val & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

	return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

//按照A律到U律转化编码表进行A律到U律码型转换
unsigned char alaw2ulaw(unsigned char aval)
{
	aval &= 0xff;
	return (unsigned char)((aval & 0x80) ? (0xFF ^ _a2u[aval ^ 0xD5]) : (0x7F ^ _a2u[aval ^ 0x55]));
}

//按照U律到A律转化编码表进行U律到A律码型转换
unsigned char ulaw2alaw(unsigned char uval)
{
	uval &= 0xff;
	return (unsigned char)((uval & 0x80) ? (0xD5 ^ (_u2a[0xFF ^ uval] - 1)) : (0x55 ^ (_u2a[0x7F ^ uval] - 1)));
}


/* 16 bit swapping */
//将16位PCM数据高低字节交换
short swap_linear(short pcm_val)
{
	struct lohibyte
	{
		unsigned char lb, hb;
	};
	union
	{
		struct lohibyte b;
		short i;
	}exchange;

	unsigned char c;
	exchange.i = pcm_val;            //16位PCM数据存入交换联合体
	c = exchange.b.hb;               //将16位PCM数据高8字节暂存
	exchange.b.hb = exchange.b.lb;   //低8字节存入高8字节
	exchange.b.lb = c;               //将高8字节存入低8字节
	return (exchange.i);             //返回交换高低8位后的PCM数据
}

typedef struct
{
	WORD  wFormatTag;       //编码方式一般为0x0001
	WORD  wChannels;        //通道数
	DWORD dwSamplesPerSec;  //采样率
	DWORD dwAvgBytesPerSec; //每秒数据字节数
	WORD  wBlockAlign;      //采样单样本字节数
	WORD  wBitsPerSample;   //样本位数
}WAVE_FORMAT;

typedef struct
{
	char  chRIFF[4];   //RIFF
	DWORD dwRIFFLen;   //FILE_SIZE-8
	char  chWAVE[4];   //WAVE
	char  chFMT[4];    //fmt 
	DWORD dwFMTLen;    //16 18
	WAVE_FORMAT wavFormat;
	char  chDATA[4];   //data
	DWORD dwDATALen;   //data_size
}WAVE_FILE_HEAD;

#pragma pack()

WAVE_FILE_HEAD m_gWaveFileHead;
//============================================================================



#define BLOCK_SIZE 1024
union
{
	unsigned char in_samples_char[BLOCK_SIZE];
	short in_samples_short[BLOCK_SIZE];
}inbuf;

union
{
	unsigned char out_samples_char[BLOCK_SIZE];
	short out_samples_short[BLOCK_SIZE];
}outbuf;

void PrintUsage(FILE *ef)
{
	fprintf(ef, "CCITT G.711 u-law, A-law and linear PCM conversions.\n");
	fprintf(ef, "          Version 94/12/30 - COST 232\n");
	fprintf(ef, "\nUsage:\n\n");
	fprintf(ef, "\tcostg711 -au|ua|la|al|lu|ul|ll infile outfile\n\n");
	fprintf(ef, "\t\t-au\tA-law to u-law conversion\n");
	fprintf(ef, "\t\t-ua\tu-law to A-law conversion\n");
	fprintf(ef, "\t\t-la\t16-bit linear PCM to A-law\n");
	fprintf(ef, "\t\t-al\tA-law to 16-bit linear PCM\n");
	fprintf(ef, "\t\t-lu\t16-bit linear PCM to u-law\n");
	fprintf(ef, "\t\t-ul\tu-law to 16-bit linear PCM\n");
	fprintf(ef, "\t\t-ll\t16-bit linear low/high byte swapping\n");
}

BOOL EncodeProcess()
{
	short in_size, out_size, count;
	short(*char_short_routine)(unsigned char uval) = NULL;
	unsigned char(*char_char_routine)(unsigned char uval) = NULL;
	short(*short_short_routine)(short pcm_val) = NULL;
	unsigned char(*short_char_routine)(short pcm_val) = NULL;
	FILE *pInFile;
	FILE *pOutFile;
	char ch_1, ch_2;
	char name1[] = "f:\\wave.pcm";
	char name2[] = "f:\\test.wave";

	/*
	//=====================================================================
	printf("CCITT G.711 u-law, A-law and linear PCM conversions.\n");
	printf("          Version 2011/09/23\n");
	printf("\nUsage:\n\n");
	printf("\tcostg711 -au|ua|la|al|lu|ul|ll infile outfile\n\n");
	printf("\t\t-au\tA-law to u-law conversion\n");
	printf("\t\t-ua\tu-law to A-law conversion\n");
	printf("\t\t-la\t16-bit linear PCM to A-law\n");
	printf("\t\t-al\tA-law to 16-bit linear PCM\n");
	printf("\t\t-lu\t16-bit linear PCM to u-law\n");
	printf("\t\t-ul\tu-law to 16-bit linear PCM\n");
	printf("\t\t-ll\t16-bit linear low/high byte swapping\n\n");


	printf(" input first&second char\n");
	scanf("%c%c", &ch_1, &ch_2);*/
	ch_1 = 'l';
	ch_2 = 'a';

	//初始化输入输出大小
	out_size = sizeof(char);
	in_size = sizeof(char);
	switch (ch_1)
	{
	case 'u':
		switch (ch_2)
		{
		case 'a':
			char_char_routine = ulaw2alaw;     //U律到A律转换--ua
			break;
		case 'l':
			out_size = sizeof(short);         //输出2byte
			char_short_routine = ulaw2linear;  //8位U律非线性解码为16位线性编码
			break;
		}
		break;
	case 'a':
		switch (ch_2)
		{
		case 'u':
			char_char_routine = alaw2ulaw;      //A律到U律转换--au
			break;
		case 'l':
			out_size = sizeof(short);          //输出2byte
			char_short_routine = alaw2linear;   //8位A律非线性解码为16位线性编码
			break;
		}
		break;
	case 'l':
		in_size = sizeof(short);               //输入2byte
		switch (ch_2)
		{
		case 'u':
			short_char_routine = linear2ulaw;   //16位线性码编码为8位U律非线性码
			break;
		case 'a':
			short_char_routine = linear2alaw;   //16位线性码编码为8位A律非线性码
			break;
		case 'l':
			out_size = sizeof(short);          //输出仍为2byte
			short_short_routine = swap_linear;  //将16位PCM数据高低字节交换
			break;
		}
		break;
	default:
		return FALSE;
	}

	//打开需要转换的文件
	if ((pInFile = fopen(name1, "rb")) == NULL)
	{
		printf("Cannot open input file %s\r\n", name1);
		return FALSE;
	}

	//打开转换结果文件
	if ((pOutFile = fopen(name2, "wb")) == NULL)
	{
		printf("Cannot open output file %s\r\n", name2);
		return FALSE;
	}

	/* Read input file and process */

	//获取文件大小
	fseek(pInFile, 0, SEEK_END);
	DWORD dwInFileTotalSize = ftell(pInFile);
	fseek(pInFile, 0, SEEK_SET);

	//文件头赋值
	DWORD dwDstFileSize = ((dwInFileTotalSize - 44) / 2) + 44;
	DWORD dwDstDataLen = (dwInFileTotalSize - 44) / 2;
	memcpy(m_gWaveFileHead.chRIFF, "RIFF", 4);
	m_gWaveFileHead.dwRIFFLen = dwDstFileSize - 8;
	memcpy(m_gWaveFileHead.chWAVE, "WAVE", 4);
	memcpy(m_gWaveFileHead.chFMT, "fmt ", 4);
	m_gWaveFileHead.dwFMTLen = 16;
	m_gWaveFileHead.wavFormat.wFormatTag = 6;
	m_gWaveFileHead.wavFormat.wChannels = 1;
	m_gWaveFileHead.wavFormat.dwSamplesPerSec = 8000;
	m_gWaveFileHead.wavFormat.dwAvgBytesPerSec = 8000;
	m_gWaveFileHead.wavFormat.wBlockAlign = 0x0001;
	m_gWaveFileHead.wavFormat.wBitsPerSample = 0x0008;

	memcpy(m_gWaveFileHead.chDATA, "data", 4);
	m_gWaveFileHead.dwDATALen = dwDstDataLen;

	fwrite(&m_gWaveFileHead, 1, 44, pOutFile);    //生成目标文件先写文件头
	fseek(pInFile, 44, SEEK_SET);                 //源文件头44byte不转换

	do
	{
		short i;
		count = fread(&inbuf, in_size, BLOCK_SIZE, pInFile);
		switch (out_size)
		{
		case 1: switch (in_size)
		{
		case 1: for (i = 0; i < count; i++)
			outbuf.out_samples_char[i] = (*char_char_routine)
			(inbuf.in_samples_char[i]);
			break;
		case 2: for (i = 0; i < count; i++)
			outbuf.out_samples_char[i] = (*short_char_routine)
			(inbuf.in_samples_short[i]);
			break;
		}
				break;
		case 2: switch (in_size)
		{
		case 1: for (i = 0; i < count; i++)
			outbuf.out_samples_short[i] = (*char_short_routine)
			(inbuf.in_samples_char[i]);
			break;
		case 2: for (i = 0; i < count; i++)
			outbuf.out_samples_short[i] = (*short_short_routine)
			(inbuf.in_samples_short[i]);
			break;
		}
				break;
		} /* end switch */

		if (fwrite(&outbuf, out_size, count, pOutFile) != count)
		{
			printf("Write Access Error, File=%s", name2);
			return FALSE;
		}
	} while (count == BLOCK_SIZE);

	fclose(pInFile);
	fclose(pOutFile);

	return TRUE;
}

#if 1
int g711encode()
{
	//while (true)
	{
		EncodeProcess();

	}



	return 0;
}
#endif

#if 0
int main()
{
	short                   in_size, out_size, count;
	short(*char_short_routine)(unsigned char uval) = NULL;
	short(*short_short_routine)(short pcm_val) = NULL;
	unsigned char(*char_char_routine)(unsigned char uval) = NULL;
	unsigned char(*short_char_routine)(short     pcm_val) = NULL;
	FILE                    *infile, *outfile;
	char  ch_1, ch_2, name1[30], name2[30];
	printf(" input first&second char\n");
	scanf("%c%c", &ch_1, &ch_2);
	printf(" input source name\n");
	scanf("%s", name1);
	printf(" input result name\n");
	scanf("%s", name2);


	out_size = sizeof(char);
	in_size = sizeof(char);
	switch (ch_1) {
	case 'u':
		switch (ch_2) {
		case 'a':
			char_char_routine = ulaw2alaw;
			break;
		case 'l':
			out_size = sizeof(short);
			char_short_routine = ulaw2linear;
			break;
		}
		break;
	case 'a':
		switch (ch_2) {
		case 'u':
			char_char_routine = alaw2ulaw;
			break;
		case 'l':
			out_size = sizeof(short);
			char_short_routine = alaw2linear;
			break;
		}
		break;
	case 'l':
		in_size = sizeof(short);
		switch (ch_2) {
		case 'u':
			short_char_routine = linear2ulaw;
			break;
		case 'a':
			short_char_routine = linear2alaw;
			break;
		case 'l':
			out_size = sizeof(short);
			short_short_routine = swap_linear;
			break;
		}
		break;
	default:
		PrintUsage(stderr); return 1; /* Exit, error code 1 */
	}

	if ((infile = fopen(name1, "rb")) == NULL)
	{
		fprintf(stderr, "Cannot open input file %s\n", name1);
		return 1; /* Exit, error code 1 */
	}

	if ((outfile = fopen(name2, "wb")) == NULL)
	{
		fprintf(stderr, "Cannot open output file %s\n", name2);
		return 1; /* Exit, error code 1 */
	}

	/* Read input file and process */
	do
	{
		short i;

		count = fread(&inbuf, in_size, BLOCK_SIZE, infile);
		switch (out_size) {

		case 1: switch (in_size) {
		case 1: for (i = 0; i < count; i++)
			outbuf.out_samples_char[i] = (*char_char_routine)
			(inbuf.in_samples_char[i]);
			break;
		case 2: for (i = 0; i < count; i++)
			outbuf.out_samples_char[i] = (*short_char_routine)
			(inbuf.in_samples_short[i]);
			break;
		}
				break;

		case 2: switch (in_size) {
		case 1: for (i = 0; i < count; i++)
			outbuf.out_samples_short[i] = (*char_short_routine)
			(inbuf.in_samples_char[i]);
			break;
		case 2: for (i = 0; i < count; i++)
			outbuf.out_samples_short[i] = (*short_short_routine)
			(inbuf.in_samples_short[i]);
			break;
		}
				break;

		} /* end switch */
		if (fwrite(&outbuf, out_size, count, outfile) != count)
		{
			fprintf(stderr, "Write Access Error, File=%s", name2); return 1;
		}
	} while (count == BLOCK_SIZE);

	fclose(infile);
	fclose(outfile);

	return 0; /* Exit, no errors */
}

#endif