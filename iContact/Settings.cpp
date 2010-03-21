#include <stdafx.h>
#include <string.h>

#include "Settings.h"
#include "Macros.h"

Settings::Settings(void) {
    this->ini = CSimpleIniW(false, false, false);

    TCHAR szIniPath[MAX_PATH];
    TCHAR szLanguagePath[MAX_PATH];
    TCHAR szThemePath[MAX_PATH];

    // Get program file path
	GetModuleFileName(NULL, szIniPath, MAX_PATH);

    TCHAR * pstr = _tcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';

    StringCchCopy(szLanguagePath, MAX_PATH, szIniPath);
    StringCchCopy(szThemePath, MAX_PATH, szIniPath);

    StringCchCat(szIniPath, MAX_PATH, TEXT("settings.ini"));

    //////////////////////////////////////////////////
    // Settings
	this->ini.LoadFile(szIniPath);

    this->email_account = ini.GetValue(
        MAIN_SECTION, TEXT("EmailAccount"), TEXT(""));

    this->favorite_category = ini.GetValue(
        MAIN_SECTION, TEXT("FavoriteCategory"), TEXT("Favorites"));

	this->doExitOnAction = 0 == wmemcmp(ini.GetValue(
		MAIN_SECTION, TEXT("ExitOnAction"), TEXT("0")), TEXT("1"), 1);

	this->doExitOnMinimize = 0 == wmemcmp(ini.GetValue(
		MAIN_SECTION, TEXT("ExitOnMinimize"), TEXT("0")), TEXT("1"), 1);

    this->doFastGraphics = 0 == wmemcmp(ini.GetValue(
        MAIN_SECTION, TEXT("FastGraphics"), TEXT("0")), TEXT("1"), 1);
    
    StringCchCat(szLanguagePath, MAX_PATH, ini.GetValue(
        MAIN_SECTION, TEXT("Language"), TEXT("english")));
    StringCchCat(szLanguagePath, MAX_PATH, TEXT(".lng"));

    StringCchCat(szThemePath, MAX_PATH, ini.GetValue(
        MAIN_SECTION, TEXT("Skin"), TEXT("default")));
    StringCchCat(szThemePath, MAX_PATH, TEXT(".skn"));

    this->skin_name = ini.GetValue(
        MAIN_SECTION, TEXT("Skin"), TEXT("default"));

    //////////////////////////////////////////////////
    // Language
	this->ini.LoadFile(szLanguagePath);

    const struct LanguageSetting languageSettings[] = {
        &this->alphabet, TEXT("alphabet"), TEXT(""),
        &this->mobile_string, TEXT("mobile"), TEXT("Mobile"),
        &this->home_string, TEXT("home"), TEXT("Home"),
        &this->work_string, TEXT("work"), TEXT("Work"),
        &this->company_string, TEXT("company"), TEXT("Company"),
        &this->car_string, TEXT("car"), TEXT("Car"),
        &this->assistant_string, TEXT("assistant"), TEXT("Assistant"),
        &this->fax_string, TEXT("fax"), TEXT("Fax"),
        &this->pager_string, TEXT("pager"), TEXT("Pager"),
        &this->radio_string, TEXT("radio"), TEXT("Radio"),
        &this->email_string, TEXT("email"), TEXT("E-mail"),
        &this->sms_string, TEXT("sms"), TEXT("SMS"),
        &this->today_string, TEXT("today"), TEXT("Today"),
        &this->yesterday_string, TEXT("yesterday"), TEXT("Yesterday"),
        &this->sunday_string, TEXT("sunday"), TEXT("Sunday"),
        &this->monday_string, TEXT("monday"), TEXT("Monday"),
        &this->tuesday_string, TEXT("tuesday"), TEXT("Tuesday"),
        &this->wednesday_string, TEXT("wednesday"), TEXT("Wednesday"),
        &this->thursday_string, TEXT("thursday"), TEXT("Thursday"),
        &this->friday_string, TEXT("friday"), TEXT("Friday"),
        &this->saturday_string, TEXT("saturday"), TEXT("Saturday"),
        &this->older_string, TEXT("older"), TEXT("Older"),
        &this->date_string, TEXT("date"), TEXT("Date"),
        &this->duration_string, TEXT("duration"), TEXT("Duration"),
        &this->seconds_string, TEXT("seconds"), TEXT("Seconds"),
        &this->outgoing_string, TEXT("outgoing"), TEXT("Outgoing Call"),
        &this->missed_string, TEXT("missed"), TEXT("Missed Call"),
        &this->incoming_string, TEXT("incoming"), TEXT("Incoming Call"),
        &this->unknown_string, TEXT("unknown"), TEXT("Unknown"),
        &this->returncall_string, TEXT("returncall"), TEXT("Return Call"),
        &this->savecontact_string, TEXT("savecontact"), TEXT("Save Contact"),
        &this->showsummary_string, TEXT("showsummary"), TEXT("Show Summary"),
        &this->editcontact_string, TEXT("editcontact"), TEXT("Edit Contact"),
        &this->recents_string, TEXT("recents"), TEXT("Recents"),
        &this->allcontacts_string, TEXT("allcontacts"), TEXT("All Contacts"),
        &this->details_string, TEXT("details"), TEXT("Details"),
    };

    for (int i = 0; i < ARRAYSIZE(languageSettings); i++) {
        *(languageSettings[i].ppSetting) = ini.GetValue(LANGUAGE_SECTION,
            languageSettings[i].pszName, languageSettings[i].pszDefault);
    }

    //////////////////////////////////////////////////
    // Theme
	this->ini.LoadFile(szThemePath);

    // Colors, Brushes & Fonts
    this->rgbTitlebarBackground = this->initColor(
        THEME_SECTION, TEXT("TitlebarBackground"), TEXT("000000"));

    this->rgbTitlebarText = this->initColor(
        THEME_SECTION, TEXT("TitlebarText"), TEXT("ffffff"));

    this->rgbTitlebarSignal = this->initColor(
        THEME_SECTION, TEXT("TitlebarSignal"), TEXT("ffffff"));

    this->rgbTitlebarBattery = this->initColor(
        THEME_SECTION, TEXT("TitlebarBattery"), TEXT("ffffff"));

    this->rgbTitlebarBatteryCharge = this->initColor(
        THEME_SECTION, TEXT("TitlebarBatteryCharge"), TEXT("00ff00"));

    this->rgbHeader = this->initColor(
        THEME_SECTION, TEXT("HeaderText"), TEXT("ffffff"));

    this->rgbHeaderLoading = this->initColor(
        THEME_SECTION, TEXT("HeaderLoadingText"), TEXT("7f7f7f"));

    this->hbrListBackground = CreateSolidBrush(this->initColor(
        THEME_SECTION, TEXT("ListBackground"), TEXT("182633")));

	this->hbrListGroupBackground = CreateSolidBrush(this->initColor(
		THEME_SECTION, TEXT("ListGroupBackground"), TEXT("000000")));

	this->rgbListGroupText = this->initColor(
		THEME_SECTION, TEXT("ListGroupText"), TEXT("ffffff"));

    this->hbrListItemBackground = CreateSolidBrush(this->initColor(
		THEME_SECTION, TEXT("ListItemBackground"), TEXT("1e1e1e")));

	this->rgbListItemText = this->initColor(
		THEME_SECTION, TEXT("ListItemText"), TEXT("dcdcdc"));
    this->hbrListItemForeground = CreateSolidBrush(this->rgbListItemText);
    this->hpenListItemForeground = CreatePen(PS_SOLID, 1, this->rgbListItemText);

    this->rgbListItemMissedText = this->initColor(
		THEME_SECTION, TEXT("ListItemMissedText"), TEXT("ff0000"));

    this->rgbListItemFavoriteText = this->initColor(
		THEME_SECTION, TEXT("ListItemFavoriteText"), TEXT("ffff00"));

    this->rgbListItemSelectedBackground1 = this->initColor(
        THEME_SECTION, TEXT("ListItemSelectedBackground1"), TEXT("4b5ab5"));

    this->rgbListItemSelectedBackground2 = this->initColor(
        THEME_SECTION, TEXT("ListItemSelectedBackground2"), TEXT("162793"));

    this->hbrListItemSelectedBackground = CreateSolidBrush(this->initColor(
        THEME_SECTION, TEXT("ListItemSelectedBackground1"), TEXT("4b5ab5")));

	this->rgbListItemSelectedText = this->initColor(
		THEME_SECTION, TEXT("ListItemSelectedText"), TEXT("e6e6e6"));

	this->rgbListItemSelectedShadow = this->initColor(
		THEME_SECTION, TEXT("ListItemSelectedShadow"), TEXT("505050"));

	this->hbrListItemSeparator = CreateSolidBrush(this->initColor(
		THEME_SECTION, TEXT("ListItemSeparator"), TEXT("323232")));

    this->hbrDetailItemSeparator = CreateSolidBrush(this->initColor(
        THEME_SECTION, TEXT("DetailItemSeparator"), TEXT("999999")));

    this->rgbKeyboardText = this->initColor(
		THEME_SECTION, TEXT("KeyboardText"), TEXT("dcdcdc"));

    this->hbrKeyboardBackground = CreateSolidBrush(this->initColor(
        THEME_SECTION, TEXT("KeyboardBackground"), TEXT("464646")));

    this->hpenKeyboardGrid = CreatePen(PS_SOLID, 1, this->initColor(
		THEME_SECTION, TEXT("KeyboardGrid"), TEXT("646464")));
}

Settings::~Settings(void) {
    // Deallocate all memory used by the ini object
    this->ini.Reset();
}

int Settings::initColor(TCHAR * wp_section, TCHAR * wp_key,
                        TCHAR * wp_default_hex) {

    const TCHAR * wpColor = ini.GetValue(
        wp_section, wp_key, wp_default_hex);

    return parseColor(wpColor, wp_default_hex);
}

int parseColor(const TCHAR * color_hex, const TCHAR * default_hex) {
    const TCHAR * source = _tcslen(color_hex) == 6 
        ? color_hex : default_hex;

    TCHAR red[3];
    TCHAR green[3];
    TCHAR blue[3];

	StringCchCopy(red, 3, source);
	StringCchCopy(green, 3, source + 2);
	StringCchCopy(blue, 3, source + 4);

	int r = _tcstol(red, NULL, 16);
	int g = _tcstol(green, NULL, 16);
	int b = _tcstol(blue, NULL, 16);

	return RGB(r, g, b);
}