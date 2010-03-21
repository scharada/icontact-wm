HFONT BuildFont(int, BOOL, BOOL);
void DrawGradient(HDC, RECT, COLORREF, COLORREF);
void DrawGradientGDI(HDC, RECT, COLORREF, COLORREF);
void BltAlpha(HDC, int, int, int, int, HDC, BYTE);
void BltAlpha(HDC, int, int, int, int, HDC, int, int, int, int, BYTE alpha);