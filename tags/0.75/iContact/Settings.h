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
    const TCHAR * mobile_string;
    const TCHAR * home_string;
    const TCHAR * work_string;
    const TCHAR * car_string;
    const TCHAR * assistant_string;
    const TCHAR * fax_string;
    const TCHAR * pager_string;
    const TCHAR * radio_string;
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
    const TCHAR * date_string;
    const TCHAR * duration_string;
    const TCHAR * seconds_string;
    const TCHAR * outgoing_string;
    const TCHAR * missed_string;
    const TCHAR * incoming_string;
    const TCHAR * unknown_string;
    const TCHAR * returncall_string;
    const TCHAR * savecontact_string;
    const TCHAR * showsummary_string;
    const TCHAR * editcontact_string;
    const TCHAR * recents_string;
    const TCHAR * allcontacts_string;
    const TCHAR * details_string;

	bool doExitOnAction;
	bool doExitOnMinimize;
    bool doFastGraphics;

    const TCHAR * skin_name;

    // Theme
    COLORREF rgbTitlebarBackground;
    COLORREF rgbTitlebarText;
    COLORREF rgbTitlebarSignal;
    COLORREF rgbTitlebarBattery;
    COLORREF rgbTitlebarBatteryCharge;
    COLORREF rgbHeader;
    COLORREF rgbHeaderLoading;
    HBRUSH hbrListBackground;
    HBRUSH hbrListGroupBackground;
    COLORREF rgbListGroupText;
    HBRUSH hbrListItemBackground;
	COLORREF rgbListItemText;
    COLORREF rgbListItemMissedText;
    COLORREF rgbListItemFavoriteText;
    HBRUSH hbrListItemForeground;
    HPEN hpenListItemForeground;
	COLORREF rgbListItemSelectedBackground1;
	COLORREF rgbListItemSelectedBackground2;
    HBRUSH hbrListItemSelectedBackground;
    COLORREF rgbListItemSelectedText;
	COLORREF rgbListItemSelectedShadow;
    HBRUSH hbrListItemSeparator;
    HBRUSH hbrDetailItemSeparator;
	COLORREF rgbKeyboardText;
    HBRUSH hbrKeyboardBackground;
    HPEN hpenKeyboardGrid;
};
