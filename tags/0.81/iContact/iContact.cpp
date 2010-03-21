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

// iContact.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "iContact.h"
#include "CSettings.h"
#include "ListData.h"
#include "ListDataCallLog.h"
#include "ListDataPoom.h"
#include "GraphicFunctions.h"
#include "PhoneUtils.h"
#include "Titlebar.h"

//TODO: fix memory leak when updating contact pictures

//-----------------------------------------------------------------------------
// Global data
//
HINSTANCE   hInst;                     // Program instance handle

CSettings * pSettings = NULL;
ListData *  pListData = NULL;

ScreenType  stScreenType = stList;
int         nCurrentTab = 2;

int		    ListHeight = 0;
int         AverageItemHeight = DEFAULT_ITEM_HEIGHT;
int         StartPosition[MAX_LIST_ITEMS];
int         GroupPosition[ALPHABET_MAX_SIZE];
POINT       ptMouseDown = { -1, -1 };

// Graphic
RECT		rScreen = {0};
int         nScreenHeight = {0};
RECT        rTitlebar = {0};
RECT        rHeader = {0};
RECT		rMenubar = {0};
RECT		rList = {0};
RECT        rContent = {0};
int         rListHeight = 0;
int         minScrolled = 0;
int         maxScrolled = 0;

// UI Element Sizes. These can't be static because
// of different DPI devices
int         TitlebarHeight = TITLEBAR_HEIGHT;
int         DefaultItemHeight = DEFAULT_ITEM_HEIGHT;
int         DefaultGroupHeight = DEFAULT_GROUP_HEIGHT;
int         HeaderHeight = HEADER_HEIGHT;
int         HeaderClickHeight = HEADER_CLICK_HEIGHT;
int         MenuBarHeight = MENU_BAR_HEIGHT;
int         MenuBarIconWidth = MENU_BAR_ICON_WIDTH;

int         TitlebarFontSize = TITLEBAR_FONT_SIZE;
int	        ItemFontSize = ITEM_FONT_SIZE;
int	        ItemSecondaryFontSize = ITEM_SECONDARY_FONT_SIZE;
int	        KeyboardFontSize = KEYBOARD_FONT_SIZE;
int	        ItemDetailsFontSize = ITEM_DETAILS_FONT_SIZE;
int         ItemDetailsPictureSize = ITEM_DETAILS_PICTURE_SIZE;
int         ItemDetailsPadding = ITEM_DETAILS_PADDING;
int	        GroupItemFontSize = GROUP_ITEM_FONT_SIZE;
int	        ListIndicatorFontSize = LIST_INDICATOR_FONT_SIZE;

int	        ListItemIndent = LIST_ITEM_INDENT;
int	        ListGroupItemIndent = LIST_GROUP_ITEM_INDENT;
int	        ListSeparatorHeight = LIST_SEPARATOR_HEIGHT;

// Fonts
HFONT       TitlebarFont;
HFONT		PrimaryListFont;
HFONT		SecondaryListFont;
HFONT		GroupFont;
HFONT		ItemDetailsFont;
HFONT		ListIndicatorFont;
HFONT		KeyboardFont;

// Screen buffers
HDC         hdcMem = NULL;
HBITMAP		hbmMem = NULL;
HDC         hdcTmp = NULL;
HBITMAP		hbmTmp = NULL;
HDC			hdcSkin = NULL;
HBITMAP		hbmSkin = NULL;
HDC         hdcPage1 = NULL;
HBITMAP     hbmPage1 = NULL;
HDC         hdcPage2 = NULL;
HBITMAP     hbmPage2 = NULL;
HBITMAP     hbmCanvas = NULL;
HBRUSH      hbrCanvas = NULL;

// Scrolling
bool		bDragging = false;
bool		bScrolling = false;
int			Scrolled = 0;
int         ListScrolled = 0;
int			LastX;
int			LastY;
int			tStartTime;
int			tEndTime;
double		Velocity = 0;
int         nKeyRepeatCount = 0;

// Scroll To
DWORD       Scroll_StartTime = 0;
double		Scroll_TimeCounter = 0.0;
int 		Scroll_StartPosition = 0;
int 		Scroll_Change = 0;
double		Scroll_Duration = 0.0;

// Screen Transition
DWORD       dwTransitionStart = 0;
double      dTransitionPct = 0.0;
int         nTransitionDuration = 0;
bool        bTransitioning = false;
TransitionType trTransitionType = ttSlideLeft;

// Popup window
bool        bDisplayingPopup = false;
PopupType   ptPopupType = ptKeyboard;

// Keyboard Rows/Columns
TCHAR       alphabet[ALPHABET_MAX_SIZE];
int         nKeyboardLetters = 26;
int         nKeyboardRows = 0;
int         nKeyboardCols = 0;
int		    GroupWidth = 0;     // Keyboard group width
int		    GroupHeight = 0;    // Keyboard group height



// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_PAINT, DoPaintMain,
    WM_DESTROY, DoDestroyMain,
    WM_ACTIVATE, DoActivate,
    WM_SIZE, DoSize,
    WM_LBUTTONDOWN, DoMouseDown,
    WM_MOUSEMOVE, DoMouseMove,
    WM_LBUTTONUP, DoMouseUp,
    WM_TIMER, DoTimer,
    WM_KEYDOWN, DoKeyDown,
    WM_COMMAND, DoCommand,
    WM_TITLEBAR, DoTitlebarCallback,
};


//=============================================================================
//
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    HWND hwndMain;

    // Initialize this instance.
    hwndMain = InitInstance(hInstance, lpCmdLine, nCmdShow);
    if (hwndMain == 0)
        return 0x10;

    // Application message loop
    while (GetMessage (&msg, NULL, 0, 0)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
    // Instance cleanup
    return TermInstance (hInstance, msg.wParam);
}
//-----------------------------------------------------------------------------
// InitInstance - Instance initialization
//
HWND InitInstance (HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow){
    WNDCLASS wc;
    HWND hWnd;

    // Save program instance handle in global variable.
    hInst = hInstance;

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // If Windows Mobile, allow only one instance of the application.
    hWnd = FindWindow (SZ_APP_NAME, NULL);
    if (hWnd) {
        SetForegroundWindow ((HWND)(((DWORD)hWnd) | 0x01));   
        ParseCommandLine(hWnd, lpCmdLine);
        return 0;
    }
#endif

    // Register application main window class.
    wc.style = 0;                             // Window style
    wc.lpfnWndProc = MainWndProc;             // Callback function
    wc.cbClsExtra = 0;                        // Extra class data
    wc.cbWndExtra = 0;                        // Extra window data
    wc.hInstance = hInstance;                 // Owner handle
    wc.hIcon = NULL;                          // Application icon
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);// Default cursor
    wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wc.lpszMenuName =  NULL;                  // Menu name
    wc.lpszClassName = SZ_APP_NAME;         // Window class name

    if (RegisterClass (&wc) == 0) return 0;
    // Create main window.
    hWnd = CreateWindowEx(WS_EX_NODRAG,       // Ex Style
                         SZ_APP_NAME,         // Window class
                         SZ_APP_NAME,         // Window title
                         WS_SYSMENU,          // Style flags
                                              // don't use WS_VISIBLE.
                                              // Why not? It causes an "error report"
                                              // Why? I don't know.
                         CW_USEDEFAULT,       // x position
                         CW_USEDEFAULT,       // y position
                         CW_USEDEFAULT,       // Initial width
                         CW_USEDEFAULT,       // Initial height
                         NULL,                // Parent
                         NULL,                // Menu, must be null
                         hInstance,           // Application instance
                         NULL);               // Pointer to create
                                              // parameters
    // Return fail code if window not created.
    if (!IsWindow (hWnd)) return 0;

    // Initialize COM libraries (for POOM)
    if (FAILED(CoInitializeEx(NULL, 0)))
        return 0;

    // Additional initialization

    // Perform DPI adjustments
    HDC hdc = GetDC(hWnd);
    int dpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
    if (dpi > DEFAULT_DPI) {
        TitlebarHeight = MulDiv(TitlebarHeight, dpi, DEFAULT_DPI);
        DefaultItemHeight = MulDiv(DefaultItemHeight, dpi, DEFAULT_DPI);
        DefaultGroupHeight = MulDiv(DefaultGroupHeight, dpi, DEFAULT_DPI);
        HeaderHeight = MulDiv(HeaderHeight, dpi, DEFAULT_DPI);
        HeaderClickHeight = MulDiv(HeaderClickHeight, dpi, DEFAULT_DPI);
        MenuBarHeight = MulDiv(MenuBarHeight, dpi, DEFAULT_DPI);
        MenuBarIconWidth = MulDiv(MenuBarIconWidth, dpi, DEFAULT_DPI);

        TitlebarFontSize = MulDiv(TitlebarFontSize, dpi, DEFAULT_DPI);
        ItemFontSize = MulDiv(ItemFontSize, dpi, DEFAULT_DPI);
        ItemSecondaryFontSize = MulDiv(ItemSecondaryFontSize, dpi, DEFAULT_DPI);
        KeyboardFontSize = MulDiv(KeyboardFontSize, dpi, DEFAULT_DPI);
        ItemDetailsFontSize = MulDiv(ItemDetailsFontSize, dpi, DEFAULT_DPI);
        ItemDetailsPictureSize = MulDiv(ItemDetailsPictureSize, dpi, DEFAULT_DPI);
        ItemDetailsPadding = MulDiv(ItemDetailsPadding, dpi, DEFAULT_DPI);
        GroupItemFontSize = MulDiv(GroupItemFontSize, dpi, DEFAULT_DPI);
        ListIndicatorFontSize = MulDiv(ListIndicatorFontSize, dpi, DEFAULT_DPI);

        ListItemIndent = MulDiv(ListItemIndent, dpi, DEFAULT_DPI);
        ListGroupItemIndent = MulDiv(ListGroupItemIndent, dpi, DEFAULT_DPI);
        ListSeparatorHeight = MulDiv(ListSeparatorHeight, dpi, DEFAULT_DPI);
    }

    // Initialize titlebar callbacks
    InitTitlebar(hWnd);

    // Create fonts
    TitlebarFont = BuildFont(TitlebarFontSize, FALSE, FALSE);
	PrimaryListFont = BuildFont(ItemFontSize, FALSE, FALSE);
	SecondaryListFont = BuildFont(ItemSecondaryFontSize, TRUE, FALSE);
	ItemDetailsFont = BuildFont(ItemDetailsFontSize, FALSE, FALSE);
	GroupFont = BuildFont(GroupItemFontSize, TRUE, FALSE);
    ListIndicatorFont = BuildFont(ListIndicatorFontSize, TRUE, FALSE);
	KeyboardFont = BuildFont(KeyboardFontSize, TRUE, FALSE);

    // Create data lists
    pSettings = new CSettings();
    SwitchTab(hWnd, 2);
    InitSurface(hWnd);

    // Standard show and update calls
    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);

    ParseCommandLine(hWnd, lpCmdLine);

    return hWnd;
}

