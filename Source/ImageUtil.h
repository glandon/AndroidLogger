/*
  AndroidLogger: Android Logger plugin for Notepad++
  Copyright (C) 2015 Simbaba at Najing <zhaoxi.du@gmail.com>

  ******************************************************
  Thanks for GedcomLexer & NppFtp plugin source code.
  ******************************************************
 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/
#ifndef _IMAGE_UTIL_H_
#define _IMAGE_UTIL_H_

#include <Windows.h>

//截断函数 
#define BOUND(x,a,b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x))) 


//几何变换的质量 
#define	IMAGE_NEAREST_NEIGHBOR_INTERPOLATE	0X00050100 
#define	IMAGE_BILINEAR_INTERPOLATE			0X00050101 
#define	IMAGE_THREE_ORDER_INTERPOLATE		0X00050102 

typedef struct raw_image{
	int version;
	int bpp;
	int size;
	int width;
	int height;
	int red_offset;
	int red_length;
	int blue_offset;
	int blue_length;
	int green_offset;
	int green_length;
	int alpha_offset;
	int alpha_length;
	BYTE *data;
}raw_image_t;

//图像颜色:RGB 
typedef struct tagPIXELCOLORRGB 
{ 
	BYTE red; 
	BYTE green; 
	BYTE blue; 
}PIXELCOLORRGB; 
 
//图像颜色:RGBA 
typedef struct tagPIXELCOLORRGBA 
{ 
	BYTE red; 
	BYTE green; 
	BYTE blue; 
	BYTE alpha; 
}PIXELCOLORRGBA; 

void  ConstructBih(int nWidth,int nHeight,BITMAPINFOHEADER& bih);
void  ContructBhh(int nWidth,int nHeight,BITMAPFILEHEADER& bhh);

HBITMAP CreateDdb(HDC hDC, int nWidth, int nHeight, const LPBYTE lpbyBits);

BYTE* RawImage16toARGB(raw_image_t *raw_in);
BYTE* RawImage32toARGB(raw_image_t *raw_in);

BOOL  SaveDIB2Bmp(BYTE *data, int iWidth, int iHeight, LPCTSTR filePath);
BYTE* Convert2RGB888(raw_image_t *raw);

float Sinxx(float fx);


class CImageUtil {
public:
	CImageUtil(DWORD quality, WORD bitCount){m_dwQuality = quality, m_bitCount = bitCount;};

public:
	BOOL Scale(LPBYTE lpbyBitsSrc, int nWidth, int nHeight, LPBYTE lpbyBitsDst, int nWidthImgDst, int nHeightImgDst);
	BOOL Scale(LPBYTE lpbyBitsSrc, int x, int y, int nWidth, int nHeight, int nScanWidth, int nScanHeight,LPBYTE lpbyBitsDst,int nWidthImgDst,int nHeightImgDst);

	PIXELCOLORRGB Interpolate(LPBYTE lpbySrcXY, int x, int y, float fu, float fv, int nScanWidth, int nScanHeight);

private:
	DWORD  m_dwQuality;
	WORD   m_bitCount;
};


#endif
