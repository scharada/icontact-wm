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
#include "ListData.h"

//----------------------------------------------------------------------
// Generic defines and data types
//
struct decodeUINT {                             // Structure associates
    UINT Code;                                  // messages 
                                                // with a function. 
    LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
}; 

struct CmdLineArg {
    TCHAR * arg;
    WPARAM wparam;
};

enum TransitionType {
    ttSlideLeft,
    ttSlideRight,
    ttKeyboardExpand,
    ttKeyboardShrink,
};

enum ScreenType {
    stList,
    stDetails,
};

enum PopupType {
    ptKeyboard,
};

#define SZ_APP_NAME                 TEXT("iContact")
#define SZ_ABOUT                    TEXT("iContact v0.81 - supware.net")
#define ABOUT_LENGTH                28

#define CMD_GOTO_FAVORITES          0x01
#define CMD_GOTO_RECENTS            0x02
#define CMD_GOTO_CONTACTS           0x03
#define CMD_GOTO_DIALER             0x04
#define CMD_GOTO_SEARCH             0x05

#define SCROLL_THRESHOLD            5

#define IDT_TIMER_SCROLL			10010
#define IDT_TIMER_SCROLL_TO			10012
#define IDT_TIMER_TRANSITION        10015
#define IDT_TIMER_LOADLIST          10016
#define IDT_TIMER_LONG_PRESS        10017

#define SCROLL_TO_PERIOD            300
#define EXPAND_DETAILS_PERIOD       300
#define TRANSITION_PERIOD           300
#define EXPAND_KEYBOARD_PERIOD      300
#define SHRINK_KEYBOARD_PERIOD      200
#define ALPHABET_MAX_SIZE           128

#define LIST_DISPLAY_LIST			10
#define LIST_DISPLAY_DETAILS		20
#define LIST_DISPLAY_KEYBOARD		30
#define LIST_DISPLAY_ADDFAVORITES	40

#define MAX_LOADSTRING              100

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
LRESULT DoCommand(HWND, UINT, WPARAM, LPARAM);
LRESULT DoDestroyMain(HWND, UINT, WPARAM, LPARAM);

void InitSurface(HWND);

void DrawScreenOn(HDC, ScreenType, HDC, RECT, int);
void DrawListOn(HDC, HDC, RECT, int);
void DrawGroupHeaderOn(HDC, int, RECT);
void DrawItemDetailsOn(HDC, Data, int);
void DrawItemSelectedOn(HDC hdc, Data, RECT);
void DrawItemHoverOn(HDC, Data, RECT);
void DrawItemOn(HDC, Data, RECT);
void DrawKeyboardOn(HDC, RECT);
void DrawHeaderOn(HDC, ScreenType, RECT, HDC);

void CalculateHeights();
int GetPixelToItem(int);
void ScrollBar(int);
void ScrollTo (HWND, int, int = SCROLL_TO_PERIOD);
void StartTransition(HWND, TransitionType, int = TRANSITION_PERIOD);
void SwitchTab(HWND, int);
void ParseCommandLine(HWND, LPTSTR);