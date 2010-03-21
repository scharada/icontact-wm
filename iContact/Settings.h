#include "SimpleIni.h"
#include "stdafx.h"

#pragma once

int parseColor(const wchar_t * color_hex, const wchar_t * default_hex);

class Settings {
private:
    CSimpleIniW ini;
    wchar_t ini_path[MAX_PATH];

    int initColor(wchar_t *, wchar_t *, wchar_t *);

public:
    Settings(void);
    ~Settings(void);

    // Strings
    //const wchar_t * abc_button_string;
    const wchar_t * favorite_category;

    const wchar_t * email_account;
    const wchar_t * mobile_string;
    const wchar_t * home_string;
    const wchar_t * work_string;
    const wchar_t * company_string;
    const wchar_t * email_string;
    const wchar_t * sms_string;

    // Theme
    HBRUSH hbrTitlebarBackground;
    HBRUSH hbrListBackground;

    HBRUSH hbrListGroupBackground;
    int rgbListGroupText;

    HBRUSH hbrListItemBackground;
    int rgbListItemHoverBackground1;
    int rgbListItemHoverBackground2;

	int rgbListItemText;
    HBRUSH hbrListItemForeground;
    HPEN hpenListItemForeground;
	
	int rgbListItemSelectedBackground1;
	int rgbListItemSelectedBackground2;
    int rgbListItemSelectedText;
	int rgbListItemSelectedShadow;
	
    HBRUSH hbrListItemSeparator;
    HBRUSH hbrDetailItemSeparator;
	
	int rgbListIndicatorText;
};