//-----------------------------------------------------------------------------
// TermInstance - Program cleanup
//
int TermInstance (HINSTANCE hInstance, int nDefRC) {

    return nDefRC;
}
//=============================================================================
// Message handling procedures for MainWindow
//

//-----------------------------------------------------------------------------
// MainWndProc - Callback function for application window
//
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT wMsg, WPARAM wParam, 
                              LPARAM lParam) {
    INT i;
    //
    // Search message list to see if we need to handle this
    // message. If in list, call procedure.
    //
    for (i = 0; i < ARRAYSIZE(MainMessages); i++) {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoPaintMain - Process WM_PAINT message for window.
//
LRESULT DoPaintMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                     LPARAM lParam) {

    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;
    
    int rScreenWidth = rScreen.right - rScreen.left;
    int rScreenHeight = rScreen.bottom - rScreen.top;   

    hdc = BeginPaint (hWnd, &ps); 

    // rect is the region that needs to be painted
    rect = ps.rcPaint;

    if (bTransitioning) {
        RECT rContent = rScreen;

        if (trTransitionType == ttSlideRight 
            || trTransitionType == ttSlideLeft) {

            rContent.top = rTitlebar.bottom;
            int rHeight = rContent.bottom - rContent.top;
            int width1 = (int)(rScreenWidth * dTransitionPct);
            if (trTransitionType == ttSlideLeft)
                width1 = rScreenWidth - width1;
            int width2 = rScreenWidth - width1;

            BitBlt(hdcMem, rContent.left, rContent.top, width1, rHeight, 
                hdcPage1, width2, rContent.top, SRCCOPY);

            BitBlt(hdcMem, width1, rContent.top, width2, rHeight,
                hdcPage2, rContent.left, rContent.top, SRCCOPY);

            BitBlt(hdcMem, rTitlebar.left, rTitlebar.top, 
                rScreenWidth, rTitlebar.bottom - rTitlebar.top,
                hdcPage1, rTitlebar.left, rTitlebar.top, SRCCOPY);
        }

        else if (trTransitionType == ttKeyboardExpand 
            || trTransitionType == ttKeyboardShrink) {

            double cubic = pow(dTransitionPct - 1, 3) + 1;
            if (trTransitionType == ttKeyboardShrink)
                cubic = 1 - cubic;

            int nKbWidth = (int)(rScreenWidth * cubic);
            int nKbHeight = (int)(rScreenHeight * cubic);

            BitBlt(hdcMem, rScreen.left, rScreen.top, 
                rScreenWidth, rScreenHeight, 
                hdcPage1, rScreen.left, rScreen.top, SRCCOPY);

            // for faster rendering, don't stretch or alpha-ize the keyboard
            if (pSettings->doFastGraphics) {
                BitBlt(hdcMem, rScreen.right - nKbWidth, 
                    rScreen.bottom - nKbHeight, nKbWidth, nKbHeight,
                    hdcPage2, 0, 0, SRCCOPY);
            }
            else {
                BltAlpha(hdcMem, rScreen.right - nKbWidth, 
                    rScreen.bottom - nKbHeight, nKbWidth, nKbHeight,
                    hdcPage2, 0, 0, rScreenWidth, rScreenHeight, 220);
            }
        }
    }

    // not transitioning
    else {
        DrawScreenOn(hdcMem, stScreenType, hdcTmp, rect, Scrolled);

        if (bDisplayingPopup) {
            switch (ptPopupType) {
                case ptKeyboard:
	                // draw the keyboard
	                DrawKeyboardOn(hdcTmp, rScreen);
                    BltAlpha(hdcMem, rScreen.left, rScreen.top, rScreenWidth, rScreenHeight,
	                    hdcTmp, 0, 0, rScreenWidth, rScreenHeight, 220);
                    break;
            }
        }
    }

    // Transfer everything to the actual screen
    BitBlt(hdc, rect.left, rect.top, rect.right - rect.left,
        rect.bottom - rect.top, hdcMem, rect.left, rect.top, SRCCOPY);

    EndPaint (hWnd, &ps);

    return 0;
}

//-----------------------------------------------------------------------------
// DoActivate - Process WM_ACTIVATE message for window
//
LRESULT DoActivate (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {

    DWORD dwState;
    RECT rc;

    if (wParam == WA_CLICKACTIVE || wParam == WA_ACTIVE) {
        // To switch to full screen mode, first hide all of the shell parts.
        dwState = (SHFS_HIDETASKBAR 
            | SHFS_HIDESTARTICON 
            | SHFS_HIDESIPBUTTON);

        SHFullScreen(hWnd, dwState);

        // Next resize the main window to the size of the screen.
        SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN), 
            GetSystemMetrics(SM_CYSCREEN));
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, 
            rc.bottom-rc.top, TRUE);

        ShowWindow(hWnd, SW_SHOWNORMAL);
    }

    // The window is being deactivated... restore it to non-fullscreen
    else {
        // To switch to normal mode, first show all of the shell parts.
        dwState = (SHFS_SHOWTASKBAR 
            | SHFS_SHOWSTARTICON 
            | SHFS_SHOWSIPBUTTON);

        SHFullScreen(hWnd, dwState);

        // Next resize the main window to the size of the work area.
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE);

        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left,
            rc.bottom-rc.top, TRUE);
    }

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoTitlebarCallback - Process WM_TITLEBAR message for window
//
LRESULT DoTitlebarCallback (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {
    RefreshTitlebar(lParam);
    InvalidateRect(hWnd, &rTitlebar, false);
    return 0;
}

//-----------------------------------------------------------------------------
// DoSize - Process WM_SIZE message for window
//
LRESULT DoSize (HWND hWnd, UINT wMsg, WPARAM wParam,
                LPARAM lParam) {

	InitSurface(hWnd);

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoMouseDown - Process WM_LBUTTONDOWN message for window
//
LRESULT DoMouseDown (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

	LastX = ptMouseDown.x = LOWORD(lParam);
	LastY = ptMouseDown.y = HIWORD(lParam);
    tStartTime = ::GetTickCount();

    SetTimer(hWnd, IDT_TIMER_LONG_PRESS, 1000, NULL);

    if (bScrolling) {
        KillTimer(hWnd, IDT_TIMER_SCROLL);
        KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
        Velocity = 0;
        bScrolling = false;
    }

    if (bDisplayingPopup || bTransitioning) {
        return 0;
    }

    if (stScreenType == stList 
        && LastX > rList.right - 20 
        && LastY >= rHeader.bottom 
        && LastY < rMenubar.top) {

        bScrolling = true;
        ScrollBar(LastY);
        InvalidateRect(hWnd, &rList, false);
        UpdateWindow(hWnd);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DoMouseMove - Process WM_MOUSEMOVE message for window
//
LRESULT DoMouseMove (HWND hWnd, UINT wMsg, WPARAM wParam,
                     LPARAM lParam) {

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
    int t = ::GetTickCount();
    KillTimer(hWnd, IDT_TIMER_LONG_PRESS);

    if (bDisplayingPopup || bTransitioning) {
    }

    else if (stScreenType == stList && ptMouseDown.y >= rMenubar.top) {
    }

    else if (ptMouseDown.y < rHeader.bottom) {
    }

    // "back" button in header bar
    //TODO: back to categories from stScreenType == stList
    else if (ptMouseDown.y < rHeader.top + HeaderClickHeight 
        && ptMouseDown.x <= HeaderClickHeight
        && stScreenType == stDetails) {

    }

    // "+" button in header bar, or * in detail view
    else if (ptMouseDown.y < rHeader.top + HeaderClickHeight 
        && ptMouseDown.x >= rList.right - HeaderClickHeight 
        && (
            stScreenType == stList && pListData->CanAdd()
            || stScreenType == stDetails && pListData->CanFavorite()
        ) ) {

    }

	else if (bScrolling) {
        ScrollBar(y);
        InvalidateRect(hWnd, &rList, false);
        UpdateWindow(hWnd);
	}

	else if (bDragging 
        || abs(y - ptMouseDown.y) > SCROLL_THRESHOLD) {

        if (!bDragging) {
            if (stScreenType == stList)
                pListData->UnselectItem();
            else if (stScreenType == stDetails)
                pListData->SelectDetail(-1);
            bDragging = true;
        }

        // SCROLL
        Scrolled = Scrolled - y + LastY;
	    LastY = y;
	    LastX = x;

        Velocity = (double)(LastY - ptMouseDown.y) / (t - tStartTime);
        tEndTime = t;

        InvalidateRect(hWnd, 
            (stScreenType == stDetails ? &rContent : &rList), false);

        UpdateWindow(hWnd);
    }

	return 0;
}

//-----------------------------------------------------------------------------
// DoMouseUp - Process WM_LBUTTONUP message for window
//
LRESULT DoMouseUp (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    POINT pt;
    KillTimer(hWnd, IDT_TIMER_LONG_PRESS);

    if (bTransitioning) {
        return 0;
    }

    if (bScrolling) {
        bScrolling = false;
        InvalidateRect(hWnd, &rList, FALSE);
        UpdateWindow(hWnd);
        return 0;
    }

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

    // They clicked on the popup, no matter what screen
    if (bDisplayingPopup) {
        StartTransition(hWnd, ttKeyboardShrink, SHRINK_KEYBOARD_PERIOD);

        int keyboardIndex = (pt.y / GroupHeight) 
            * (rScreen.right - rScreen.left) / GroupWidth 
            + (pt.x - rScreen.left) / GroupWidth;
        if (keyboardIndex < nKeyboardLetters) {
            pListData->UnselectItem();
            ScrollTo(hWnd, GroupPosition[keyboardIndex]);
        }
    }


    // They clicked in the titlebar
    // no matter what the screen type is
	else if (PtInRect(&rTitlebar, ptMouseDown) && PtInRect(&rTitlebar, pt)) {
        if (pSettings->doExitOnMinimize) {
            DestroyWindow(hWnd);
        }
        else {
            ShowWindow(hWnd, SW_MINIMIZE);
        }
	}


    // They clicked in the bottom menus (in list screen)
    else if (stScreenType == stList 
        && PtInRect(&rMenubar, ptMouseDown) && PtInRect(&rMenubar, pt)) {
        int tab = pt.x * 5 / (rMenubar.right - rMenubar.left);

        // we're assuming, here, that 
        // CMD_SHOW_FAVORITES ... CMD_SHOW_KEYBOARD are sequential
        SendNotifyMessage(hWnd, WM_COMMAND, CMD_GOTO_FAVORITES + tab, NULL);
    }


    // They scrolled the screen up too far
    else if (bDragging && Scrolled < minScrolled) {
        bDragging = false;
        ScrollTo(hWnd, minScrolled);
    }


    // They scrolled the screen down too far
    else if (bDragging && Scrolled > maxScrolled) {
        bDragging = false;
        ScrollTo(hWnd, maxScrolled);
    }


    // now we're scrolling
    else if (bDragging) {
        SetTimer(hWnd, IDT_TIMER_SCROLL, 
            REFRESH_RATE, (TIMERPROC) NULL);
        bScrolling = true;
        bDragging = false;
        return 0;
    } 

    else {
        int pos = 0;

        // This is the normal *click* event
        switch (stScreenType) {
            case stList:

                // "back" button in header bar
                if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HeaderClickHeight 
                    && pt.x <= HeaderClickHeight) {

                    //TODO: back to categories
                }

                // "+" button in header bar
                else if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HeaderClickHeight 
                    && pt.x >= rList.right - HeaderClickHeight 
                    && pListData->CanAdd()) {

			        pListData->AddItem();

                    // This will refresh the data in the current tab
                    SwitchTab(hWnd, nCurrentTab);
		        }

                // Clicked a list item
                else if (pt.y >= rList.top) {
                    pos = pt.y + Scrolled - rList.top;

                    pListData->SelectItem(GetPixelToItem(pos));
                    StartTransition(hWnd, ttSlideLeft, EXPAND_DETAILS_PERIOD);
                }
                break;

            case stDetails:

                // "back" button in header bar
                if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HeaderClickHeight 
                    && pt.x <= HeaderClickHeight) {

                    // Back to list
                    StartTransition(hWnd, ttSlideRight, EXPAND_DETAILS_PERIOD);
                }

                // "*" button in header bar
                else if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HeaderClickHeight 
                    && pt.x >= rList.right - HeaderClickHeight 
                    && pListData->CanFavorite()) {

                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    pListData->ToggleFavorite();
                    SetCursor(NULL);

                    // Switch back to list of favorites, if they just
                    // de-favorited one of them. Presumably, the one they
                    // just favorited will be missing from the list now.
                    if (nCurrentTab == 0) {
                        SwitchTab(hWnd, 0);
                    }
                    else {
                        pListData->PopulateDetails();
                        InvalidateRect(hWnd, &rHeader, false);
                    }
		        }

                // Clicked a list item
                else if (pt.y >= rList.top) {
			        // HANDLE SUBLIST EVENTS
                    pos = pt.y + Scrolled - rContent.top;
                    int subListIndex = pos / DefaultItemHeight;
                    if (!pListData->SelectDetail(subListIndex))
                        break;

                    InvalidateRect(hWnd, &rContent, false);
                    UpdateWindow(hWnd);

                    // TODO: is this the best way to send SMS?
                    int column = pt.x > rList.right - HeaderClickHeight ? 2 : 1;

                    HRESULT hr = pListData->PerformCurrentDetailAction(column);

                    if (SUCCEEDED(hr)) {
                        int index = pListData->GetCurrentDetailIndex();
                        DataDetail d = pListData->GetItemDetail(index);
                        if (d.type == diEditButton || d.type == diDetailsButton) {
                            pListData->PopulateDetails();
                            CalculateHeights();
                        }
                        else {
                            if (pSettings->doExitOnAction)
                                DestroyWindow(hWnd);
                            SwitchTab(hWnd, nCurrentTab);
                        }
                    }
    		        InvalidateRect(hWnd, &rScreen, FALSE);
		        }
                if (NULL != pListData)
    		        pListData->SelectDetail(-1);

	            break;
        }
    }

    UpdateWindow(hWnd);

	return 0;
}

