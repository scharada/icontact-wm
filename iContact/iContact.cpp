// iContact.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "iContact.h"
#include "Settings.h"
#include "ListData.h"
#include "ListDataCallLog.h"
#include "ListDataPoom.h"
#include "GraphicFunctions.h"
#include "PhoneUtils.h"
#include "Titlebar.h"

// TODO: add CHR() etc. for error detection
//-----------------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = SZ_APP_NAME;
HINSTANCE   hInst;                     // Program instance handle

Settings *  pSettings = NULL;
ListData *  pListData = NULL;

int         nListDisplayType = LIST_DISPLAY_LIST;
int         nCurrentTab = 2;

Data	    dItem;              // temporary storage for data item

int		    ListHeight = 0;
int         AverageItemHeight = DEFAULT_ITEM_HEIGHT;
int         StartPosition[MAX_LIST_ITEMS];
int         GroupPosition[ALPHABET_MAX_SIZE];
int		    ListItemHover = -1;
int		    ListItemSelected = -1;
int		    LastListItemSelected = -1;
bool	    ItemEdit = false;
bool	    ItemBack = false;

// Graphic
RECT		rScreen;
int         nScreenHeight;
RECT        rTitlebar;
RECT		rMenubar;
RECT		rList;
int         rListHeight;

HFONT		PrimaryListFont;
HFONT		SecondaryListFont;
HFONT		GroupFont;
HFONT		ItemDetailsFont;
HFONT		ListIndicatorFont;
HFONT		KeyboardFont;

HBITMAP		hbmMem;
HBITMAP		hbmTmp;
HBITMAP     hbmTmp2;
HDC			hdcSkin;
HBITMAP		hbmSkin;

// Scrolling
bool		bDragging = false;
bool		bScrolling = false;
bool		bClickable = true;
int			Scrolled = 0;
int			StartX;
int			StartY;
int			LastX;
int			LastY;
int			tStartTime;
int			tEndTime;
double		Speed = 0;
double		Offset = 0;
int         nKeyRepeatCount = 0;

// Scroll To
double		Scroll_TimeCounter = 0.0;
double		Scroll_StartPosition = 0.0;
double		Scroll_Change = 0.0;
double		Scroll_Duration = 0.0;

// Screen Transition
double      dTransitionPct = 0.0;
double      dTransitionStep = 0.0;
bool        bTransitioning = false;
int         nTransitionType = T_SLIDE_LEFT;

// Keyboard Switch
double		Keyboard_Start = 0.0;
double		Keyboard_Change = 0.0;
double		Keyboard_Duration = 0.0;
double		Keyboard_TimeCounter = 0.0;
int			Keyboard_Ratio = 0;
bool		KeyboardShown = false;

// Keyboard Rows/Columns
TCHAR       alphabet[ALPHABET_MAX_SIZE];
int         nKeyboardLetters = 26;
int         nKeyboardRows = 0;
int         nKeyboardCols = 0;
int		    GroupWidth = 0;     // Keyboard group width
int		    GroupHeight = 0;    // Keyboard group height

bool		Plus_Down = false;



// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_PAINT, DoPaintMain,
    WM_DESTROY, DoDestroyMain,
    WM_ACTIVATE, DoActivate,
    WM_SIZE, DoSize,
    WM_LBUTTONDOWN, DoLButtonDown,
    WM_MOUSEMOVE, DoMouseMove,
    WM_LBUTTONUP, DoLButtonUp,
    WM_TIMER, DoTimer,
    WM_KEYDOWN, DoKeyDown,
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
    hWnd = FindWindow (szAppName, NULL);
    if (hWnd) {
        SetForegroundWindow ((HWND)(((DWORD)hWnd) | 0x01));    
        return 0;
    }
