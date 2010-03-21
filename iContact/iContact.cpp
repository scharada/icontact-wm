// iContact.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "iContact.h"
#include "ContactList.h"
#include "GraphicFunctions.h"
#include "poom.h"
#include <windows.h>
#include <commctrl.h>
#include <pimstore.h>
#include <string.h>
#include <cctype>


#define MAX_LOADSTRING 100
#define IDT_TIMER_WINDOW_MOVE	10020
#define IDT_TIMER_SIP_MOVE		10030

// Global Variables:
HINSTANCE			g_hInst;			// current instance
RECT				rApp;
RECT				rApp2;
RECT				rContactList;

HFONT hFnt;

HWND				hList;
HWND				hWnd;

ATOM				MyRegisterClass(HINSTANCE, LPTSTR);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL AddContactsFolderText(IPOutlookItemCollection * pItemCol);
bool PopulateList();

IFolder * pCurrFldr3 = NULL;
IPOutlookItemCollection * pItemCol3;

COLORREF	MainBackgroundColor = RGB(100,100,100);
COLORREF	MainBackgroundColor2 = RGB(30,30,30);

double scroll = 320;

int restoreSipLeft = 0;
int restoreSipTop = 0;
int SipCounter = 10;


int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPTSTR    lpCmdLine,
				   int       nCmdShow)
{
	MSG msg;

	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ICONTACT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_PARENTDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	TCHAR szTitle[MAX_LOADSTRING];		// title bar text
	TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

	scroll = 320;

	g_hInst = hInstance; // Store instance handle in our global variable

	SHInitExtraControls();

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
	LoadString(hInstance, IDC_ICONTACT, szWindowClass, MAX_LOADSTRING);

	hWnd = FindWindow(szWindowClass, szTitle);

	if (hWnd) 
	{
		SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
		return 0;
	} 

	if (!MyRegisterClass(hInstance, szWindowClass))
	{
		return FALSE;
	}

	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	SetWindowText(hWnd, TEXT("iContact"));

	if (!hWnd)
	{
		return FALSE;
	}

	MoveWindow(hWnd, 0, (int)scroll, 240, rApp.bottom - rApp.top, false);
	ShowWindow(hWnd, nCmdShow);
	InvalidateRect(hWnd, &rApp, TRUE);
	UpdateWindow(hWnd);

	hFnt = BuildFont(15, TRUE, FALSE);

	SetTimer(hWnd, IDT_TIMER_WINDOW_MOVE, 12, NULL);
	//SetTimer(hWnd, IDT_TIMER_SIP_MOVE, 400, NULL);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//PAINTSTRUCT ps;
	//HDC hdc;

	static SHACTIVATEINFO s_sai;

	switch (message) 
	{
	case WM_CREATE:
		{
			// Initialize the shell activate info structure
			memset(&s_sai, 0, sizeof (s_sai));
			s_sai.cbSize = sizeof (s_sai);

			InitPoom(hWnd);

			GetClientRect(hWnd, &rApp);
			rContactList = rApp;
			//rContactList.top = 30;

			hList = CreateContactList(hWnd, rContactList, PopulateList);
			SetFocus(hList);
			//PopulateList();
			//CalculateHeights();
		}
		break;

	case WM_PAINT:
		{
			/*
			hdc = BeginPaint(hWnd, &ps);

			RECT rPaint;
			rPaint.left = 0; rPaint.top = 0; rPaint.right = 240; rPaint.bottom = 30;
			DrawGradientGDI(hdc, rPaint, MainBackgroundColor, MainBackgroundColor2);

			TCHAR buf[15];
			wsprintf(buf, _T("All contacts"));

			SelectObject(hdc, hFnt);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(255,255,255));
			rPaint.left = 10;
			DrawText(hdc, buf, _tcslen(buf), &rPaint , DT_LEFT| DT_WORDBREAK | DT_VCENTER | DT_NOCLIP);

			EndPaint(hWnd, &ps);
			*/
		}
		break;

	case WM_TIMER:
		{
			switch (wParam)
			{
				///// TIMER for scrolling
			case IDT_TIMER_WINDOW_MOVE:
				{
					if (scroll > 320 - rApp.bottom)
					{
						scroll = scroll * 0.65;
						MoveWindow(hWnd, rApp.left, (int)scroll, rApp.right - rApp.left, rApp.bottom - rApp.top, TRUE);
					}
					else
					{
						scroll = 320 - rApp.bottom;
						MoveWindow(hWnd, rApp.left, (int)scroll, rApp.right - rApp.left, rApp.bottom - rApp.top, TRUE);
						KillTimer(hWnd, IDT_TIMER_WINDOW_MOVE);
					}
				}
				break;

			case IDT_TIMER_SIP_MOVE:
				MoveSIP(false, 0);
				KillTimer(hWnd, IDT_TIMER_SIP_MOVE);
				break;

			}
		}
		break;

	case WM_DESTROY:
		pItemCol3->Release();
		pCurrFldr3->Release();
		DestroyList();
		ShutdownPoom();
		//MoveSIP(true, 10);
		PostQuitMessage(0);
		break;

	case WM_ACTIVATE:
		SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
		switch (LOWORD(wParam))
		{
		case WA_ACTIVE || WA_CLICKACTIVE:

			ClearList();
			PopulateList();
			CalculateHeights();

			UpdateWindow(hWnd);
			RedrawList();
			SHFullScreen(hWnd, SHFS_HIDESIPBUTTON); 
			break;

		case WA_INACTIVE:
			//SHFullScreen(hWnd, SHFS_SHOWSIPBUTTON);
			break;
		}
		break;

	case WM_SETTINGCHANGE:
		GetClientRect(hWnd, &rApp);
		rContactList = rApp;
		ResizeList(rContactList);
		SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool PopulateList()
{

	GetPoomFolder(olFolderContacts, &pCurrFldr3);
	pCurrFldr3->get_Items(&pItemCol3);

	AddContactsFolderText(pItemCol3);

	return true;
}

BOOL AddContactsFolderText(IPOutlookItemCollection * pItemCol)
{
	BSTR bstrContactInfo;
	BSTR bstrFileAs;

	WCHAR * grpBuf;
	grpBuf = new WCHAR[1];

	int groupID = 3800;

	IContact * pContact = NULL;
	int cItems = 0;

	pItemCol->get_Count(&cItems);
	
	WCHAR * newItem;
	newItem = new WCHAR[30];
	wsprintf(newItem, TEXT("Add new contact"));
	AddItem(10000, newItem, false, 40, 0, -1);
	for (int i = 1; i <= cItems; i++)
	{
		if (SUCCEEDED(pItemCol->Item (i, reinterpret_cast<IDispatch**>(&pContact))))
		{
			LONG lOid;
			// grab properties
			pContact->get_FileAs(&bstrFileAs);
			pContact->get_Oid(&lOid);

			// allocate a buffer for all the properties plus a comma, space. newline, and terminating null
			bstrContactInfo = SysAllocStringByteLen(NULL, SysStringByteLen(bstrFileAs) + (4*sizeof(OLECHAR)));
			_tcscpy(bstrContactInfo, bstrFileAs);

			WCHAR * buf;
			buf = new WCHAR[90];
			wcscpy(buf, bstrContactInfo);

			if (toupper(grpBuf[0]) != toupper(buf[0]))
			{
				buf[0] = toupper(buf[0]);
				groupID++;
				WCHAR * grp;
				grp = new WCHAR[1];
				wsprintf(grp, TEXT("%c"), buf[0]);
				wsprintf(grpBuf, TEXT("%c"), buf[0]);
				AddItem(groupID, grp, true, 17, -1, -1);
			}
			
			AddItem(i-1, buf, false, 42, groupID, lOid);

			// clean up
			SysFreeString(bstrFileAs);
			SysFreeString(bstrContactInfo);
			pContact->Release();
		}
	}

	return TRUE;
}

void MoveSIP(bool restore, int screenWidth)
{
	HWND hSip;
	RECT rect;
	if (restoreSipLeft == 0)
	{
		hSip = FindWindow(TEXT("MS_SIPBUTTON"), TEXT("MS_SIPBUTTON"));
		GetWindowRect(hSip, &rect);
		restoreSipLeft = rect.left;
		restoreSipTop = rect.top;
	}
	if (restore)
	{
		SetWindowPos(hSip, NULL, restoreSipLeft, 295, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos(hSip, NULL, screenWidth + 20, rect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW);
	}
}