//-----------------------------------------------------------------------------
// DoTimer - Process WM_TIMER message for window
//
LRESULT DoTimer (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

    DWORD t = ::GetTickCount();
    DWORD dt = 0;
    double s = 0.0;
    KillTimer(hWnd, IDT_TIMER_LONG_PRESS);

	switch (wParam)	{

        ///// TIMER for scrolling
	    case IDT_TIMER_SCROLL:

            // Time
            dt = t - tEndTime;

            // Velocity
            if (Scrolled < minScrolled)
                Velocity = (double)(Scrolled - minScrolled) / 2 / dt;
            else if (Scrolled > maxScrolled)
                Velocity = (double)(Scrolled - maxScrolled) / 2 / dt;
            else {
                double dv = Velocity * FRICTION_COEFF * dt;
                if (fabs(dv) > fabs(Velocity)) 
                    Velocity = 0;
                else 
			        Velocity = Velocity - dv;
            }

            // Displacement
            s = Velocity * dt;
            if (s < 0 && s > -1 && Scrolled < minScrolled)
                s = -1;
            else if (s > 0 && s < 1 && Scrolled > maxScrolled)
                s = 1;
            
            // We're done scrolling
            if ((int)s == 0) {
                KillTimer(hWnd, IDT_TIMER_SCROLL);
		        bScrolling = false;
		        Velocity = 0;
            }

            Scrolled = Scrolled - (int)s;
            tEndTime = t;
            if (stScreenType == stList)
                InvalidateRect(hWnd, &rList, false);
            else
                InvalidateRect(hWnd, &rContent, false);
		    break;

        ///// TIMER for scroll to
        case IDT_TIMER_SCROLL_TO:
            KillTimer(hWnd, IDT_TIMER_SCROLL);
            Scroll_TimeCounter = (double)(t - Scroll_StartTime);
            if (Scroll_TimeCounter < Scroll_Duration) {
                bScrolling = true;

                // Cubic
                double amount = Scroll_Change
                    * (pow(Scroll_TimeCounter/Scroll_Duration - 1, 3) + 1);
                Velocity = (amount - Scroll_StartPosition) / Scroll_TimeCounter;
                Scrolled = Scroll_StartPosition + (int)amount;
            }
            else {
                bScrolling = false;
                Velocity = 0;
                KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
                Scrolled = Scroll_Change + Scroll_StartPosition;
            }
            if (stScreenType == stList)
                InvalidateRect(hWnd, &rList, false);
            else
                InvalidateRect(hWnd, &rContent, false);
            break;

        case IDT_TIMER_TRANSITION:
            // The list could be loading right now... wait till it's done
            if (!pListData) {
                break;
            }

            dTransitionPct = (double)(t - dwTransitionStart) 
                / nTransitionDuration;

            if (dTransitionPct >= 1.0) {
                dTransitionPct = 1.0;
                bTransitioning = false;

                if (trTransitionType == ttSlideRight) {
                    stScreenType = stList;
                    Scrolled = ListScrolled;
                    pListData->UnselectItem();
                    CalculateHeights();
                }
                else if (trTransitionType == ttSlideLeft) {
                    stScreenType = stDetails;
                    CalculateHeights();
                }
                else if (trTransitionType == ttKeyboardExpand) {
                    bDisplayingPopup = true;
                }
                else if (trTransitionType == ttKeyboardShrink) {
                    bDisplayingPopup = false;
                }

                KillTimer(hWnd, IDT_TIMER_TRANSITION);
            }
            InvalidateRect(hWnd, &rScreen, false);
            break;

        case IDT_TIMER_LOADLIST:
            SetCursor(LoadCursor(NULL, IDC_WAIT));

            pListData = 
                // Favorites
                nCurrentTab == 0 ? (ListData *)new ListDataPoom(pSettings, true)

                // Call Log
                : nCurrentTab == 1 ? (ListData *)new ListDataCallLog(pSettings)

                // Contacts
                : (ListData *)new ListDataPoom(pSettings);

            CalculateHeights();

            SetCursor(NULL);

            InvalidateRect(hWnd, &rHeader, false);
            KillTimer(hWnd, IDT_TIMER_LOADLIST);
            Scrolled = -rListHeight;
            ScrollTo(hWnd, 0);
            if (bTransitioning)
                dwTransitionStart = ::GetTickCount();
            break;

        case IDT_TIMER_LONG_PRESS:
            if (PtInRect(&rTitlebar, ptMouseDown))
                DestroyWindow(hWnd);

	} 

	UpdateWindow(hWnd);
	return 0;
}

