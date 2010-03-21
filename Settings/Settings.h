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

#pragma once
#include "resourceppc.h"

#define MAX_LOADSTRING 100
#define MAX_OPTIONS 32

//----------------------------------------------------------------------
// Generic defines and data types
//
struct decodeUINT {                             // Structure associates
    UINT Code;                                  // messages 
                                                // with a function. 
    LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
}; 

enum SettingType {
    stList,
    stOnOff,
};

struct SingleSetting {
    const TCHAR * caption;
    const TCHAR * key;
    const TCHAR * def;
    SettingType type;
    int (*filler)(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR *);
};

enum TransitionType {
    ttExpand,
    ttContract,
};

enum PopupType {
    ptYesNo,
};

#define SZ_APP_NAME                 TEXT("iContact Settings")
#define SZ_ABOUT                    TEXT("iContact v0.8 - supware.net")
#define ABOUT_LENGTH                27
#define MAX_LIST_ITEMS              50

#define SZ_DOWNLOAD_SKINS           TEXT("download more skins...")
#define URL_DOWNLOAD_SKINS          TEXT("http://supware.net/iContact/skins")
#define SZ_DOWNLOAD_LANGUAGES       TEXT("download more languages...")
#define URL_DOWNLOAD_LANGUAGES      TEXT("http://supware.net/iContact/language")

#define SZ_ON                       TEXT("ON")
#define SZ_OFF                      TEXT("OFF")
#define SZ_AUTO                     TEXT("AUTO")

#define SCROLL_THRESHOLD            5

#define IDT_TIMER_SCROLL			10010
#define IDT_TIMER_SCROLL_TO			10012
#define IDT_TIMER_TRANSITION        10015

#define SCROLL_TO_PERIOD            300
#define EXPAND_DETAILS_PERIOD       300
#define TRANSITION_PERIOD           300
#define EXPAND_PERIOD               300
#define CONTRACT_PERIOD             200
#define ALPHABET_MAX_SIZE           128

#define	REFRESH_RATE                11
#define FRICTION_COEFF              0.001

// Sizes of skin elements
#define SKIN_HEADER_HEIGHT          24
#define SKIN_HEADER_Y_OFFSET        16
#define SKIN_MENU_BAR_HEIGHT        40
#define SKIN_MENU_BAR_Y_OFFSET      40
#define SKIN_MENU_BAR_SEL_Y_OFFSET  80
#define SKIN_MENU_BAR_ICON_WIDTH    48
#define SKIN_CANVAS_HEIGHT          4
#define SKIN_CANVAS_Y_OFFSET        120
#define SKIN_WIDTH                  240

// These sizes are for screens of 96 DPI only
// For 192 DPI, these will be adjusted
#define DEFAULT_DPI                 96

#define TITLEBAR_HEIGHT             16

#define DEFAULT_ITEM_HEIGHT			36
#define DEFAULT_GROUP_HEIGHT        17
#define HEADER_HEIGHT               24
#define HEADER_CLICK_HEIGHT         40
#define MENU_BAR_HEIGHT             40
#define MENU_BAR_ICON_WIDTH         48

#define TITLEBAR_FONT_SIZE          13
#define	ITEM_FONT_SIZE              20
#define	ITEM_SECONDARY_FONT_SIZE    12
#define	KEYBOARD_FONT_SIZE          40
#define	ITEM_DETAILS_FONT_SIZE      12
#define ITEM_DETAILS_PICTURE_SIZE   64
#define ITEM_DETAILS_PADDING        10
#define	GROUP_ITEM_FONT_SIZE        13
#define	LIST_INDICATOR_FONT_SIZE    50

#define	LIST_ITEM_INDENT            8
#define	LIST_GROUP_ITEM_INDENT      14
#define	LIST_SEPARATOR_HEIGHT       1


//----------------------------------------------------------------------
// Function prototypes
//
HWND InitInstance(HINSTANCE, LPWSTR, int);
int TermInstance(HINSTANCE, int);

// Window procedures
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

// Message handlers
LRESULT DoPaintMain(HWND, UINT, WPARAM, LPARAM);
LRESULT DoActivate(HWND, UINT, WPARAM, LPARAM);
LRESULT DoTitlebarCallback (HWND, UINT, WPARAM, LPARAM);
LRESULT DoSize(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseMove(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseUp(HWND, UINT, WPARAM, LPARAM);
LRESULT DoTimer(HWND, UINT, WPARAM, LPARAM);
LRESULT DoKeyDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoDestroyMain(HWND, UINT, WPARAM, LPARAM);

void InitSurface(HWND);

void DrawScreenOn(HDC, HDC, RECT);
void DrawListDetailsOn(HDC hdc, RECT rItem, const TCHAR * value);

void ScrollBar(int);
void ScrollTo (HWND, int, int = SCROLL_TO_PERIOD);
void CalculateHeights();
void ScrollTo(HWND hWnd, int position, int duration);
void StartTransition(HWND, TransitionType, int = TRANSITION_PERIOD);
void ExpandIt(HWND hWnd, int index);
HFONT BuildFont(int iFontSize, BOOL bBold, BOOL bItalic);

// load/save
void LoadSettings(void);
void SaveSettings(void);
void LoadDefaults(void);

// generic setting handlers
void DrawOnOff(HDC, RECT, bool);
void DrawValue(HDC, RECT, const TCHAR *);

// setting handlers
int emailAccountFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
int favoriteCategoryFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
HRESULT _initPoom(void);

int skinFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
int languageFiller(TCHAR [MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value);
