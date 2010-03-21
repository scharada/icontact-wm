#include "stdafx.h"

int wstrlen(WCHAR* s)
{
     int len = 0;
     WCHAR *c = &s[0];
     while (*c++) ++len;
     return len;
}

HFONT BuildFont(int FontSize, BOOL Bold, BOOL Italic)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	lf.lfHeight = FontSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;

	if (Bold == TRUE)
		lf.lfWeight = 600;
	else
		lf.lfWeight = 500;

	lf.lfItalic = Italic;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = EASTEUROPE_CHARSET;
	lf.lfOutPrecision = OUT_RASTER_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcsncpy (lf.lfFaceName, TEXT("Tahoma"), LF_FACESIZE);
	//lf.lfFaceName[LF_FACESIZE-1] = TEXT('\0');  // Ensure null termination
	return CreateFontIndirect(&lf);
}

void DrawGradientGDI(HDC tdc, RECT iRect, COLORREF StartRGB, COLORREF EndRGB)
{
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
	GradientFill(tdc,vert,2,&gRect,1,GRADIENT_FILL_RECT_V);
}

void DrawGradient(HDC tdc, RECT rect, COLORREF StartRGB, COLORREF EndRGB)
{
	int rStep = 0, gStep = 0, bStep = 0;
	int R, G, B;
	int dHeight = (rect.bottom - rect.top);

	rStep = (GetRValue(EndRGB) - GetRValue(StartRGB)) / dHeight;
	gStep = (GetGValue(EndRGB) - GetGValue(StartRGB)) / dHeight;
	bStep = (GetBValue(EndRGB) - GetBValue(StartRGB)) / dHeight;

	R=GetRValue(StartRGB);
    G=GetGValue(StartRGB);
    B=GetBValue(StartRGB);
	
	for (int k = 0; k < dHeight; k++)
	{
		rect.bottom = rect.top + 1;
		FillRect(tdc, &rect, CreateSolidBrush (RGB(R, G, B)));
		R+=rStep;
        G+=gStep;
        B+=bStep;
		rect.top++;
	}
}

void BltAlpha(HDC Destination, int Left, int Top, int Width, int Height, HDC Source, int Alpha)
{
	if (Alpha > 0)
	{
		BitBlt(Destination, Left, Top, Width, Height, Source, 0, 0, SRCAND);
	}
	else
	{
		BitBlt(Destination, Left, Top, Width, Height, Source, 0, 0, SRCCOPY);
	}
	/*
	BitBlt(Destination, Left, Top, Width, Height, Source, 0, 0, SRCAND);

	COLORREF c1, c2;
	for(int j = Top; j < Height - Top; j++)
	{
		for(int i = Left; i < Width - Left; i++)
		{
			
			c1 = GetPixel(Source, i, j);
			c2 = GetPixel(Destination, i, j);
			
			SetPixel(Destination, i, j, (COLORREF) RGB(55,55,55));
		}
	}
	*/
}

int SwitchTheme()
{
	return 1;
}