//-----------------------------------------------------------------------------
// DoKeyDown - Process WM_KEYDOWN message for window
//
LRESULT DoKeyDown (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {
    int top = 0;
    int bot = 0;
    bool bRepeating = (lParam & (1 << 30)) != 0;
    int index;

    if (bTransitioning) {
        return 0;
    }

	switch (stScreenType) {

		case stList:
            if (bScrolling) {
                KillTimer(hWnd, IDT_TIMER_SCROLL);
			    bScrolling = false;
			    Velocity = 0;
            }

            if (bRepeating) {
                nKeyRepeatCount++;
                if (nKeyRepeatCount < 10)
                    bRepeating = false;
                else if (nKeyRepeatCount % 5 != 0)
                    return 0;
            }
            else {
                nKeyRepeatCount = 0;
            }

			switch (wParam) {
				case VK_UP:
                    pListData->SelectPreviousItem(
                        GetPixelToItem(Scrolled + rListHeight), bRepeating);

					// make sure the selected item is visible
                    index = pListData->GetCurrentItemIndex();
					top = StartPosition[index];
					bot = StartPosition[index + 1];

                    if (bScrolling) {
                        Scrolled = max(0, min(StartPosition[index], 
                            ListHeight - rListHeight));
                    }
					else if (!bScrolling && (top < Scrolled 
                        || bot > Scrolled + rListHeight)) {

                        Scrolled = max(0, bot - rListHeight);
                    }
    
					break;

			    case VK_DOWN:
                    pListData->SelectNextItem(
                        GetPixelToItem(Scrolled), bRepeating);

					// make sure the selected item is visible
                    index = pListData->GetCurrentItemIndex();
					top = StartPosition[index];
					bot = StartPosition[index + 1];

                    if (bScrolling) {
                        Scrolled = max(0, min(StartPosition[index], 
                            ListHeight - rListHeight));
                    }
                    else if (!bScrolling && (top < Scrolled 
                        || bot > Scrolled + rListHeight)) {
                            Scrolled = min(top, ListHeight - rListHeight);
                    }
    
					break;

                case VK_LEFT:
                    if (nCurrentTab > 0)
                        SwitchTab(hWnd, nCurrentTab - 1);
                    break;

                case VK_RIGHT:
                    if (nCurrentTab < 2)
                        SwitchTab(hWnd, nCurrentTab + 1);
                    else if (nCurrentTab == 2)
                        RunDialer();
                    break;

			    case VK_TACTION:
					if (pListData->GetCurrentItemIndex() >= 0)
                        StartTransition(hWnd, ttSlideLeft, 
                            EXPAND_DETAILS_PERIOD);
					break;

			}
            InvalidateRect(hWnd, &rList, FALSE);
			break;


		case stDetails:
			switch (wParam) {
                case VK_BACK:
				case VK_LEFT:
					pListData->SelectDetail(-1);
                    StartTransition(hWnd, ttSlideRight);
					break;

				case VK_UP:
					pListData->IncrementDetailIndex(-1);
					pListData->IncrementDetailIndex(-1);
                    // there's no "break" here on purpose, code optimization
				case VK_DOWN:
					pListData->IncrementDetailIndex(1);
                    top = DefaultItemHeight 
                        * pListData->GetCurrentDetailIndex();
                    bot = top + DefaultItemHeight;
                    if (top - Scrolled < 0)
                        ScrollTo(hWnd, max(0, 
                            rContent.top - rContent.bottom + bot));
                    if (bot - Scrolled > rContent.bottom - rContent.top)
                        ScrollTo(hWnd, max(
                            ListHeight - rContent.bottom + rContent.top, 
                            top));
					break;

                case VK_RIGHT:
			    case VK_TACTION:
                    HRESULT hr = pListData->PerformCurrentDetailAction(
                        wParam == VK_RIGHT ? 2 : 1);
                    
                    if (SUCCEEDED(hr)) {
                        int index = pListData->GetCurrentDetailIndex();
                        DataDetail d = pListData->GetItemDetail(index);
                        if (d.type == diEditButton || d.type == diDetailsButton) {
                            pListData->PopulateDetails();
                            CalculateHeights();
                        }
                        else {
                            if (pSettings->doExitOnAction)
                                DestroyWindow(hWnd);
                            SwitchTab(hWnd, nCurrentTab);
                        }
                    }
					break;

			}
            InvalidateRect(hWnd, &rContent, FALSE);
			break;
	}
	UpdateWindow(hWnd);

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoCommand - Process WM_COMMAND message for window
//
LRESULT DoCommand (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    switch (wParam) {
        case CMD_GOTO_FAVORITES:
            SwitchTab(hWnd, 0);
            break;
        case CMD_GOTO_RECENTS:
            SwitchTab(hWnd, 1);
            break;
        case CMD_GOTO_CONTACTS:
            SwitchTab(hWnd, 2);
            break;
        case CMD_GOTO_DIALER:
            RunDialer();
            if (pSettings->doExitOnAction)
                DestroyWindow(hWnd);
            break;
        case CMD_GOTO_SEARCH:
            if (nCurrentTab != 2)
                SwitchTab(hWnd, 2);

            StartTransition(hWnd, ttKeyboardExpand, EXPAND_KEYBOARD_PERIOD);
            break;
    }

    InvalidateRect(hWnd, &rMenubar, FALSE);
    UpdateWindow(hWnd);

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoDestroyMain - Process WM_DESTROY message for window
//
LRESULT DoDestroyMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                       LPARAM lParam) {
   
    // Uninitialize the COM classes
    CoUninitialize();

    // Quit
    PostQuitMessage (0);
    return 0;
}

//-----------------------------------------------------------------------------
// Screen Drawing Functions
//
void DrawScreenOn(HDC hdc, ScreenType st, HDC hdcTmp, 
    RECT rClip, int yListOffset) {

    // MAIN CONTENT
    switch (st) {
        case stList:
            if (rClip.bottom > rList.top || rClip.top < rList.bottom)
                DrawListOn(hdc, hdcTmp, rList, yListOffset);

            // MENU BAR
            if (rClip.bottom > rMenubar.top) {
                int rMenubarWidth = rMenubar.right - rMenubar.left;

                // draw the background of the menu bar
                // This will stretch the first column of the menu bar
                // fully across the screen
                StretchBlt(hdc, rMenubar.left, rMenubar.top, 
                    rMenubarWidth, rMenubar.bottom - rMenubar.top,
                    hdcSkin, 0, SKIN_MENU_BAR_Y_OFFSET, 1, SKIN_MENU_BAR_HEIGHT, SRCCOPY);

                // draw buttons
                for (int i = 0; i < 5; i++) {
                    int xdest = rMenubar.left 
                        + rMenubarWidth / 10 * (2 * i + 1) - MenuBarIconWidth / 2;
                    int ydest = rMenubar.top;
                    int xsrc = i * SKIN_MENU_BAR_ICON_WIDTH;
                    int ysrc = i == nCurrentTab 
                        ? SKIN_MENU_BAR_SEL_Y_OFFSET 
                        : SKIN_MENU_BAR_Y_OFFSET;
                    StretchBlt(hdc, xdest, ydest, MenuBarIconWidth, MenuBarHeight, 
                        hdcSkin, xsrc, ysrc, SKIN_MENU_BAR_ICON_WIDTH, SKIN_MENU_BAR_HEIGHT,
                        SRCCOPY);
                }
            }
            break;
        case stDetails:
            DrawItemDetailsOn(hdc, pListData->GetCurrentItem(), yListOffset);
            break;
    }

    // TITLE BAR
    if (rClip.top < rTitlebar.bottom)
	    DrawTitlebarOn(hdc, rTitlebar, hdcSkin, TitlebarFont,
            pSettings->rgbTitlebarBackground, pSettings->rgbTitlebarText,
            pSettings->rgbTitlebarSignal);

    // HEADER BAR
    DrawHeaderOn(hdc, st, rHeader, hdcSkin);
}

void DrawListOn(HDC hdc, HDC hdcTmp, RECT rList, int yOffset) {
	int nFirstItem, nItem;
    Data dItemTmp;
    TCHAR buffer[16];
    int count = NULL == pListData ? 0 : pListData->GetItemCount();

    nFirstItem = nItem = yOffset < 0 ? 0 : GetPixelToItem(yOffset);

    RECT rItem;
    rItem = rList;

    rItem.bottom = rList.top + StartPosition[nItem] - yOffset;

	// ******* DRAW LIST BACKGROUND
    FillRect(hdc, &rList, hbrCanvas);
	SetBkMode(hdc, TRANSPARENT);

    // "About" at bottom of screen
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
    SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
    ExtTextOut(hdc, rList.left + 2, rList.bottom - 2, 
        NULL, NULL, SZ_ABOUT, ABOUT_LENGTH, 0);

    if (count == 0) {
        //TODO: maybe indicate empty list?
        return;
    }

    // ******* DRAW LIST ITEMS
	while (nItem < count && rItem.bottom < rList.bottom) {
		dItemTmp = pListData->GetItem(nItem);

        rItem.top = rItem.bottom;
        rItem.bottom = rItem.top + StartPosition[nItem+1] 
            - StartPosition[nItem];
        
        if (nItem == pListData->GetCurrentItemIndex() 
            && stScreenType == stList) {
            DrawItemSelectedOn(hdc, dItemTmp, rItem);
		}
        else {
            DrawItemOn(hdc, dItemTmp, rItem);
		}

        // ****** Group Header
		if (stScreenType == stList
			&& pListData->IsItemNewGroup(nItem)
            && dItemTmp.wcGroup && rItem.top >= rList.top) {

            DrawGroupHeaderOn(hdc, nItem, rItem);
		} 

        // Next nItem
        nItem++;
	}

    // Special: Draw the group of the list nItem that's at the top of the list
    dItemTmp = pListData->GetItem(nFirstItem);
    if (dItemTmp.wcGroup && yOffset >= 0) {
        RECT rTopGroup = {rList.left, 0, rList.right, DefaultGroupHeight};
        DrawGroupHeaderOn(hdcTmp, nFirstItem, rTopGroup);

        int nHeight = DefaultGroupHeight;
        int nBottom = nHeight;

        if (pListData->IsItemNewGroup(nFirstItem + 1)) {
            nBottom = min(nBottom, StartPosition[nFirstItem + 1] - yOffset);
        }

        // account for the fact that the list
        // doesn't start at the top of the screen
        nBottom += rList.top;

        int nLeft = rList.left;
        int nWidth = rList.right - rList.left;
        int nTop = nBottom - nHeight;

        if (bScrolling && pSettings->doFastGraphics) {
            BitBlt(hdc, nLeft, nTop, nWidth, nHeight, hdcTmp, 0, 0, SRCCOPY);
        }
        else {
            BltAlpha(hdc, nLeft, nTop, nWidth, nHeight, hdcTmp, 200);
        }
    }

    // Draw list indicator if scrolling quickly
	if (bScrolling) {
		SelectObject(hdc, ListIndicatorFont);
	    SetTextAlign(hdc, TA_CENTER);
		SetTextColor(hdc, pSettings->rgbListItemText);
		SetBkMode(hdc, TRANSPARENT);

        pListData->GetItemGroup(nFirstItem, buffer);
        int length = _tcslen(buffer);

        ExtTextOut(hdc, (rList.right - rList.left) / 2 + rList.left, 
            rList.top + 10, NULL, NULL, buffer, length, 0);
	}
}

void DrawGroupHeaderOn(HDC hdc, int index, RECT rItem) {
    RECT rHeader = rItem;
    rHeader.bottom = rHeader.top + DefaultGroupHeight;
    TCHAR buffer[10];

    pListData->GetItemGroup(index, buffer);
    int length = _tcslen(buffer);

	// ****** GroupHeader background
	FillRect(hdc, &rHeader, pSettings->hbrListGroupBackground);

    // separator
    RECT rSep = rHeader;
    rSep.top = rHeader.bottom - ListSeparatorHeight;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

	// ******* Draw Group Header Text
	SelectObject(hdc, GroupFont);
   	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, pSettings->rgbListGroupText);
	ExtTextOut(hdc, rItem.left + ListGroupItemIndent, 
        rHeader.top - 1 + ((DefaultGroupHeight - GroupItemFontSize) / 2),
        NULL, NULL, buffer, length, 0);
}

void DrawItemSelectedOn(HDC hdc, Data dItem, RECT rItem) {
	// ******* DRAW ITEM BACKGROUND
	DrawGradientGDI(hdc, rItem,
        pSettings->rgbListItemSelectedBackground1,
        pSettings->rgbListItemSelectedBackground2);

	// ****** Draw Item Text
	SelectObject(hdc, PrimaryListFont);
	SetTextAlign(hdc, TA_LEFT);

    // Item Shadow Text
    if (pSettings->rgbListItemSelectedShadow 
        != pSettings->rgbListItemSelectedText) {

	    SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
	    ExtTextOut(hdc, rItem.left + ListItemIndent,
            rItem.bottom - ((DefaultItemHeight + ItemFontSize) / 2),
            ETO_OPAQUE, NULL, dItem.szPrimaryText, 
            dItem.nPrimaryTextLength, 0);
    }

    // Item Text
	SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	ExtTextOut(hdc, rItem.left + ListItemIndent, 
        rItem.bottom - 1 - ((DefaultItemHeight + ItemFontSize) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);
}

void DrawItemOn(HDC hdc, Data dItem, RECT rItem) {
    // Item Background
    FillRect(hdc, &rItem, pSettings->hbrListItemBackground);

    // separator
    RECT rSep = rItem;
    rSep.top = rItem.bottom - ListSeparatorHeight;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

    // Item Primary Text
	SelectObject(hdc, PrimaryListFont);
	SetTextColor(hdc, dItem.rgbPrimaryText);
	ExtTextOut(hdc, rItem.left + ListItemIndent,
        rItem.bottom - 2 - ((DefaultItemHeight + ItemFontSize) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

	// Item Secondary Text
    if (dItem.nSecondaryTextLength == 0)
        return;
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_RIGHT);
    SetTextColor(hdc, pSettings->rgbListItemText);
    ExtTextOut(hdc, rItem.right - ListItemIndent,
        rItem.bottom - 2 - ((DefaultItemHeight + ItemSecondaryFontSize) / 2),
	    ETO_OPAQUE, NULL, dItem.szSecondaryText, dItem.nSecondaryTextLength, 0);

}

void DrawItemDetailsOn(HDC hdc, Data dItem, int yOffset) {
    int nameHeight = 0;
    int nPictureSize = 0;
    DataDetail dd = {0};
    RECT rRow, rClip;

    SelectObject(hdc, PrimaryListFont);
    SetTextAlign(hdc, TA_LEFT);
    SetBkMode(hdc, TRANSPARENT);

    int iItemCount = pListData->GetItemDetailCount();
    int iCurrentIndex = pListData->GetCurrentDetailIndex();
    int iBitmapHeight = 0;
    int iBitmapWidth = 0;

	// ******* DRAW ITEM BACKGROUND
    FillRect(hdc, &rContent, hbrCanvas);

    // ******* Draw the current item's picture, if it exists
    HBITMAP hBitmap = pListData->GetHBitmap(ItemDetailsPictureSize);
    if (NULL != hBitmap) {
        iBitmapHeight = pListData->GetHBitmapHeight();
        iBitmapWidth = pListData->GetHBitmapWidth();

        int left = ItemDetailsPadding;
        int top = (DefaultItemHeight * 2 - iBitmapHeight) / 2 + rContent.top - yOffset;
        int right = left + iBitmapWidth;
        int bottom = top + iBitmapHeight;

        // draw black square with a black border of 1
        RECT rcBitmap = { left - 1, top - 1, right + 1, bottom + 1};
        DrawRect(hdc, &rcBitmap, (COLORREF)0);

        // paint the bitmap on the DC
        TransparentImage(hdc, left, top, iBitmapWidth, iBitmapHeight, 
            hBitmap, 0, 0, iBitmapWidth, iBitmapHeight, (COLORREF)0);
    }

    // ******* Now, draw the item data
	for (int c = 0; c < iItemCount; c++) {
        dd = pListData->GetItemDetail(c);

        if (dd.type == diNothing)
            continue;

        rRow.top = c * DefaultItemHeight + rContent.top - yOffset;
        rRow.bottom = rRow.top + DefaultItemHeight;
        if (dd.type == diName)
            rRow.bottom += DefaultItemHeight;

        rRow.left = rContent.left;

        if (rRow.top + yOffset 
            < iBitmapHeight + rContent.top + ItemDetailsPadding / 2 
            && iBitmapWidth > 0)

            rRow.left += iBitmapWidth + ItemDetailsPadding;

        rRow.right = rContent.right;

        rClip.left = rRow.left + ItemDetailsPadding / 2 + 2;
        rClip.right = rRow.right - ItemDetailsPadding / 2 - 2;
        rClip.top = rRow.top + ItemDetailsPadding / 2 + 2;
        rClip.bottom = rRow.bottom - ItemDetailsPadding / 2 - 2;

        // If this is the first item of a new group of items, draw a rectangle
        // around the group
        if ((dd.type == diPhone 
            || dd.type == diEmail 
            || dd.type == diUrl
            || dd.type == diDetailsButton 
            || dd.type == diCallButton
            || dd.type == diSmsButton
            || dd.type == diEditButton
            || dd.type == diSaveContactButton) 
            && c > 0 
            && pListData->GetItemDetail(c-1).type != dd.type) {

            int nRows = 1;
            while (c + nRows < iItemCount 
                && pListData->GetItemDetail(c + nRows).type == dd.type)
                nRows++;

            SelectObject(hdc, pSettings->hbrListItemBackground);
            SetTextColor(hdc, pSettings->rgbListItemText);
            RoundRect(hdc, 
                rRow.left + ItemDetailsPadding / 2, 
                rRow.top + ItemDetailsPadding / 3, 
                rRow.right - ItemDetailsPadding / 2, 
                rRow.bottom - ItemDetailsPadding / 3 + DefaultItemHeight * (nRows - 1),
                ItemDetailsPadding, ItemDetailsPadding);
        }

        if (rRow.bottom < 0)
            continue;

        // Indicate the currently selected item
	    if (c == iCurrentIndex) {
            SelectObject(hdc, pSettings->hbrListItemSelectedBackground);
            RoundRect(hdc, 
                rRow.left + ItemDetailsPadding / 2, 
                rRow.top + ItemDetailsPadding / 3, 
                rRow.right - ItemDetailsPadding / 2, 
                rRow.bottom - ItemDetailsPadding / 3,
                ItemDetailsPadding, ItemDetailsPadding);
            SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	    }
        else {
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
    	    SetTextColor(hdc, pSettings->rgbListItemText);
        }

        if (dd.type == diName) {
            SelectObject(hdc, PrimaryListFont);
            SetTextAlign(hdc, TA_LEFT | TA_TOP);

            // Display the shadow
            if (pSettings->rgbDetailMainShadow 
                != pSettings->rgbDetailMainText) {

                SetTextColor(hdc, pSettings->rgbDetailMainShadow);
		        ExtTextOut(hdc, rRow.left + ItemDetailsPadding + 1, 
                    rRow.top + ItemDetailsPadding + 1,
                    ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), NULL);
            }

            // Display the name
            SetTextColor(hdc, pSettings->rgbDetailMainText);
		    ExtTextOut(hdc, rRow.left + ItemDetailsPadding, 
                rRow.top + ItemDetailsPadding,
                ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), NULL);
        }

        else if (dd.type == diCompany) {
            SelectObject(hdc, SecondaryListFont);
            SetTextColor(hdc, pSettings->rgbDetailMainText);
		    ExtTextOut(hdc, rRow.left + ItemDetailsPadding, rRow.top,
                NULL, NULL, dd.text, _tcslen(dd.text), NULL);
        }

        else if (dd.type == diDetailsButton 
            || dd.type == diEditButton
            || dd.type == diCallButton
            || dd.type == diSmsButton
            || dd.type == diSaveContactButton) {
		    
            SelectObject(hdc, PrimaryListFont);
            SetTextAlign(hdc, TA_CENTER);

            ExtTextOut(hdc, (rRow.right - rRow.left) / 2 + rRow.left,
                (rRow.bottom - rRow.top - ItemFontSize) / 2 + rRow.top,
                ETO_CLIPPED, &rClip, dd.text,
                _tcslen(dd.text), 0);

        }

        else if (dd.type == diEmail
                || dd.type == diUrl) {
            SelectObject(hdc, PrimaryListFont);

            SIZE textSize;
            GetTextExtentPoint(hdc, dd.text, _tcslen(dd.text), &textSize);

            // The text is too wide to fit, so left-justify & clip it
            if (textSize.cx > rClip.right - rClip.left) {
                SetTextAlign(hdc, TA_LEFT);
                ExtTextOut(hdc, rClip.left,
                    (rRow.bottom - rRow.top - ItemFontSize) / 2 + rRow.top,
                    ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), 0);
            }

            else {
                SetTextAlign(hdc, TA_CENTER);
                ExtTextOut(hdc, (rClip.right - rClip.left) / 2 + rClip.left,
                    (rRow.bottom - rRow.top - ItemFontSize) / 2 + rRow.top,
                    NULL, NULL, dd.text, _tcslen(dd.text), 0);
            }
        }

        else {
		    SelectObject(hdc, ItemDetailsFont);
            SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
            int y = (rRow.bottom - rRow.top) / 2 + rRow.top;

            // Draw the label: "Home", "Work", etc.
            rClip.right = rRow.left + ItemDetailsPadding * 5;
            ExtTextOut(hdc, rClip.right, 
                y + (ItemDetailsFontSize / 2),
                ETO_CLIPPED, &rClip, dd.label, _tcslen(dd.label), NULL);

            rClip.left = rClip.right + 3;
            rClip.right = rRow.right - ItemDetailsPadding;

            // Draw the right label: "SMS"
            if (dd.type == diPhone) {
                rClip.right -= ItemDetailsPadding * 2;
                ExtTextOut(hdc, rRow.right - ItemDetailsPadding, 
                    y + (ItemDetailsFontSize / 2),
                    NULL, NULL, pSettings->sms_string, 
                    _tcslen(pSettings->sms_string), NULL);
            }

            SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
            SelectObject(hdc, PrimaryListFont);

		    ExtTextOut(hdc, rClip.left + ItemDetailsPadding / 3, y + (ItemFontSize / 2),
                ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), NULL);
        }
	}
}

