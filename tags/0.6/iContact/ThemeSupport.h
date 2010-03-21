#include "ContactList.h"

struct theme_struct
{
	COLORREF	ListBackgroundColor;
	COLORREF	ListItemBackgroundColor;
	COLORREF	ListItemHoverBackgroundColor;
	COLORREF	ListItemTextColor;
	COLORREF	ListSelectedEditBackgroundColor;
	COLORREF	ListSelectedBackgroundColor;
	COLORREF	ListSelectedBackgroundColor2;
	COLORREF	ListSelectedItemTextColor;
	COLORREF	ListSelectedItemTextShadowColor;
	COLORREF	ListSelectedItemDetailsButtonColor;
	COLORREF	ListGroupTextColor;
	COLORREF	ListGroupBackgroundColor;
	COLORREF	ListGroupBackgroundColor2;
	COLORREF	ListSeparatorColor;
	COLORREF	ListIndicatorBrushColor;
	COLORREF	ListIndicatorFontColor;
	COLORREF	KeyboardButtonBrushColor;
	COLORREF	KeyboardButtonFontColor;
	COLORREF	KeyboardButtonSelectedFontColor;
	COLORREF	KeyboardFontColor;
	COLORREF	KeyboardBackgroundBrushColor;
	COLORREF	KeyboardKeyBrushColor;
	COLORREF	KeyboardSelectedBrushColor;
	COLORREF	KeyboardSelectedFontColor;
	COLORREF	KeyboardGridColor;
};

void SwitchTheme(theme_struct * Theme, int Type);