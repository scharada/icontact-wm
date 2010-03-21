#include "SimpleIni.h"
#include "stdafx.h"

#pragma once

#define MAIN_SECTION     TEXT("main")
#define LANGUAGE_SECTION TEXT("language")
#define THEME_SECTION    TEXT("theme")

int parseColor(const TCHAR * color_hex, const TCHAR * default_hex);

class Settings {
private:
    CSimpleIniW ini;

    int initColor(TCHAR *, TCHAR *, TCHAR *);

public:
    Settings(void);
    ~Settings(void);

    // POOM settings
    const TCHAR * categories_field;
    const TCHAR * favorite_category;
    const TCHAR * email_account;

    // Strings
    const TCHAR * alphabet;
	const TCHAR * add_new_contact_string;
    const TCHAR * mobile_string;
    const TCHAR * home_string;
    const TCHAR * work_string;
    const TCHAR * company_string;
    const TCHAR * email_string;
    const TCHAR * sms_string;
    const TCHAR * today_string;
    const TCHAR * yesterday_string;
    const TCHAR * sunday_string;
    const TCHAR * monday_string;
    const TCHAR * tuesday_string;
    const TCHAR * wednesday_string;
    const TCHAR * thursday_string;
    const TCHAR * friday_string;
    const TCHAR * saturday_string;
    const TCHAR * older_string;
    const TCHAR * type_string;
    const TCHAR * number_string;
    const TCHAR * date_string;
    const TCHAR * duration_string;
    const TCHAR * seconds_string;
    const TCHAR * outgoing_string;
    const TCHAR * missed_string;
    const TCHAR * incoming_string;
    const TCHAR * unknown_string;

	bool doExitOnAction;
	bool doExitOnMinimize;
    bool doFastGraphics;

    const TCHAR * skin_name;

    // Theme
    COLORREF rgbTitlebarBackground;
    COLORREF rgbTitlebarText;
    COLORREF rgbTitlebarSignal;
    HBRUSH hbrListBackground;
    HBRUSH hbrListGroupBackground;
    COLORREF rgbListGroupText;
    HBRUSH hbrListItemBackground;
    COLORREF rgbListItemHoverBackground1;
    COLORREF rgbListItemHoverBackground2;
	COLORREF rgbListItemText;
    HBRUSH hbrListItemForeground;
    HPEN hpenListItemForeground;
	COLORREF rgbListItemSelectedBackground1;
	COLORREF rgbListItemSelectedBackground2;
    COLORREF rgbListItemSelectedText;
	COLORREF rgbListItemSelectedShadow;
    HBRUSH hbrListItemSeparator;
    HBRUSH hbrDetailItemSeparator;
	COLORREF rgbListIndicatorText;
	COLORREF rgbKeyboardText;
    HBRUSH hbrKeyboardBackground;
    HPEN hpenKeyboardGrid;
};