void DrawKeyboardOn(HDC hdc, RECT rKeyboard) {
	SelectObject(hdc, KeyboardFont);
	SelectObject(hdc, pSettings->hpenKeyboardGrid);
    SetTextColor(hdc, pSettings->rgbKeyboardText);
	SetBkMode(hdc, TRANSPARENT);
    int x, y, g, h;

    FillRect(hdc, &rKeyboard, pSettings->hbrKeyboardBackground);

    SetTextAlign(hdc, TA_CENTER);

    // Draw the horizontal lines
    for (h = 0; h < nKeyboardRows; h++) {
        y = rKeyboard.top + h 
            * (rKeyboard.bottom - rKeyboard.top) / nKeyboardRows;
        MoveToEx(hdc, rKeyboard.left, y, (LPPOINT) NULL);
        LineTo(hdc, rKeyboard.right, y);
    }
    MoveToEx(hdc, rKeyboard.left, rKeyboard.bottom - 1, (LPPOINT) NULL);
    LineTo(hdc, rKeyboard.right, rKeyboard.bottom - 1);

    // Draw the vertical lines
    for (g = 0; g < nKeyboardCols; g++) {
        x = rKeyboard.left + g 
            * (rKeyboard.right - rKeyboard.left) / nKeyboardCols;
        MoveToEx(hdc, x, rKeyboard.top, (LPPOINT) NULL);
        LineTo(hdc, x, rKeyboard.bottom);
    }
    MoveToEx(hdc, rKeyboard.right - 1, rKeyboard.top, (LPPOINT) NULL);
    LineTo(hdc, rKeyboard.right - 1, rKeyboard.bottom);

    // Draw the letters
    int i = 0;
	for (h = 0; h <= nKeyboardRows; h++) {
        y = rKeyboard.top 
            + ((GroupHeight - KeyboardFontSize) / 2) 
            + (GroupHeight * h);

        for (g = 0; g < nKeyboardCols; g++) {
            x = rKeyboard.left + (GroupWidth / 2) + (GroupWidth * g);

            if (i < nKeyboardLetters) {
                ExtTextOut(hdc, x, y, NULL, NULL, &alphabet[i++], 1, 0);
            }
		}
	}
}

