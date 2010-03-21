/*******************************************************************
This file is part of iContact.

iContact is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

iContact is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with iContact.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************/

#include "stdafx.h"
#include <imaging.h>

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