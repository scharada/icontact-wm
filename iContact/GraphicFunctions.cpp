#include "stdafx.h"

HFONT BuildFont(int iFontSize, BOOL bBold, BOOL bItalic) {
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	lf.lfHeight = iFontSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = bBold ? 600 : 500;
	lf.lfItalic = bItalic;
	lf.lfUnderline = false;
	lf.lfStrikeOut = false;
	lf.lfCharSet = EASTEUROPE_CHARSET;
	lf.lfOutPrecision = OUT_RASTER_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcsncpy (lf.lfFaceName, TEXT("Tahoma"), LF_FACESIZE);
	//lf.lfFaceName[LF_FACESIZE-1] = L'\0';  // Ensure null termination
	return CreateFontIndirect(&lf);
}

void DrawGradientGDI(HDC tdc, RECT iRect, 
				     COLORREF StartRGB, COLORREF EndRGB) {
					 
	unsigned int Shift = 8;
	TRIVERTEX        vert[2] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = iRect.left;
	vert [0] .y      = iRect.top;
	vert [0] .Red    = GetRValue(StartRGB) << Shift;
	vert [0] .Green  = GetGValue(StartRGB) << Shift;
	vert [0] .Blue   = GetBValue(StartRGB) << Shift;
	vert [0] .Alpha  = 0x0000;
	vert [1] .x      = iRect.right;
	vert [1] .y      = iRect.bottom; 
	vert [1] .Red    = GetRValue(EndRGB) << Shift;
	vert [1] .Green  = GetGValue(EndRGB) << Shift;
	vert [1] .Blue   = GetBValue(EndRGB) << Shift;
	vert [1] .Alpha  = 0x0000;
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	GradientFill(tdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void DrawGradient (HDC tdc, RECT rect, COLORREF StartRGB, COLORREF EndRGB) {
	int rStep = 0, gStep = 0, bStep = 0;
	int R, G, B;
	int dHeight = rect.bottom - rect.top;

	rStep = (GetRValue(EndRGB) - GetRValue(StartRGB)) / dHeight;
	gStep = (GetGValue(EndRGB) - GetGValue(StartRGB)) / dHeight;
	bStep = (GetBValue(EndRGB) - GetBValue(StartRGB)) / dHeight;

	R = GetRValue(StartRGB);
    G = GetGValue(StartRGB);
    B = GetBValue(StartRGB);
	
	for (int k = 0; k < dHeight; k++) {
		rect.bottom = rect.top + 1;
		FillRect(tdc, &rect, CreateSolidBrush (RGB(R, G, B)));
		R += rStep;
        G += gStep;
        B += bStep;
		rect.top++;
	}
}

void BltAlpha(HDC hdcDest, int nXOriginDest, int nYOriginDest,
              int nWidthDest, int nHeightDest,
              HDC hdcSrc, int nXOriginSrc, int nYoriginSrc,
              int nWidthSrc, int nHeightSrc,
              BYTE alpha) {

	// For WM2003
	//DWORD dRop = Alpha > 0 ? SRCAND : SRCCOPY;
	//BitBlt(hdcDest, Left, Top, Width, Height, hdcSrc, 0, 0, dRop);
	
    // For WM5.0
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = alpha;
	bf.AlphaFormat = 0;
	AlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, 
		hdcSrc, nXOriginSrc, nYoriginSrc, nWidthSrc, nHeightSrc, bf);
}

void BltAlpha(HDC hdcDest, int nLeft, int nTop, int nWidth, int nHeight, 
			  HDC hdcSrc, BYTE alpha) {

    BltAlpha(hdcDest, nLeft, nTop, nWidth, nHeight, 
		hdcSrc, 0, 0, nWidth, nHeight, alpha);
}

