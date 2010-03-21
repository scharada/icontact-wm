int wstrlen(WCHAR* s);
HFONT BuildFont(int FontSize, BOOL Bold, BOOL Italic);
void DrawGradient(HDC tdc, RECT rect, COLORREF StartRGB, COLORREF EndRGB);
void DrawGradientGDI(HDC tdc, RECT iRect, COLORREF StartRGB, COLORREF EndRGB);
void BltAlpha(HDC Destination, int Left, int Top, int Width, int Height, HDC Source, int Alpha);
int SwitchTheme();