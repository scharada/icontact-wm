#include <stdafx.h>
#include <string.h>
#include "GraphicFunctions.h"

#include "Settings.h"

Settings::Settings(void) {
    this->ini = CSimpleIniW(false, false, false);

    wchar_t szIniPath[MAX_PATH];

    // Get program file path
	GetModuleFileName(NULL, szIniPath, MAX_PATH);


    //////////////////////////////////////////////////
    // Settings
    wchar_t * pstr = wcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';

	// Append settings.ini
	wcscat(szIniPath, L"settings.ini");

	this->ini.LoadFile(szIniPath);

    this->email_account = ini.GetValue(
        L"main", L"EmailAccount", L"default");

    this->favorite_category = ini.GetValue(
        L"main", L"FavoriteCategory", L"Favorites");


    
    //////////////////////////////////////////////////
    // Theme
    pstr = wcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';

	// Append theme.ini
	wcscat(szIniPath, L"theme.ini");

	this->ini.LoadFile(szIniPath);

    // Language
    this->mobile_string = ini.GetValue(
        L"language", L"mobile", L"Mobile");

    this->home_string = ini.GetValue(
        L"language", L"home", L"Home");

    this->work_string = ini.GetValue(
        L"language", L"work", L"Work");

    this->company_string = ini.GetValue(
        L"language", L"company", L"Company");

    this->email_string = ini.GetValue(
        L"language", L"email", L"E-mail");

    this->sms_string = ini.GetValue(
        L"language", L"sms", L"SMS");


    // Colors, Brushes & Fonts
    this->hbrTitlebarBackground = CreateSolidBrush(this->initColor(
        L"theme", L"TitlebarBackground", L"000000"));

    this->hbrListBackground = CreateSolidBrush(this->initColor(
        L"theme", L"ListBackground", L"182633"));

	this->hbrListGroupBackground = CreateSolidBrush(this->initColor(
		L"theme", L"ListGroupBackground", L"000000"));

	this->rgbListGroupText = this->initColor(
		L"theme", L"ListGroupText", L"ffffff");

    this->hbrListItemBackground = CreateSolidBrush(this->initColor(
		L"theme", L"ListItemBackground", L"1e1e1e"));

	this->rgbListItemText = this->initColor(
		L"theme", L"ListItemText", L"dcdcdc");
    this->hbrListItemForeground = CreateSolidBrush(this->rgbListItemText);
    this->hpenListItemForeground = CreatePen(PS_SOLID, 1, this->rgbListItemText);

    this->rgbListItemHoverBackground1 = this->initColor(
		L"theme", L"ListItemHoverBackground1", L"000000");
    
    this->rgbListItemHoverBackground2 = this->initColor(
		L"theme", L"ListItemHoverBackground2", L"282828");

    this->rgbListItemSelectedBackground1 = this->initColor(
        L"theme", L"ListItemSelectedBackground1", L"4b5ab5");

    this->rgbListItemSelectedBackground2 = this->initColor(
        L"theme", L"ListItemSelectedBackground2", L"162793");

	this->rgbListItemSelectedText = this->initColor(
		L"theme", L"ListItemSelectedText", L"e6e6e6");

	this->rgbListItemSelectedShadow = this->initColor(
		L"theme", L"ListItemSelectedShadow", L"505050");

	this->hbrListItemSeparator = CreateSolidBrush(this->initColor(
		L"theme", L"ListItemSeparator", L"323232"));

    this->hbrDetailItemSeparator = CreateSolidBrush(this->initColor(
        L"theme", L"DetailItemSeparator", L"999999"));

	this->rgbListIndicatorText = this->initColor(
		L"theme", L"ListIndicatorText", L"ffffff");
}

Settings::~Settings(void) {
    // Deallocate all memory used by the ini object
    this->ini.Reset();
}

int Settings::initColor(wchar_t * wp_section, wchar_t * wp_key,
                        wchar_t * wp_default_hex) {

    const wchar_t * wpColor = ini.GetValue(
        wp_section, wp_key, wp_default_hex);

    return parseColor(wpColor, wp_default_hex);
}

int parseColor(const wchar_t * color_hex, const wchar_t * default_hex) {
    const wchar_t * source = wcslen(color_hex) == 6 
        ? color_hex : default_hex;

    wchar_t red[3];
    wchar_t green[3];
    wchar_t blue[3];

    red[2] = 0;
    green[2] = 0;
    blue[2] = 0;

	wcsncpy(red, source, 2);
	wcsncpy(green, source + 2, 2);
	wcsncpy(blue, source + 4, 2);

	int r = wcstol(red, NULL, 16);
	int g = wcstol(green, NULL, 16);
	int b = wcstol(blue, NULL, 16);

    int rgbcolor = RGB(r, g, b);

	return rgbcolor;
}