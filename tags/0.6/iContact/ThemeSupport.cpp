#include "stdafx.h"
#include "ThemeSupport.h"

#define	THEME_DAY	10
#define THEME_NIGHT	20

void SwitchTheme(struct theme_struct * Theme, int Type)
{
	if (Type == THEME_DAY)
	{

		Theme->ListBackgroundColor = RGB(24,38,51);
		Theme->ListItemBackgroundColor = RGB(255,255,255);
		Theme->ListItemHoverBackgroundColor = RGB(255,255,255);
		Theme->ListItemTextColor = RGB(20,20,20);
		Theme->ListSelectedBackgroundColor = RGB(124,138,151);
		Theme->ListSelectedBackgroundColor2 = RGB(64,98,111);
		Theme->ListSelectedItemTextColor = RGB(230,230,230);
		Theme->ListSelectedItemTextShadowColor = RGB(80,80,80);
		Theme->ListSelectedItemDetailsButtonColor = RGB(200,210,210);
		Theme->ListGroupTextColor = RGB(24,38,51);
		Theme->ListGroupBackgroundColor = RGB(203,218,221);
		Theme->ListGroupBackgroundColor2 = RGB(183,198,201);
		Theme->ListSeparatorColor = RGB(220,220,220);
		Theme->ListIndicatorBrushColor = RGB(30,30,30);
		Theme->ListIndicatorFontColor = RGB(30,30,30);
		Theme->KeyboardButtonBrushColor = RGB(200,200,200);
		Theme->KeyboardButtonFontColor	= RGB(200,200,200);
		Theme->KeyboardButtonSelectedFontColor	= RGB(200,200,200);
		Theme->KeyboardFontColor = RGB(220,220,220);
		Theme->KeyboardBackgroundBrushColor = RGB(70,70,70);
		Theme->KeyboardKeyBrushColor = RGB(70,70,70);
		Theme->KeyboardSelectedBrushColor = RGB(124,138,151);
		Theme->KeyboardSelectedFontColor = RGB(20,20,20);
		Theme->KeyboardGridColor = RGB(100,100,100);

	} else if (Type == THEME_NIGHT) {

		Theme->ListBackgroundColor = RGB(124,138,151);
		Theme->ListItemBackgroundColor = RGB(30,30,30);
		Theme->ListItemHoverBackgroundColor = RGB(40,40,40);
		Theme->ListItemTextColor = RGB(220,220,220);
		Theme->ListSelectedEditBackgroundColor = RGB(20,20,20);
		Theme->ListSelectedBackgroundColor = RGB(124,138,151);
		Theme->ListSelectedBackgroundColor2 = RGB(64,98,111);
		Theme->ListSelectedItemTextColor = RGB(230,230,230);
		Theme->ListSelectedItemTextShadowColor = RGB(80,80,80);
		Theme->ListSelectedItemDetailsButtonColor = RGB(200,210,210);
		Theme->ListGroupTextColor = RGB(255,255,255);
		Theme->ListGroupBackgroundColor = RGB(0,0,0);
		Theme->ListGroupBackgroundColor2 = RGB(183,198,201);
		Theme->ListSeparatorColor = RGB(50,50,50);
		Theme->ListIndicatorBrushColor = RGB(30,30,30);
		Theme->ListIndicatorFontColor = RGB(255,255,255);
		Theme->KeyboardButtonBrushColor = RGB(200,200,200);
		Theme->KeyboardButtonFontColor	= RGB(70,70,70);
		Theme->KeyboardButtonSelectedFontColor	= RGB(200,200,200);
		Theme->KeyboardFontColor = RGB(220,220,220);
		Theme->KeyboardBackgroundBrushColor = RGB(70,70,70);
		Theme->KeyboardKeyBrushColor = RGB(70,70,70);
		Theme->KeyboardSelectedBrushColor = RGB(124,138,151);
		Theme->KeyboardSelectedFontColor = RGB(20,20,20);
		Theme->KeyboardGridColor = RGB(100,100,100);

	}
}