void DrawHeaderOn(HDC hdc, ScreenType st, RECT rHeader, HDC hdcSkin) {

    // The background of the header bar
    StretchBlt(hdc, rHeader.left, rHeader.top, 
        rHeader.right - rHeader.left, rHeader.bottom - rHeader.top,
        hdcSkin, 0, SKIN_HEADER_Y_OFFSET, 1, SKIN_HEADER_HEIGHT, SRCCOPY);

    if (!bTransitioning) {
        // The "back" button
        if (st == stDetails) {
            StretchBlt(hdc, rHeader.left, rHeader.top, 
                MenuBarIconWidth, HeaderHeight,
                hdcSkin, 0, SKIN_HEADER_Y_OFFSET, 
                SKIN_MENU_BAR_ICON_WIDTH, SKIN_HEADER_HEIGHT,
                SRCCOPY);
        }

        // The "+" to add a contact
        if (st == stList && pListData != NULL && pListData->CanAdd()) {
            StretchBlt(hdc, 
                rHeader.right - MenuBarIconWidth, rHeader.top,
                MenuBarIconWidth, HeaderHeight,
                hdcSkin, 
                SKIN_MENU_BAR_ICON_WIDTH * 4, SKIN_HEADER_Y_OFFSET, 
                SKIN_MENU_BAR_ICON_WIDTH, SKIN_HEADER_HEIGHT,
                SRCCOPY);
        }

        // The "favorite YES" icon
        if (st == stDetails && pListData->CanFavorite()) {
            Data dItem = pListData->GetCurrentItem();
            if (dItem.isFavorite) {
                StretchBlt(hdc, 
                    rHeader.right - MenuBarIconWidth, rHeader.top, 
                    MenuBarIconWidth, HeaderHeight,
                    hdcSkin, 
                    SKIN_MENU_BAR_ICON_WIDTH * 3, SKIN_HEADER_Y_OFFSET, 
                    SKIN_MENU_BAR_ICON_WIDTH, SKIN_HEADER_HEIGHT,
                    SRCCOPY);
            }

            // The "favorite NO" icon
            else {
                StretchBlt(hdc, 
                    rHeader.right - MenuBarIconWidth, rHeader.top, 
                    MenuBarIconWidth, HeaderHeight,
                    hdcSkin, 
                    SKIN_MENU_BAR_ICON_WIDTH * 2, SKIN_HEADER_Y_OFFSET, 
                    SKIN_MENU_BAR_ICON_WIDTH, SKIN_HEADER_HEIGHT,
                    SRCCOPY);
            }
        }
    }

    // The title
    SelectObject(hdc, PrimaryListFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextAlign(hdc, TA_LEFT);

    SetTextColor(hdc, 
        NULL == pListData
        ? pSettings->rgbHeaderLoading
        : pSettings->rgbHeader
    );

    if (st == stList) {
        DrawText(hdc, 
            ( nCurrentTab == 0 ? pSettings->favorite_category
            : nCurrentTab == 1 ? pSettings->recents_string
            : pSettings->allcontacts_string),
            -1, &rHeader, DT_CENTER | DT_VCENTER
        );
    }
    else if (st == stDetails) {
        DrawText(hdc, pListData->GetCurrentDetailTitle(),
        -1, &rHeader, DT_CENTER | DT_VCENTER);
    }
}

//-----------------------------------------------------------------------------
// Utility functions
//

void InitSurface(HWND hWnd) {
    HRESULT hr = S_OK;
	HDC hdc;
	hdc = GetDC(hWnd);

    // Update the RECTs for the individual sections
	GetClientRect(hWnd, &rScreen);
    nScreenHeight = rScreen.bottom - rScreen.top;
    int nScreenWidth = rScreen.right - rScreen.left;

    // Title bar, with date, carrier, battery, signal strength, etc.
	rTitlebar = rScreen;
	rTitlebar.bottom = rTitlebar.top + TitlebarHeight;

    // Header, with the "back" button, the "favorite" button, etc.
    rHeader = rScreen;
    rHeader.top = rTitlebar.bottom;
    rHeader.bottom = rHeader.top + HeaderHeight;

    // Menu at the bottom of the screen
	rMenubar = rScreen;
	rMenubar.top = rMenubar.bottom - MenuBarHeight;

    // From the header to the bottom of the screen
    rContent = rScreen;
    rContent.top = rHeader.bottom;

    // Between the header and the menu bar
	rList = rContent;
	rList.bottom = rMenubar.top;
    rListHeight = rList.bottom - rList.top;

    // Calculate how many rows / cols for the keyboard
    double screenRatio = (double)nScreenWidth / nScreenHeight;
    nKeyboardRows = 1;
    nKeyboardCols = 1;
    while (nKeyboardRows * nKeyboardCols < nKeyboardLetters) {
        if (nScreenWidth < nScreenHeight) {
            nKeyboardRows = (int)(++nKeyboardCols / screenRatio);
        }
        else {
            nKeyboardCols = (int)(++nKeyboardRows * screenRatio);
        }
    }
    if (nScreenWidth < nScreenHeight) {
        while ((nKeyboardRows - 1) * nKeyboardCols >= nKeyboardLetters) {
            nKeyboardRows--;
        }
    }
    else {
        while (nKeyboardRows * (nKeyboardCols - 1) >= nKeyboardLetters) {
            nKeyboardCols--;
        }
    }
    GroupWidth = nScreenWidth / nKeyboardCols;
    GroupHeight = nScreenHeight / nKeyboardRows;


    // Initialize the DCs and Bitmaps
    if (hdcMem)
        CBR(DeleteDC(hdcMem));
    hdcMem = CreateCompatibleDC(hdc);
    if (hbmMem)
        CBR(DeleteObject(hbmMem));
	hbmMem = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcMem, hbmMem);

    if (hdcTmp)
        CBR(DeleteDC(hdcTmp));
    hdcTmp = CreateCompatibleDC(hdc);
	if (hbmTmp)
        CBR(DeleteObject(hbmTmp));
	hbmTmp = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcTmp, hbmTmp);

    if (hdcPage1)
        CBR(DeleteDC(hdcPage1));
    hdcPage1 = CreateCompatibleDC(hdc);
    if (hbmPage1)
        CBR(DeleteObject(hbmPage1));
    hbmPage1 = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcPage1, hbmPage1);

    if (hdcPage2)
        CBR(DeleteDC(hdcPage2));
    hdcPage2 = CreateCompatibleDC(hdc);
    if (hbmPage2)
        CBR(DeleteObject(hbmPage2));
    hbmPage2 = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcPage2, hbmPage2);

    if (!hbmSkin) {
        // Calculate skin filename
	    TCHAR szSkinFileName[MAX_PATH];
	    GetModuleFileName(NULL, szSkinFileName, MAX_PATH);
	    TCHAR * pstr = _tcsrchr(szSkinFileName, '\\');
	    if (pstr) *(++pstr) = '\0';
	    StringCchCat(szSkinFileName, MAX_PATH, pSettings->skin_name);
        StringCchCat(szSkinFileName, MAX_PATH, TEXT(".png"));

        hbmSkin = SHLoadImageFile(szSkinFileName);

	    // Load skin
	    hdcSkin = CreateCompatibleDC(hdc);
        SelectObject(hdcSkin, hbmSkin);

        // find the size of the skin
        BITMAP bmSkin;
        GetObject(hbmSkin, sizeof(bmSkin), &bmSkin);
        int nSkinWidth = bmSkin.bmWidth;
        int nSkinHeight = bmSkin.bmHeight;

        // Create canvas brush by copying the appropriate region
        // from the skin into a bitmap
        HDC hdcCanvas = CreateCompatibleDC(hdc);
        hbmCanvas = CreateCompatibleBitmap(hdc, nSkinWidth, SKIN_CANVAS_HEIGHT);
        SelectObject(hdcCanvas, hbmCanvas);

        // for compatibility with older skins...
        if (nSkinHeight != SKIN_CANVAS_Y_OFFSET + SKIN_CANVAS_HEIGHT) {
            RECT rCanvas = {0, 0, nSkinWidth, SKIN_CANVAS_HEIGHT};
            FillRect(hdcCanvas, &rCanvas, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        else {
            BitBlt(hdcCanvas, 0, 0, nSkinWidth, SKIN_CANVAS_HEIGHT, 
                hdcSkin, 0, SKIN_CANVAS_Y_OFFSET, SRCCOPY);
        }

        hbrCanvas = CreatePatternBrush(hbmCanvas);
        CBR(DeleteDC(hdcCanvas));
    }

    CBR(ReleaseDC(hWnd, hdc));

Error:
    ASSERT(SUCCEEDED(hr));
}

void CalculateHeights() {
	int c = 0;

    TCHAR letter[2];
    TCHAR * pdest;
    int index;

    letter[1] = 0;
    int count = 0;

    if (stScreenType == stDetails) {
        if (NULL != pListData) {
            count = pListData->GetItemDetailCount();
            c = (count - 1) * DefaultItemHeight;
        }
    }
    else {
        if (NULL != pListData)
            count = pListData->GetItemCount();

        nKeyboardLetters = _tcslen(pSettings->alphabet);
        bool bAutoAlphabet = nKeyboardLetters == 0;

        StringCchCopy(alphabet, ALPHABET_MAX_SIZE, pSettings->alphabet);

        for (int i = 0; i < ALPHABET_MAX_SIZE; i++) {
            GroupPosition[i] = -1;
            StartPosition[i] = 0;
        }

        if (count == 0) {
            ListHeight = maxScrolled = 0;
            AverageItemHeight = DefaultItemHeight;
            return;
        }

	    for (int i = 0; i < count; i++) {
            StartPosition[i] = c;

            int h = DefaultItemHeight;

            if (pListData->IsItemNewGroup(i) && pListData->GetItem(i).wcGroup) {
                h += DefaultGroupHeight;

                letter[0] = pListData->GetItem(i).wcGroup;

                pdest = wcsstr(pSettings->alphabet, letter);
                index = (int)(pdest - pSettings->alphabet);
                if (index >= 0 && index < ALPHABET_MAX_SIZE) {
                    GroupPosition[index] = c;
                }
                else if (bAutoAlphabet && nKeyboardLetters < ALPHABET_MAX_SIZE) {
                    GroupPosition[nKeyboardLetters] = c;
                    alphabet[nKeyboardLetters] = letter[0];
                    nKeyboardLetters++;
                }
            }

            c += h;
	    }

        if (GroupPosition[0] == -1)
            GroupPosition[0] = 0;

        for (int i = 1; i < ALPHABET_MAX_SIZE; i++) {
            if (GroupPosition[i] == -1)
                GroupPosition[i] = GroupPosition[i-1];
        }

        StartPosition[count] = c;
    }

	ListHeight = c;
    maxScrolled = ListHeight > rListHeight ? ListHeight - rListHeight : 0;

    AverageItemHeight = ListHeight / count;
}

int GetPixelToItem(int y) {
    y = min(ListHeight - 1, y);
    y = max(0, y);

    // estimate based on DefaultItemHeight
    int guess = y / AverageItemHeight;
    int max = NULL == pListData ? 0 : pListData->GetItemCount();
    if (guess > max)
        guess = max;

    while (y < StartPosition[guess] && guess > 0) {guess--;}

    while (y >= StartPosition[guess+1] && guess < max) {guess++;}

    return guess;
}

void ScrollBar(int y) {
	bScrolling = true;
	Velocity = 20;

    // if "Contacts", scroll by chunks of A,B,C,etc.
    if (nCurrentTab == 2) {
        int index = (y - rList.top) * nKeyboardLetters / rListHeight;
        Scrolled = index < 0 ? 0
            : index >= nKeyboardLetters ? maxScrolled
            : GroupPosition[index];
    }

    // otherwise, just do a normal scroll
    else {
        double pct = (double)(y - rList.top) 
            / (double)rListHeight;
	    Scrolled = (int)(maxScrolled * pct);
    }

    Scrolled = min(Scrolled, maxScrolled);
    Scrolled = max(Scrolled, 0);
}

void ScrollTo(HWND hWnd, int position, int duration) {
    if (position < minScrolled)
        position = minScrolled;
    if (position > maxScrolled)
        position = maxScrolled;

    Scroll_StartPosition = Scrolled;
	Scroll_Change = position - Scroll_StartPosition;
	Scroll_Duration = duration;
	Scroll_StartTime = ::GetTickCount();
	Scroll_TimeCounter = 0;
	SetTimer(hWnd, IDT_TIMER_SCROLL_TO, REFRESH_RATE, NULL);
}

void StartTransition(HWND hWnd, TransitionType tr, int duration) {
    if (tr == ttSlideLeft)
        if (FAILED(pListData->PopulateDetails()))
            return;

    bTransitioning = true;
    nTransitionDuration = duration;
    dTransitionPct = 0.0;
    trTransitionType = tr;
    dwTransitionStart = ::GetTickCount();

    InvalidateRect(hWnd, &rScreen, FALSE);

    if (tr == ttSlideLeft || tr == ttSlideRight) {
        if (tr == ttSlideLeft) {
            ListScrolled = Scrolled;
            Scrolled = 0;
        }
        DrawScreenOn(hdcPage1, stList, hdcTmp, rScreen, ListScrolled);
        DrawScreenOn(hdcPage2, stDetails, hdcTmp, rScreen, Scrolled);
    }
    else if (tr == ttKeyboardExpand || tr == ttKeyboardShrink) {
        DrawScreenOn(hdcPage1, stList, hdcTmp, rScreen, Scrolled);
        DrawKeyboardOn(hdcPage2, rScreen);
    }

	SetTimer(hWnd, IDT_TIMER_TRANSITION, REFRESH_RATE, NULL);
}

void SwitchTab(HWND hWnd, int which) {
    if (pListData) {
        pListData->Release();
        delete pListData;
    }
    stScreenType = stList;
    nCurrentTab = which;
    Scrolled = 0;
    bScrolling = false;
    bDragging = false;
    InvalidateRect(hWnd, &rMenubar, false);
    InvalidateRect(hWnd, &rHeader, false);
    InvalidateRect(hWnd, &rList, false);

    pListData = NULL;
    SetTimer(hWnd, IDT_TIMER_LOADLIST, REFRESH_RATE, NULL);
}

void ParseCommandLine(HWND hWnd, LPTSTR lpCmdLine) {
    const struct CmdLineArg cmdLineArgs[] = {
        TEXT("-favorites"), CMD_GOTO_FAVORITES,
        TEXT("-recents"), CMD_GOTO_RECENTS,
        TEXT("-contacts"), CMD_GOTO_CONTACTS,
        TEXT("-dialer"), CMD_GOTO_DIALER,
        TEXT("-search"), CMD_GOTO_SEARCH,
    };

    if (_tcslen(lpCmdLine) > 0) {
        for (int i = 0; i < ARRAYSIZE(cmdLineArgs); i++) {
            if (_tcscmp(lpCmdLine, cmdLineArgs[i].arg) == 0) {
                SendNotifyMessage(hWnd, WM_COMMAND, cmdLineArgs[i].wparam, NULL);
                return;
            }
        }
    }
}