#endif

    // Register application main window class.
    wc.style = 0;                             // Window style
    wc.lpfnWndProc = MainWndProc;             // Callback function
    wc.cbClsExtra = 0;                        // Extra class data
    wc.cbWndExtra = 0;                        // Extra window data
    wc.hInstance = hInstance;                 // Owner handle
    wc.hIcon = NULL,                          // Application icon
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);// Default cursor
    wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wc.lpszMenuName =  NULL;                  // Menu name
    wc.lpszClassName = szAppName;             // Window class name

    if (RegisterClass (&wc) == 0) return 0;
    // Create main window.
    hWnd = CreateWindowEx (WS_EX_NODRAG,      // Ex Style
                         szAppName,           // Window class
                         TEXT("iContact"),    // Window title
                         WS_VISIBLE,          // Style flags
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

    // Initialize titlebar callbacks
    InitTitlebar(hWnd);

    // Create fonts
	PrimaryListFont = BuildFont(ITEM_FONT_SIZE, FALSE, FALSE);
	SecondaryListFont = BuildFont(ITEM_SECONDARY_FONT_SIZE, TRUE, FALSE);
	ItemDetailsFont = BuildFont(ITEM_DETAILS_FONT_SIZE, FALSE, FALSE);
	GroupFont = BuildFont(GROUP_ITEM_FONT_SIZE, TRUE, FALSE);
	ListIndicatorFont = BuildFont(LIST_INDICATOR_FONT_SIZE, TRUE, FALSE);
	KeyboardFont = BuildFont(KEYBOARD_FONT_SIZE, TRUE, FALSE);

    // Create data lists
    pSettings = new Settings();
    SwitchTab(hWnd, 2);
    InitSurface(hWnd);

    // Standard show and update calls
    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);
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
    for (i = 0; i < dim(MainMessages); i++) {
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

    hdc = BeginPaint (hWnd, &ps); 

    // rect is the region that needs to be painted
    rect = ps.rcPaint;
    
    HDC hdcMem = CreateCompatibleDC(hdc);
    HANDLE hbmMemOld = SelectObject(hdcMem, hbmMem);

    HDC hdcTmp = CreateCompatibleDC(hdc);
    HANDLE hbmTmpOld = SelectObject(hdcTmp, hbmTmp);

    if (bTransitioning) {
        HDC hdcTmp2 = CreateCompatibleDC(hdc);

        int rListWidth = rList.right - rList.left;
        
        int rRightWidth = (int)(rListWidth * dTransitionPct);
        if (nTransitionType == T_SLIDE_RIGHT)
            rRightWidth = rListWidth - rRightWidth;

        int rLeftWidth = rListWidth - rRightWidth;

        DrawListOn(hdcTmp, hdcTmp2, rList);
        BitBlt(hdcMem, rList.left, rList.top, rLeftWidth, rListHeight, 
            hdcTmp, rRightWidth, rList.top, SRCCOPY);

        dItem = pListData->GetItem(ListItemSelected);
        DrawItemDetailsOn(hdcTmp, dItem, rList);
        BitBlt(hdcMem, rLeftWidth, rList.top, rRightWidth, rListHeight,
            hdcTmp, rList.left, rList.top, SRCCOPY);

        DeleteDC(hdcTmp2);
    }
    else {
	    // LIST / DETAILS
        if (rect.bottom > rList.top || rect.top < rList.bottom) { 
            if (nListDisplayType == LIST_DISPLAY_DETAILS) {
                DrawItemDetailsOn(hdcMem, dItem, rList);
            }
            else {
    	        DrawListOn(hdcMem, hdcTmp, rList);
            }
        }
    }

    // TITLE BAR
    if (rect.top < rTitlebar.bottom)
	    DrawTitlebarOn(hdcMem, rTitlebar, hdcSkin, 
            pSettings->rgbTitlebarBackground, pSettings->rgbTitlebarText,
            pSettings->rgbTitlebarSignal);

    // MENU BAR
    if (rect.bottom > rMenubar.top) {
        // draw the background of the menu bar
        StretchBlt(hdcMem, rMenubar.left, rMenubar.top, 
            rMenubar.right - rMenubar.left, rMenubar.bottom - rMenubar.top,
            hdcSkin, 0, MENU_BAR_Y_OFFSET, 1, MENU_BAR_HEIGHT, SRCCOPY);

        // draw buttons
        for (int i = 0; i < 5; i++) {
            int xdest = rMenubar.left + (rMenubar.right - rMenubar.left) / 10 * (2 * i + 1) - 24;
            int ydest = rMenubar.top;
            int xsrc = i * 48;
            int ysrc = i == nCurrentTab ? MENU_BAR_SELECTED_Y_OFFSET : MENU_BAR_Y_OFFSET;
            StretchBlt(hdcMem, xdest, ydest, 48, MENU_BAR_HEIGHT, 
                hdcSkin, xsrc, ysrc, 48, MENU_BAR_HEIGHT,
                SRCCOPY);
        }
    }

    // POPUP KEYBOARD
    if (nListDisplayType == LIST_DISPLAY_KEYBOARD) {
	    // draw the keyboard
	    DrawKeyboardOn(hdcTmp, rScreen);
	
        int nClientWidth = rScreen.right - rScreen.left;
        int nClientHeight = nScreenHeight;
	    int nKbWidth = (int)(nClientWidth * Keyboard_Ratio / 100);
	    int nKbHeight = (int)(nClientHeight * Keyboard_Ratio / 100);

        // for faster rendering, don't stretch or alpha-ize the keyboard
        if (Keyboard_Ratio != 100 && pSettings->doFastGraphics) {
	        BitBlt(hdcMem, rScreen.right - nKbWidth, rScreen.bottom - nKbHeight, nKbWidth, nKbHeight,
		        hdcTmp, 0, 0, SRCCOPY);
        }
        else {
	        BltAlpha(hdcMem, rScreen.right - nKbWidth, rScreen.bottom - nKbHeight, nKbWidth, nKbHeight,
		        hdcTmp, 0, 0, nClientWidth, nClientHeight, 220);
        }
    }

    // Transfer everything to the actual screen
	BitBlt(hdc, rScreen.left, rScreen.top, rScreen.right - rScreen.left,
        nScreenHeight, hdcMem, rScreen.left, rScreen.top, SRCCOPY);

    SelectObject(hdcTmp, hbmTmpOld);
    DeleteDC(hdcTmp);

    SelectObject(hdcMem, hbmMemOld);
    DeleteDC(hdcMem);

    EndPaint (hWnd, &ps);

    DeleteDC(hdc);
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
// DoLButtonDown - Process WM_LBUTTONDOWN message for window
//
LRESULT DoLButtonDown (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
    StopScrolling(hWnd);

    if (nListDisplayType == LIST_DISPLAY_KEYBOARD) {
    }

	else if (y < rTitlebar.bottom) {
        if (pSettings->doExitOnMinimize) {
            DestroyWindow(hWnd);
        }
        else {
            ShowWindow(hWnd, SW_MINIMIZE);
        }
	}

	// They clicked in the bottom menus
	else if (y >= rMenubar.top) {

        int tabWidth = (rMenubar.right - rMenubar.left) / 5;

        if (x < tabWidth * 3) {
            SwitchTab(hWnd, x / tabWidth);
        }

		else if (x < tabWidth * 4) { // Dialer
            RunDialer();
            if (pSettings->doExitOnAction)
                DestroyWindow(hWnd);
		}

		//else { // Voicemail
        //    CallVmail();
		//}
        else { // Keyboard
            if (nCurrentTab == 2) {
			    nListDisplayType = LIST_DISPLAY_KEYBOARD;
			    SwitchKeyboard(hWnd, 1, 200);
            }
        }
	}

	else if (nListDisplayType == LIST_DISPLAY_LIST && bClickable) {
		// virtual scroll bar
		if (x > rList.right - 20) {
            ScrollBar(y);
		}

		else {
			LastX = StartX = x;
			LastY = StartY = y;
			tStartTime = GetTickCount();
		}
	}

	else if (nListDisplayType == LIST_DISPLAY_DETAILS) {
		if ((y > rList.top) && (y < rList.top + DEFAULT_ITEM_HEIGHT)) {
			if (x <= 40) {
				ItemBack = true;
			}
            else if (x >= rList.right - 40 && pListData->CanFavorite()) {
                Plus_Down = true;
            }
            else {
				ItemEdit = true;
			}
		} 
        else {
            int subListIndex = (y - rList.top) / DEFAULT_ITEM_HEIGHT - 1;
            pListData->SetSubListCurrentActionIndex(subListIndex);
			if (pListData->GetSubListCurrentAction().canSms && x > 200)
				pListData->SetSms(true);
		}
	}

	RedrawList(hWnd);
    return 0;
}

//-----------------------------------------------------------------------------
// DoMouseMove - Process WM_MOUSEMOVE message for window
//
LRESULT DoMouseMove (HWND hWnd, UINT wMsg, WPARAM wParam,
                     LPARAM lParam) {

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	int deltaY = 0;

	switch (nListDisplayType) {
		case LIST_DISPLAY_LIST:
			if (bScrolling) {
                ScrollBar(y);
				RedrawList(hWnd);
				break;
			}

			// SCROLL
			deltaY = y - LastY;

			if (abs(deltaY) > 3) {
                ListItemSelected = -1;
                MoveList(hWnd, deltaY);
				LastY = y;
			}

			bDragging = StartY != LastY;

			LastX = x;
			break;

		case LIST_DISPLAY_DETAILS:
			break;

        case LIST_DISPLAY_KEYBOARD:
			if (!KeyboardShown)
				break;

			break;
    }

	return 0;
}

//-----------------------------------------------------------------------------
// DoLButtonUp - Process WM_LBUTTONUP message for window
//
LRESULT DoLButtonUp (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

    POINT pt;
    int index;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
    ItemEdit = false;
	ItemBack = false;
	RedrawList(hWnd);
    
    int minScroll = 0;
    int maxScroll = ListHeight > rListHeight ? ListHeight - rListHeight : 0;

    switch (nListDisplayType) {
        case LIST_DISPLAY_LIST:
            if (bScrolling || bDragging 
                || Scrolled < minScroll 
                || Scrolled > maxScroll) {

                tEndTime = GetTickCount();
			    Speed = (pt.y - StartY) * SPEED_MULTIPLIER 
                    / (tEndTime - tStartTime);

                if (Scrolled < minScroll) {
                    ScrollTo(hWnd, 1, 300);
                }

                else if (Scrolled > maxScroll) {
                    ScrollTo(hWnd, maxScroll - 1, 300);
                }
                
			    else if (fabs(Speed) > 6) {
				    SetTimer(hWnd, IDT_TIMER_SCROLL, 
                        REFRESH_RATE, (TIMERPROC) NULL);
				    bScrolling = true;
			    } 
                else {
				    bScrolling = false;
				    bDragging = false;
			    }
		    } 

            else {
                int pos = pt.y + Scrolled - rList.top;
                ListItemSelected = GetPixelToItem(pos);

                if (pListData->PopulateDetailsFor(ListItemSelected))
                    ExpandDetails(hWnd);
		    }

			ListItemHover = -1;
		    RedrawList(hWnd);
	        break;

        case LIST_DISPLAY_DETAILS:
            Plus_Down = false;

            // They clicked in the bottom menus
			if (PtInRect(&rMenubar, pt)) {	
				break;
			}

		    if (pt.y > rList.top && pt.y < rList.top + DEFAULT_ITEM_HEIGHT) {
			    if (pt.x <= 40) {
				    LastListItemSelected = -1;
				    ExpandDetails(hWnd);
                }

                else if (pt.x >= rList.right - 40) {
				    pListData->ToggleFavorite(ListItemSelected);
                    dItem = pListData->GetItem(ListItemSelected);
                }

                else {
                    pListData->DisplayItem(ListItemSelected);
                    pListData->PopulateDetailsFor(ListItemSelected);
                    dItem = pListData->GetItem(ListItemSelected);
			    } 
		    }
            else {
			    // HANDLE SUBLIST EVENTS
                int subListIndex = (pt.y - rList.top) / DEFAULT_ITEM_HEIGHT - 1;
			    if (subListIndex == pListData->GetSubListCurrentActionIndex()) {
                    DataDetail sla = pListData->GetSubListCurrentAction();

                    // That sublist item might not have an action
                    if (!sla.action)
                        break;

				    switch (sla.action) {
					    case SLA_CALL:
                            if (pListData->CanSms()) SendSMS(sla.text);
                            else Call(sla.text, sla.text);
						    break;
					    case SLA_EMAIL:
                            SendEMail(pSettings->email_account, sla.text);
						    break;
				    }
				    LastListItemSelected = -1;
				    ExpandDetails(hWnd);
                    if (pSettings->doExitOnAction)
                        DestroyWindow(hWnd);
			    }
		    }
		    pListData->SetSubListCurrentActionIndex(-1);
		    pListData->SetSms(false);
		    RedrawList(hWnd);
	        break;

        case LIST_DISPLAY_KEYBOARD:
            if (!KeyboardShown)
                break;
    		
            SwitchKeyboard(hWnd, 3, 100);

            index = (pt.y / GroupHeight) 
                * (rScreen.right - rScreen.left) / GroupWidth 
                + (pt.x - rScreen.left) / GroupWidth;
            if (index < nKeyboardLetters)
                ScrollTo(hWnd, GroupPosition[index], 200);

            break;

        default:
            break;
    }

	return 0;
}

//-----------------------------------------------------------------------------
// DoTimer - Process WM_TIMER message for window
//
LRESULT DoTimer (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

	switch (wParam)	{

        ///// TIMER for scrolling
	    case IDT_TIMER_SCROLL:
		    Offset = 0;
		    if (fabs(Speed) <= 0.01) {
			    KillTimer(hWnd, IDT_TIMER_SCROLL);
			    bScrolling = false;
			    Speed = 0;
		    }
            else {
			    bScrolling = true;
			    Speed = Speed - Speed / FRICTION;
			    MoveList(hWnd, Speed);
		    }
		    break;

	    ///// TIMER for bouncing
	    case IDT_TIMER_BOUNCE:
	        if ( fabs(Offset) <= 0.01 ) {
		        KillTimer(hWnd, IDT_TIMER_BOUNCE);
		        Offset = 0;
	        }
            else {
		        Offset = Offset - Offset / FRICTION * 4;
	        }
            break;

	    ///// TIMER for scroll to
	    case IDT_TIMER_SCROLL_TO:
		    Offset = 0;
		    KillTimer(hWnd, IDT_TIMER_SCROLL);
		    if (Scroll_TimeCounter < Scroll_Duration) {
			    double amount;
			    bScrolling = true;

			    // Cubic
			    amount = Scroll_Change
                    * (pow(Scroll_TimeCounter/Scroll_Duration - 1, 3) + 1)
                    + Scroll_StartPosition;
			    Speed = amount;
			    MoveList(hWnd, Scrolled - amount);
			    Scroll_TimeCounter++;
		    }
		    else {
			    bScrolling = false;
			    Speed = 0;
			    KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
		    }
		    break;

        case IDT_TIMER_TRANSITION:
            dTransitionPct += dTransitionStep;
            if (dTransitionPct >= 1.0) {
                dTransitionPct = 1.0;
                bTransitioning = false;

                nListDisplayType = 
                    nTransitionType == T_SLIDE_RIGHT 
                    ? LIST_DISPLAY_LIST
                    : LIST_DISPLAY_DETAILS;

                if (nListDisplayType == LIST_DISPLAY_LIST)
                    ListItemSelected = -1;

                KillTimer(hWnd, IDT_TIMER_TRANSITION);
            }
            break;

	    ///// TIMER for keyboard show and hide
	    case IDT_TIMER_KEYBOARD_SWITCH:
		    ListItemSelected = -1;
		    if (Keyboard_TimeCounter < Keyboard_Duration) {
			    // Cubic
			    Keyboard_Ratio = (int)(Keyboard_Change 
                    * (pow(Keyboard_TimeCounter/Keyboard_Duration - 1, 3) + 1)
                    + Keyboard_Start);
			    Keyboard_TimeCounter++;
		    }
		    else {
			    KillTimer(hWnd, IDT_TIMER_KEYBOARD_SWITCH);
			    KeyboardShown = true;
			    if (Keyboard_Ratio <= 11) {
				    nListDisplayType = LIST_DISPLAY_LIST;
				    ListItemSelected = LastListItemSelected;
				    KeyboardShown = false;
			    }
		    }
            InvalidateRect(hWnd, &rScreen, false);
            break;
	} 

    RedrawList(hWnd);
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

	switch (nListDisplayType) {

		case LIST_DISPLAY_LIST:
            if (bScrolling) {
                KillTimer(hWnd, IDT_TIMER_SCROLL);
			    bScrolling = false;
			    Speed = 0;
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
					if (ListItemSelected < 0) {
						ListItemSelected = GetPixelToItem(Scrolled + rListHeight);
					}
                    else if (bRepeating) {
                        // Jump to the previous group
                        int min = 0;
                        if (ListItemSelected == min)
                            break;

                        ListItemSelected--;
                        while (ListItemSelected > min && 
                            !pListData->IsItemNewGroup(ListItemSelected))
                            ListItemSelected--;

                        Scrolled = max(0, min(StartPosition[ListItemSelected], 
                            ListHeight - rListHeight));

                        break;
                    }
                    else {
						ListItemSelected = max(0, ListItemSelected - 1);
					}

					// make sure the selected item is visible
					top = StartPosition[ListItemSelected];
					bot = StartPosition[ListItemSelected + 1];

					if (!bScrolling && (top < Scrolled 
                        || bot > Scrolled + rListHeight)) {

                        Scrolled = max(0, bot - rListHeight);
                    }
    
					break;

			    case VK_DOWN:
                    if (ListItemSelected < 0) {
						ListItemSelected = GetPixelToItem(Scrolled);
					}
                    else if (bRepeating) {
                        // Jump to the next group
                        int max = pListData->GetItemCount() - 1;
                        if (ListItemSelected == max)
                            break;

                        ListItemSelected++;
                        while (ListItemSelected < max && 
                            !pListData->IsItemNewGroup(ListItemSelected))
                            ListItemSelected++;

                        Scrolled = max(0, min(StartPosition[ListItemSelected], 
                            ListHeight - rListHeight));

                        break;
                    }
					else {
						ListItemSelected = min(pListData->GetItemCount() - 1, 
							ListItemSelected + 1);
					}

					// make sure the selected item is visible
					top = StartPosition[ListItemSelected];
					bot = StartPosition[ListItemSelected + 1];

					if (!bScrolling && (top < Scrolled 
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
                    break;

			    case VK_TACTION:
					if (ListItemSelected == -1)
						break;
					LastListItemSelected = ListItemSelected;
					pListData->PopulateDetailsFor(LastListItemSelected);
					ExpandDetails(hWnd);
					break;

			}
			break;


		case LIST_DISPLAY_DETAILS:
			switch (wParam) {
                case VK_BACK:
				case VK_LEFT:
					LastListItemSelected = ListItemSelected;
					pListData->SetSubListCurrentActionIndex(-1);
					ExpandDetails(hWnd);
					break;

				case VK_RIGHT:
					if (pListData->GetSubListCurrentActionIndex() > -1 
						&& pListData->GetSubListCurrentAction().canSms) {

						SendSMS(pListData->GetSubListCurrentAction().text);
						LastListItemSelected = -1;
						ExpandDetails(hWnd);
						nListDisplayType = LIST_DISPLAY_LIST;
						break;
					}
					break;

				case VK_UP:
					pListData->SelectPreviousSubListAction();
					break;

				case VK_DOWN:
			        pListData->SelectNextSubListAction();
					break;

			    case VK_TACTION:
					if (pListData->GetSubListCurrentActionIndex() > -1) {
						DataDetail sla = pListData->GetSubListCurrentAction();
						switch (sla.action) {
							case SLA_CALL:
								Call(sla.text, sla.text);
							case SLA_EMAIL:
								SendEMail(pSettings->email_account, sla.text);
						}
						LastListItemSelected = -1;
						pListData->SetSubListCurrentActionIndex(-1);
					}
					else {
						LastListItemSelected = ListItemSelected;
						pListData->PopulateDetailsFor(LastListItemSelected);
					}
					ExpandDetails(hWnd);
					break;

			}
			break;
	}
    RedrawList(hWnd);

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
// Miscellaneous Drawing Functions
//
void DrawListOn(HDC hdc, HDC hdcTmp, RECT rList) {
	int sItem, Item;
    TCHAR buffer[10];
    int count = pListData->GetItemCount();

    sItem = Item = Scrolled < 0 ? 0 : GetPixelToItem(Scrolled + (int)Offset);

    RECT rItem;
    rItem = rList;

    rItem.bottom = rList.top + StartPosition[Item] - Scrolled + (int)Offset;

	// ******* DRAW LIST BACKGROUND
	FillRect(hdc, &rList, pSettings->hbrListBackground);
	SetBkMode(hdc, TRANSPARENT);

    // "About" at bottom of screen
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_LEFT);
    SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
    ExtTextOut(hdc, rList.left + 2, rList.bottom - 15, 
        NULL, NULL, SZ_ABOUT, ABOUT_LENGTH, 0);

    // ******* DRAW LIST ITEMS
	while (Item < count && rItem.bottom < rList.bottom) {
		dItem = pListData->GetItem(Item);

        rItem.top = rItem.bottom;
        rItem.bottom = rItem.top + StartPosition[Item+1] - StartPosition[Item];
        
        if (Item == ListItemSelected && nListDisplayType == LIST_DISPLAY_LIST) {
            DrawItemSelectedOn(hdc, dItem, rItem);
		}
        else if (Item == ListItemHover) {
            DrawItemHoverOn(hdc, dItem, rItem);
		}
        else {
            DrawItemOn(hdc, dItem, rItem);
		}

        // ****** Group Header
		if ((nListDisplayType == LIST_DISPLAY_LIST || nListDisplayType == LIST_DISPLAY_KEYBOARD)
			&& pListData->IsItemNewGroup(Item)
            && dItem.wcGroup && rItem.top >= rList.top) {

            DrawGroupHeaderOn(hdc, Item, rItem);
		} 

		// Next Item
		Item++;
	}

    // Special: Draw the group of the list item that's at the top of the list
    if (nListDisplayType != LIST_DISPLAY_DETAILS && Scrolled >= 0) {
        dItem = pListData->GetItem(sItem);
        if (dItem.wcGroup) {
            RECT rTopGroup = {rList.left, 0, rList.right, DEFAULT_GROUP_HEIGHT};
            DrawGroupHeaderOn(hdcTmp, sItem, rTopGroup);

            int nHeight = DEFAULT_GROUP_HEIGHT;
            int nBottom = nHeight;

            if (pListData->IsItemNewGroup(sItem + 1)) {
                nBottom = min(nBottom, StartPosition[sItem + 1] - Scrolled + (int)Offset);
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
    }

	// Draw list indicator if scrolling quickly
	if (fabs(Speed) > 10) {
		SelectObject(hdc, ListIndicatorFont);
	    SetTextAlign(hdc, TA_CENTER);
		SetTextColor(hdc, pSettings->rgbListIndicatorText);
		SetBkMode(hdc, TRANSPARENT);

        pListData->GetItemGroup(sItem, buffer);
        int length = _tcslen(buffer);

        ExtTextOut(hdc, (rList.right - rList.left) / 2 + rList.left, 
            rList.top + 10, NULL, NULL, buffer, length, 0);
	}
}

void DrawGroupHeaderOn(HDC hdc, int index, RECT rItem) {
    RECT rHeader = rItem;
    rHeader.bottom = rHeader.top + DEFAULT_GROUP_HEIGHT;
    TCHAR buffer[10];

    pListData->GetItemGroup(index, buffer);
    int length = _tcslen(buffer);

	// ****** GroupHeader background
	FillRect(hdc, &rHeader, pSettings->hbrListGroupBackground);

    // separator
    RECT rSep = rHeader;
    rSep.top = rHeader.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

	// ******* Draw Group Header Text
	SelectObject(hdc, GroupFont);
   	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, pSettings->rgbListGroupText);
	ExtTextOut(hdc, rItem.left + LIST_GROUP_ITEM_INDENT, 
        rHeader.top - 1 + ((DEFAULT_GROUP_HEIGHT - GROUP_ITEM_FONT_SIZE) / 2),
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
	SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

    // Item Text
	SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT, 
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);
}

void DrawItemHoverOn(HDC hdc, Data dItem, RECT rItem) {
    // Item Background
	DrawGradientGDI(hdc, rItem, 
        pSettings->rgbListItemHoverBackground1,
        pSettings->rgbListItemHoverBackground2);

    // separator
    RECT rSep = rItem;
    rSep.top = rItem.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);

	SelectObject(hdc, PrimaryListFont);
	SetTextAlign(hdc, TA_LEFT);

    // Item Shadow Text
	SetTextColor(hdc, pSettings->rgbListGroupText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

    // Item Text
    SetTextColor(hdc, pSettings->rgbListItemText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

}

void DrawItemOn(HDC hdc, Data dItem, RECT rItem) {
    // Item Background
    FillRect(hdc, &rItem, pSettings->hbrListItemBackground);

    // separator
    RECT rSep = rItem;
    rSep.top = rItem.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

    // Item Primary Text
	SelectObject(hdc, PrimaryListFont);
	SetTextColor(hdc, pSettings->rgbListItemText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

	// Item Secondary Text
    if (dItem.nSecondaryTextLength == 0)
        return;
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_RIGHT);
    SetTextColor(hdc, pSettings->rgbListItemText);
    ExtTextOut(hdc, rItem.right - LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_SECONDARY_FONT_SIZE) / 2),
	    ETO_OPAQUE, NULL, dItem.szSecondaryText, dItem.nSecondaryTextLength, 0);

}

void DrawItemDetailsOn(HDC hdc, Data dItem, RECT rItem) {
	// ******* DRAW ITEM BACKGROUND
	DrawGradientGDI(hdc, rItem,
        pSettings->rgbListItemSelectedBackground1,
        pSettings->rgbListItemSelectedBackground2);

	RECT rItem2 = rItem;
	// ****** Edit contact button
	if (ItemEdit) {
		rItem2.left += 40;
        if (pListData->CanFavorite()) 
            rItem2.right -= 40;
		rItem2.bottom = rItem2.top + DEFAULT_ITEM_HEIGHT;
		DrawGradientGDI(hdc, rItem2,
            pSettings->rgbListItemHoverBackground1,
            pSettings->rgbListItemHoverBackground2);
	}

	// ****** Draw Item Text
	rItem.left += LIST_ITEM_INDENT;

    SelectObject(hdc, PrimaryListFont);
    SetTextAlign(hdc, TA_LEFT);
    SetBkMode(hdc, TRANSPARENT);

	SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
	ExtTextOut(hdc, rItem.left + 40, rItem.top + 1 + ITEM_FONT_SIZE / 2,
        0, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);
	
	SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	ExtTextOut(hdc, rItem.left + 40, rItem.top + ITEM_FONT_SIZE / 2,
        0, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);


    // DRAW ITEM DETAILS **************	
	int ButtonOffset = DEFAULT_ITEM_HEIGHT;
	RECT rButton = rItem;

    int iItemCount = pListData->GetItemDetailCount();
    int iCurrentIndex = pListData->GetSubListCurrentActionIndex();

	for (int c = 0; c < iItemCount; c++) {
        DataDetail currentAction = pListData->GetItemDetail(c);

        // draw separator
		rButton.top += ButtonOffset;
		rButton.bottom = rButton.top + 1;
		rButton.left = 0;
		FillRect(hdc, &rButton, pSettings->hbrDetailItemSeparator);

		rButton.top = rButton.bottom;
		rButton.bottom = rButton.top + DEFAULT_ITEM_HEIGHT;

		if (c == iCurrentIndex) {
			if(!pListData->CanSms() && (currentAction.action == SLA_CALL
                || currentAction.action == SLA_EMAIL)) {
				DrawGradientGDI(hdc, rButton,
                    pSettings->rgbListItemHoverBackground1,
                    pSettings->rgbListItemHoverBackground2);
            }
		}

		rButton.left = 5;

		rButton.right = rButton.left + 44;

		SelectObject(hdc, ItemDetailsFont);
		DrawText(hdc, currentAction.label, -1,
            &rButton, DT_RIGHT | DT_VCENTER);

		rButton.right = rItem.right;
		rButton.bottom = rButton.top + 20;

		SelectObject(hdc, PrimaryListFont);
		SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
		ExtTextOut(hdc, rButton.left + 52,
            rButton.top + 1 + (ITEM_FONT_SIZE) / 2,
            ETO_OPAQUE, NULL, currentAction.text,
            _tcslen(currentAction.text), 0);

		SetTextColor(hdc, pSettings->rgbListItemSelectedText);
		ExtTextOut(hdc, rButton.left + 52,
            rButton.top + (ITEM_FONT_SIZE) / 2,
            ETO_OPAQUE, NULL, currentAction.text,
            _tcslen(currentAction.text), 0);

		if (currentAction.canSms) {
			rButton.bottom = rButton.top + 40;
			if (currentAction.canSms && c == iCurrentIndex) {
				int old = rButton.left;
				rButton.left = rButton.right - 40;
				DrawGradientGDI(hdc, rButton,
                    pSettings->rgbListItemHoverBackground1,
                    pSettings->rgbListItemHoverBackground2);
				rButton.left = old;
			}
			rButton.right -= 10;
			SelectObject(hdc, ItemDetailsFont);
			DrawText(hdc, pSettings->sms_string, 3,
                &rButton, DT_RIGHT | DT_VCENTER);
			rButton.right += 10;
		}
	}

    // draw the < "Back" triangle
    // TODO: change this to a nice picture
	if (ItemBack == true) {
		rItem.left = 0;
		rItem2.right = rItem2.left + 40;
		rItem2.bottom = rItem2.top + 40;
		DrawGradientGDI(hdc, rItem2,
            pSettings->rgbListItemHoverBackground1,
            pSettings->rgbListItemHoverBackground2);
	}

	SelectObject(hdc, pSettings->hbrListItemForeground);
	SelectObject(hdc, pSettings->hpenListItemForeground);
	POINT rectPoints[3];
	rectPoints[0].x = (LONG)40 - 27;
	rectPoints[0].y = rItem.top + 20;
	rectPoints[1].x = (LONG)40 - 15;
	rectPoints[1].y = rItem.top + 13;
	rectPoints[2].x = (LONG)40 - 15;
	rectPoints[2].y = rItem.top + 27;
	Polygon(hdc, rectPoints, 3);

    // draw the + "favorite" symbol
    // TODO: change this to a nice picture
    if (pListData->CanFavorite()) {
        if (Plus_Down) {
            rItem2.top = rItem.top;
            rItem2.bottom = rItem2.top + 40;
            rItem2.right = rItem.right;
            rItem2.left = rItem2.right - 40;
		    DrawGradientGDI(hdc, rItem2,
                pSettings->rgbListItemHoverBackground1,
                pSettings->rgbListItemHoverBackground2);
        }
	    RECT plus;
	    plus.left = rItem.right - 30;
	    plus.right = plus.left + 20;
	    plus.top = rItem.top + 18;
	    plus.bottom = plus.top + 4;
	    FillRect(hdc, &plus, pSettings->hbrListItemForeground);							
	    if (!dItem.isFavorite) {
		    plus.left = rItem.right - 22;
		    plus.right = plus.left + 4;
		    plus.top = rItem.top + 10;
		    plus.bottom = plus.top + 20;
		    FillRect(hdc, &plus, pSettings->hbrListItemForeground);
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
        y = rKeyboard.top + h * (rKeyboard.bottom - rKeyboard.top) / nKeyboardRows;
        MoveToEx(hdc, rKeyboard.left, y, (LPPOINT) NULL);
        LineTo(hdc, rKeyboard.right, y);
    }
    MoveToEx(hdc, rKeyboard.left, rKeyboard.bottom - 1, (LPPOINT) NULL);
    LineTo(hdc, rKeyboard.right, rKeyboard.bottom - 1);

    // Draw the vertical lines
    for (g = 0; g < nKeyboardCols; g++) {
        x = rKeyboard.left + g * (rKeyboard.right - rKeyboard.left) / nKeyboardCols;
        MoveToEx(hdc, x, rKeyboard.top, (LPPOINT) NULL);
        LineTo(hdc, x, rKeyboard.bottom);
    }
    MoveToEx(hdc, rKeyboard.right - 1, rKeyboard.top, (LPPOINT) NULL);
    LineTo(hdc, rKeyboard.right - 1, rKeyboard.bottom);

    // Draw the letters
    int i = 0;
	for (h = 0; h <= nKeyboardRows; h++) {
        y = rKeyboard.top + ((GroupHeight - KEYBOARD_FONT_SIZE) / 2) + (GroupHeight * h);

        for (g = 0; g < nKeyboardCols; g++) {
            x = rKeyboard.left + (GroupWidth / 2) + (GroupWidth * g);

            if (i < nKeyboardLetters) {
                ExtTextOut(hdc, x, y, NULL, NULL, &alphabet[i++], 1, 0);
            }
		}
	}
}

//-----------------------------------------------------------------------------
// Utility functions
//

void RedrawList(HWND hWnd) {
	InvalidateRect(hWnd, &rList, FALSE);
	UpdateWindow(hWnd);
}

void InitSurface(HWND hWnd) {
	HDC hdc;
	hdc = GetDC(hWnd);

    // Update the RECTs for the individual sections
	GetClientRect(hWnd, &rScreen);
    nScreenHeight = rScreen.bottom - rScreen.top;
    int nScreenWidth = rScreen.right - rScreen.left;

	rTitlebar = rScreen;
	rTitlebar.bottom = rTitlebar.top + TITLE_BAR_HEIGHT;

	rMenubar = rScreen;
	rMenubar.top = rMenubar.bottom - MENU_BAR_HEIGHT;

	rList = rScreen;
	rList.top = rTitlebar.bottom;
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
    if (hbmMem)
        DeleteObject(hbmMem);
	hbmMem = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);

	if (hbmTmp)
        DeleteObject(hbmTmp);
	hbmTmp = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);

    if (hbmTmp2)
        DeleteObject(hbmTmp2);
    hbmTmp2 = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);

    // Calculate skin filename
    if (!hbmSkin) {
	    TCHAR szSkinFileName[MAX_PATH];
	    GetModuleFileName(NULL, szSkinFileName, MAX_PATH);
	    TCHAR * pstr = wcsrchr(szSkinFileName, '\\');
	    if (pstr) *(++pstr) = '\0';
	    StringCchCat(szSkinFileName, MAX_PATH, pSettings->skin_name);
        StringCchCat(szSkinFileName, MAX_PATH, TEXT(".png"));

        hbmSkin = SHLoadImageFile(szSkinFileName);

	    // Load skin
	    hdcSkin = CreateCompatibleDC(hdc);
        SelectObject(hdcSkin, hbmSkin);
    }

	DeleteDC(hdc);
}

void CalculateHeights() {
	int c = 0;
    int count = pListData->GetItemCount();

    TCHAR letter[2];
    TCHAR * pdest;
    int index;

    letter[1] = 0;

    nKeyboardLetters = _tcslen(pSettings->alphabet);
    bool bAutoAlphabet = nKeyboardLetters == 0;

    StringCchCopy(alphabet, ALPHABET_MAX_SIZE, pSettings->alphabet);

    for (int i = 0; i < ALPHABET_MAX_SIZE; i++) {
        GroupPosition[i] = -1;
        StartPosition[i] = 0;
    }

    if (count == 0) {
        ListHeight = 0;
        AverageItemHeight = DEFAULT_ITEM_HEIGHT;
        return;
    }

	for (int i = 0; i < count; i++) {
        StartPosition[i] = c;

        int h = DEFAULT_ITEM_HEIGHT;

        if (pListData->IsItemNewGroup(i) && pListData->GetItem(i).wcGroup) {
            h += DEFAULT_GROUP_HEIGHT;

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
	ListHeight = c;
    AverageItemHeight = ListHeight / count;
}

int GetPixelToItem(int y) {
    y = min(ListHeight - 1, y);
    y = max(0, y);

    // estimate based on DEFAULT_ITEM_HEIGHT
    int guess = y / AverageItemHeight;
    int max = pListData->GetItemCount();
    if (guess > max)
        guess = max;

    while (y < StartPosition[guess] && guess > 0) {guess--;}

    while (y > StartPosition[guess+1] && guess < max) {guess++;}

    return guess;
}

void MoveList(HWND hWnd, double Amount) {
	//if (ListHeight < rList.bottom - rList.top)
	//	return;

    int tmpScrolled = Scrolled - (int)Amount;
    int minScrolled = 0;
    int maxScrolled = ListHeight <= rListHeight ? 0 : ListHeight - rListHeight;
    bool bounce = false;

    if (bScrolling && Scrolled > minScrolled && tmpScrolled <= minScrolled) {
        bounce = true;
        tmpScrolled = minScrolled;
    }

    else if (bScrolling && Scrolled < maxScrolled && tmpScrolled >= maxScrolled) {
        bounce = true;
	    tmpScrolled = maxScrolled;
    }

    if (bounce) {
	    SetTimer(hWnd, IDT_TIMER_BOUNCE, REFRESH_RATE, NULL);
	    Offset = Amount;
	    bScrolling = false;
	    KillTimer(hWnd, IDT_TIMER_SCROLL);
	    KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
	    Speed = 0;
    }

    Scrolled = tmpScrolled;
	RedrawList(hWnd);
}

void ScrollBar(int y) {
	bScrolling = true;
	Speed = 20;
    int nListMax = ListHeight - rListHeight;

    // if "Contacts", scroll by chunks of A,B,C,etc.
    if (nCurrentTab == 2) {
        int index = (y - rList.top) * nKeyboardLetters / rListHeight;
        Scrolled = index < 0 ? 0
            : index >= nKeyboardLetters ? nListMax
            : GroupPosition[index];
    }

    // otherwise, just do a normal scroll
    else {
        double pct = (double)(y - rList.top) 
            / (double)rListHeight;
	    Scrolled = (int)(nListMax * pct);
    }

    Scrolled = min(Scrolled, nListMax);
    Scrolled = max(Scrolled, 0);
}

void ScrollTo(HWND hWnd, int position, int duration) {
	Scroll_StartPosition = Scrolled;
	Scroll_Change = position - Scroll_StartPosition;
	Scroll_Duration = duration / REFRESH_RATE;
	Scroll_TimeCounter = 0;
	SetTimer(hWnd, IDT_TIMER_SCROLL_TO, REFRESH_RATE, NULL);
}

void StopScrolling(HWND hWnd) {
    KillTimer(hWnd, IDT_TIMER_SCROLL);
    KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
    Speed = 0;
    bScrolling = false;
}

void ExpandDetails(HWND hWnd) {
    bTransitioning = true;

    dTransitionPct = 0.0;
    dTransitionStep = 0.3;
    nTransitionType = 
        nListDisplayType == LIST_DISPLAY_LIST 
        ? T_SLIDE_LEFT
        : T_SLIDE_RIGHT;
    
    RedrawList(hWnd);
	SetTimer(hWnd, IDT_TIMER_TRANSITION, REFRESH_RATE, NULL);
}

void SwitchKeyboard(HWND hWnd, int action, int duration) {
	if (action == 1) {
		LastListItemSelected = ListItemSelected;

		Keyboard_Start = 10;
		Keyboard_Change = 91;
		Keyboard_Duration = duration / REFRESH_RATE;
		Keyboard_TimeCounter = 0;
	}

	else {
		Keyboard_Start = 101;
		Keyboard_Change = -91;
		Keyboard_Duration = duration / REFRESH_RATE;
		Keyboard_TimeCounter = 0;
	}
	SetTimer(hWnd, IDT_TIMER_KEYBOARD_SWITCH, REFRESH_RATE, NULL);
}

void SwitchTab(HWND hWnd, int which) {
    if (pListData) {
        pListData->Clear();
        delete pListData;
    }
    nListDisplayType = LIST_DISPLAY_LIST;
    nCurrentTab = which;
    Scrolled = 0;
    ListItemSelected = -1;
    InvalidateRect(hWnd, &rMenubar, false);

    pListData = 
        // Favorites
        nCurrentTab == 0 ? (ListData *)new ListDataPoom(pSettings, true)

        // Call Log
        : nCurrentTab == 1 ? (ListData *)new ListDataCallLog(pSettings)

        // Contacts
        : (ListData *)new ListDataPoom(pSettings);

    CalculateHeights();
}