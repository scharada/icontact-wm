#include <imaging.h>
#include "macros.h"

HFONT BuildFont(int, BOOL, BOOL);
void DrawGradientGDI(HDC, RECT, COLORREF, COLORREF);
void BltAlpha(HDC, int, int, int, int, HDC, BYTE);
void BltAlpha(HDC, int, int, int, int, HDC, int, int, int, int, BYTE alpha);
HRESULT GetStreamSize(IStream* pStream, ULONG* pulSize);
void ScaleProportional(UINT uFitToWidth, UINT uFitToHeight, 
    UINT *puWidthToScale, UINT *puHeightToScale);
HBITMAP HBITMAPFromImage (IN IImage * pImage, IN COLORREF crBackColor);
HRESULT GetBitmapFromStream(IStream* pStream, HBITMAP* phBitmap, 
    UINT* puWidth, UINT* puHeight);
void DrawRect(HDC hdc, LPRECT prc, COLORREF clr);