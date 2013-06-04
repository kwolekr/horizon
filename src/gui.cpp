/*-
 * Copyright (c) 2007 Ryan Kwolek 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this list of
 *     conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list
 *     of conditions and the following disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "optimize.h"
#include "main.h"
#include "resource.h"
#include "hashtable.h"
#include "fxns.h"
#include "clan.h"
#include "gui.h"
//#include <comctl32.h>
//#include <wingdi.h>

#define SPLASH_TEXT "  dP                         oo                             \r\n" \
                    "  88                                                        \r\n" \
                    "  88d888b. .d8888b. 88d888b. dP d888888b .d8888b. 88d888b.  \r\n" \
                    "  88'  `88 88'  `88 88'  `88 88    .d8P' 88'  `88 88'  `88  \r\n" \
                    "  88    88 88.  .88 88       88  .Y8P    88.  .88 88    88  \r\n" \
                    "  dP    dP `88888P' dP       dP d888888P `88888P' dP    dP  \r\n" \
                    "  Horizon " HORIZON_VERSION " - by BreW                     \r\n" \
                    "                                                            "  

COLORREF gradclr1, gradclr2;

HICON bnicons[NUM_BNICONS], stateicons[3], hTooltipTmp;

HIMAGELIST hCfgTabiml;

HDC wallmemdc;

HBITMAP hBmpShade;
HGDIOBJ hWallMemDCOld;

char pszTooltipTitle[64];
char pszTooltipText[256];

char *columntxts[] = {
	"User",
	"Ping",
	"Flags",
	"Clan"
};

int LogPixelsY;

#define NUM_MENUITEMS 73

MENUITEM menus[] = {
	//main menu bar
	{&hMenu_main, MF_STRING | MF_POPUP, 0, "Bot"},
	{&hMenu_main, MF_STRING, 10000, "Config"},
	{&hMenu_main, MF_STRING, 10001, "Profiles"},
	{&hMenu_main, MF_STRING | MF_POPUP, 0, "Windows"},
	{&hMenu_main, MF_STRING | MF_POPUP, 0, "Open"},
	{&hMenu_main, MF_STRING, 10002, "Favorites"},
	{&hMenu_main, MF_STRING, 10003, "About"},		  //7

	//bot popup
	{&hMenu_main_popup, MF_SEPARATOR, 0, NULL},
	{&hMenu_main_popup, MF_STRING, 10010, "Exit"},		//9

	//windows popup
	{&hMenu_windows_popup, MF_STRING, 10020, "Tile Vertically"}, 
	{&hMenu_windows_popup, MF_STRING, 10021, "Tile Horizontally"},
	{&hMenu_windows_popup, MF_STRING, 10022, "Cascade"},
	{&hMenu_windows_popup, MF_SEPARATOR, 0, NULL},		//13			 

	//open popup
	{&hMenu_open_popup, MF_STRING, 10030, "Folder"},
	{&hMenu_open_popup, MF_SEPARATOR, 0, NULL},
	{&hMenu_open_popup, MF_STRING, 10031, "config.ini"},
	{&hMenu_open_popup, MF_STRING, 10032, "CDKeys.txt"},
	{&hMenu_open_popup, MF_STRING, 10033, "Proxies.txt"},
	{&hMenu_open_popup, MF_STRING, 10034, "Servers.txt"},
	{&hMenu_open_popup, MF_SEPARATOR, 0, NULL},
	{&hMenu_open_popup, MF_STRING, 10035, "notepad.txt"},
	{&hMenu_open_popup, MF_STRING, 10036, "Help.txt"},	  //22

	//tab
	//{&hMenu_tab_popup, MF_STRING, 10040, "dfghf"},
	//{&hMenu_tab_popup, MF_STRING, 10041, "asdf"},
	//{&hMenu_tab_popup, MF_SEPARATOR, 0, NULL},
	//{&hMenu_tab_popup, MF_STRING, 10042, "Exit"},

	//rtb
	{&hMenu_rtf, MF_STRING, 10050, "Copy"},
	{&hMenu_rtf, MF_STRING, 10051, "Select All"},
	{&hMenu_rtf, MF_SEPARATOR, 0, NULL},
	{&hMenu_rtf, MF_STRING, 10052, "Notepad"},
	{&hMenu_rtf, MF_SEPARATOR, 0, NULL},
	{&hMenu_rtf, MF_STRING | MF_POPUP, 0, "Chat modifiers"},	//28th
	{&hMenu_rtf, MF_SEPARATOR, 0, NULL},
	{&hMenu_rtf, MF_STRING, 10054, "About Horizon"},	

	//lvw
	{&hMenu_lvw, MF_STRING, 10060, "Profile"},
	{&hMenu_lvw, MF_STRING, 10061, "Whisper"},
	{&hMenu_lvw, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvw, MF_STRING, 10062, "Kick"},
	{&hMenu_lvw, MF_STRING, 10063, "Ban"},
	{&hMenu_lvw, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvw, MF_STRING, 10064, "Squelch"},
	{&hMenu_lvw, MF_STRING, 10065, "Unsquelch"},
	{&hMenu_lvw, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvw, MF_STRING, 10066, "WC3 Clan Invite"},
	{&hMenu_lvw, MF_STRING, 10067, "WC3 Web Profile"},		 

	//client window
	{&hMenu_client, MF_STRING, 10070, "Set Solid Color"},
	{&hMenu_client, MF_STRING, 10071, "Choose background..."},	  
	
	//lvw columns
	{&hMenu_lvwColumns, MF_STRING, 10080, "Show columns"},
	{&hMenu_lvwColumns, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvwColumns, MF_STRING, 10081, "Username"},
	{&hMenu_lvwColumns, MF_STRING, 10082, "Ping"},
	{&hMenu_lvwColumns, MF_STRING, 10083, "Flags"},
	{&hMenu_lvwColumns, MF_STRING, 10084, "Clan"},
	
	{&hMenu_lvwBots, MF_STRING, 10090, "Unload"},
	{&hMenu_lvwBots, MF_STRING, 10091, "Reload settings"},
	{&hMenu_lvwBots, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvwBots, MF_STRING, 10092, "Connect"},
	{&hMenu_lvwBots, MF_STRING, 10093, "Reconnect"},
	{&hMenu_lvwBots, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvwBots, MF_STRING, 10094, "Configure"},


	{&hMenu_txtExt, MF_STRING, 10040, "aLt CaPs"},
	{&hMenu_txtExt, MF_STRING, 10041, "13375p33k"},
	{&hMenu_txtExt, MF_SEPARATOR, 0, NULL},
	{&hMenu_txtExt, MF_STRING, 10042, "Hex"},
	{&hMenu_txtExt, MF_STRING, 10043, "XRCrypt"},
	{&hMenu_txtExt, MF_SEPARATOR, 0, NULL},
	{&hMenu_txtExt, MF_STRING, 10044, "Convert tabs to spaces"},
	
	{&hMenu_lvwClan, MF_STRING, 10200, "Member Information"},
	{&hMenu_lvwClan, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvwClan, MF_STRING, 10201, "Make Chieftain"},
	{&hMenu_lvwClan, MF_STRING, 10202, "Make Shaman"},
	{&hMenu_lvwClan, MF_STRING, 10203, "Make Grunt"},
	{&hMenu_lvwClan, MF_STRING, 10204, "Make Peon"},
	{&hMenu_lvwClan, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvwClan, MF_STRING, 10205, "Remove Member"},
	{&hMenu_lvwClan, MF_SEPARATOR, 0, NULL},
	{&hMenu_lvwClan, MF_STRING, 10206, "Create New Clan"}
};


///////////////////////////////////////////////////////////////////////////////


void InitalizeMenus() {
	MENUITEMINFO mii;
	int i;
	hMenu_main          = CreateMenu();
	hMenu_main_popup    = CreatePopupMenu();
	hMenu_windows_popup = CreatePopupMenu();
	hMenu_open_popup    = CreatePopupMenu();
	hMenu_rtf           = CreatePopupMenu();
	hMenu_lvw           = CreatePopupMenu();
	hMenu_client        = CreatePopupMenu();
	hMenu_lvwColumns    = CreatePopupMenu();
	hMenu_lvwBots       = CreatePopupMenu();
	hMenu_txtExt        = CreatePopupMenu();
	hMenu_lvwClan       = CreatePopupMenu();
	menus[0].id         = (int)hMenu_main_popup;
	menus[3].id         = (int)hMenu_windows_popup;
	menus[4].id         = (int)hMenu_open_popup;
	menus[27].id        = (int)hMenu_txtExt;
	for (i = 0; i != NUM_MENUITEMS; i++)
		AppendMenu(*menus[i].lphMenu, menus[i].type, menus[i].id, menus[i].title);
	SetMenu(hWnd_main, hMenu_main);	
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_CHECKED;
	for (i = 10080; i != 10085; i++)
		SetMenuItemInfo(hMenu_lvwColumns, i, false, &mii);
}


void InitIML() {
	int i;
	HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32, 3, 0);
	HBITMAP hbIcons = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 48, 16, LR_DEFAULTCOLOR);
	ImageList_Add(hImageList, hbIcons, NULL);
	for (i = 0; i != 3; i++)
		stateicons[i] = ImageList_GetIcon(hImageList, i, ILD_NORMAL);
	SendMessage(hWnd_Tab, TCM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hImageList);
	
	hCfgTabiml = ImageList_Create(16, 16, ILC_COLOR32, 6, 0);
	hbIcons = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP3), IMAGE_BITMAP, 96, 16, LR_DEFAULTCOLOR);
	ImageList_AddMasked(hCfgTabiml, hbIcons, 0);
	
	/////listview images
	hImageList = ImageList_Create(28, 14, ILC_COLOR32 | ILC_MASK, 32, 0);
	ImageList_SetBkColor(hImageList, CLR_NONE);
	lvwIML = hImageList;
	hbIcons = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2), IMAGE_BITMAP, 896, 14,  /*ILC_MASK |*/ LR_DEFAULTCOLOR);
	//ImageList_Remove(hImageList, -1);

	//char asdf[32];
	//sprintf(asdf, "%d", CLR_INVALID == -1);
	//MessageBox(0,asdf,0,0);

	ImageList_AddMasked(hImageList, hbIcons, 0);
	for (i = 0; i != NUM_BNICONS; i++)
		bnicons[i] = ImageList_GetIcon(hImageList, i, ILD_NORMAL);
}


unsigned long ParseCustomDraw(HWND hWnd, LPNMLVCUSTOMDRAW lParam) {
	LPNMLVCUSTOMDRAW nmCustDraw = lParam;
	switch (nmCustDraw->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:		
			if (nmCustDraw->nmcd.uItemState & CDIS_SELECTED)				
				nmCustDraw->clrTextBk = 0x00202020;
			return CDRF_NOTIFYSUBITEMDRAW;
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			LPLVCOLOR lplvColor;
			lplvColor = (LPLVCOLOR)nmCustDraw->nmcd.lItemlParam; 
			nmCustDraw->clrText = (nmCustDraw->iSubItem != 3) ?
				*(unsigned long *)((unsigned char *)&lplvColor->cItem1 + (nmCustDraw->iSubItem << 2)) : 0xFFFFFF;
			SetBkMode(nmCustDraw->nmcd.hdc, TRANSPARENT);
			SelectObject(nmCustDraw->nmcd.hdc, lplvColor->bBold ? hFontBold : hFont);
			return CDRF_NEWFONT;
	}		  
	return CDRF_DODEFAULT;
}


void DrawGradientFill(HDC hdc, LPRECT rect, int clrfrom, int clrto) {
	TRIVERTEX vertex[2];
	GRADIENT_RECT gRect = {0, 1};

	vertex[0].x     = 0;
	vertex[0].y     = 0;
	vertex[0].Red   = (clrfrom & 0x0000FF) << 8; 
	vertex[0].Green = (clrfrom & 0x00FF00);
	vertex[0].Blue  = (clrfrom & 0xFF0000) >> 8; 
	vertex[0].Alpha = 0;

	vertex[1].x     = rect->right;
	vertex[1].y     = rect->bottom;
	vertex[1].Red   = (clrto & 0x0000FF) << 8; 
	vertex[1].Green = (clrto & 0x00FF00);
	vertex[1].Blue  = (clrto & 0xFF0000) >> 8;
	vertex[1].Alpha = 0; 

	GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}


void RegisterWindows() {
	WNDCLASS wc;
	g_hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;	
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hIcon         = g_hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szAppName;
	RegisterClass(&wc);

	wc.lpfnWndProc   = QuickSwitchProc;
	wc.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	wc.lpszClassName = "quiksw";
	RegisterClass(&wc);

	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW);
	wc.lpszMenuName  = NULL; //MAKEINTRESOURCE(IDR_MENUBOT);
	wc.lpfnWndProc   = (WNDPROC)MDIChildProc;
	wc.lpszClassName = "dsfarg";
	RegisterClass(&wc);
}


void PaintTransparentRect(HDC hdc, LPRECT paintr, COLORREF color, int alpha) {
	BLENDFUNCTION blend;
	RECT rect;

	HDC memdc    	= CreateCompatibleDC(NULL);
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 1, 1);
	HBRUSH hBrush   = CreateSolidBrush(color);
	HGDIOBJ hOld    = SelectObject(memdc, hBitmap);
	
	rect.left   = 0;
	rect.top    = 0;
	rect.bottom = 1;
	rect.right  = 1;
	
	SetPixel(memdc, 0, 0, color); 

	//HGDIOBJ hOldFont = SelectObject(hdc0, hFontBold);
	paintr->right = paintr->left + 80 + CalcRectWidth(hdc);

	blend.BlendOp			  = AC_SRC_OVER;
	blend.BlendFlags		  = 0;
	blend.SourceConstantAlpha = alpha;
	blend.AlphaFormat		  = 0;
	AlphaBlend(hdc, paintr->left, paintr->top, paintr->right - paintr->left,
		paintr->bottom - paintr->top, memdc, 0, 0, 1, 1, blend);

	SelectObject(memdc, hOld);
	DeleteObject(hBrush);
	DeleteObject(hBitmap);
	DeleteDC(memdc);
	
#if 0
	FrameRect(hdc0, paintr, hBrush);
	SetBkMode(hdc0, TRANSPARENT);
	SetTextColor(hdc0, 0xD0D0D0);
	TextOut(hdc0, paintr->left + 30, paintr->top + 7, "QuickSwitch", 11);
	SelectObject(hdc0, hFont);
	for (int i = 0; i != numbots; i++) {
		DrawIconEx(hdc0, paintr->left + 30, paintr->top + 30 + (i * 22), 
			stateicons[0], 16, 16, 0, NULL, DI_NORMAL); 
		if (curnewprof == i) {
			rect.left   = paintr->left + 28;
			rect.top    = paintr->top  + 28 + (i * 22);
			rect.right  = paintr->left + 56 + (strlen(bot[i]->profilename) * 6);
			rect.bottom = paintr->top  + 47 + (i * 22);
			FrameRect(hdc0, &rect, (HBRUSH)GetStockObject(GRAY_BRUSH));///////////
			SetTextColor(hdc0, vbWhite);
		} else {
			SetTextColor(hdc0, 0xF0A0A0);
		}
		TextOut(hdc0, paintr->left + 52, paintr->top + 30 + (i * 22),
			bot[i]->profilename, strlen(bot[i]->profilename));
	}
	SelectObject(hdc0, hOldFont);
	SelectObject(hdc, hOld);
	DeleteDC(hdc);
	ReleaseDC(0, hdc0);
	DeleteObject(hBitmap);
	DeleteObject(hBrush);
#endif
}


int CalcRectWidth(HDC hdc) {
	SIZE size;
	int bwidth = 0;
	for (int i = 0; i != numbots; i++) {
		GetTextExtentPoint(hdc, bot[i]->pname, strlen(bot[i]->pname), &size);
		if (size.cx > bwidth)
			bwidth = size.cx;
	}
	return bwidth;
}


unsigned long __stdcall SplashScreenProc(void *param) {
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	HDC hdc = GetDC(0);
	SetTextColor(hdc, 0xFFFFFF);
	SetBkColor(hdc, 0x802010);
	HFONT hFontTmp = CreateFont(20, 0, 0, 0, 0, true, 0, 0, 0, 0, 0, 0, 0, "Courier New");
	HGDIOBJ hOld = SelectObject(hdc, hFontTmp);
	int scrwidth  = GetSystemMetrics(SM_CXFULLSCREEN);
	int scrheight = GetSystemMetrics(SM_CYFULLSCREEN);
	rect.left = (scrwidth  - 600) >> 1;
	rect.top  = (scrheight - 100) >> 1;
	rect.bottom = 0;
	rect.right = 0;
	for (int i2 = 0; i2 != 8; i2++) {
		DrawText(hdc, SPLASH_TEXT, sizeof(SPLASH_TEXT) - 1, &rect, DT_NOCLIP);
		Sleep(400);
	}
	GetWindowRect(hWnd_Client, &rect);
	InvalidateRect(0, &rect, true);
	SelectObject(hdc, hOld);
	DeleteObject(hFontTmp);
	ReleaseDC(0, hdc);
	return NULL;
}


void CreateMDIChildControls(HWND hwnd, int index) {
	/////////////////////////////RTB//////////////////////////////////////
	bot[index]->hWnd_rtfChat = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_TRANSPARENT, RICHEDIT_CLASS, NULL, 
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL | WS_THICKFRAME,
		0, 0, 400, 325, hwnd, (HMENU)30, g_hInst, 0);
	SendMessage(bot[index]->hWnd_rtfChat, WM_SETFONT, (WPARAM)hFont, 0);
	lpfnMDIctlrtb = (WNDPROC)SetWindowLong(bot[index]->hWnd_rtfChat, GWL_WNDPROC, (LONG)MDIrtbProc);
	SendMessage(bot[index]->hWnd_rtfChat, EM_SETBKGNDCOLOR, 0, 0);

	if (gfstate & GFS_URLDETECT) {
		SendMessage(bot[index]->hWnd_rtfChat, EM_SETEVENTMASK, 0, ENM_LINK); 
		SendMessage(bot[index]->hWnd_rtfChat, EM_AUTOURLDETECT, true, 0);
	}

	/////////////////////////////TXT//////////////////////////////////////
	bot[index]->hWnd_txtChat = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", NULL, 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE,
		0, 326, 400, 50, hwnd, (HMENU)41, g_hInst, 0);
	SendMessage(bot[index]->hWnd_txtChat, WM_SETFONT, (WPARAM)hFont, 0);
	lpfnMDIctltxt = (WNDPROC)SetWindowLong(bot[index]->hWnd_txtChat, GWL_WNDPROC, (LONG)MDItxtProc);

	/////////////////////////////CBO///////////////////////////////////////
	/*bot[index]->hWnd_cboChat = CreateWindow("COMBOBOX", NULL,
		WS_VISIBLE | WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL, 
		0, 326, 400, 200, hwnd, (HMENU)40, g_hInst, 0);
	SendMessage(bot[index]->hWnd_cboChat, WM_SETFONT, (WPARAM)hFont, 0);
	POINT pt = {10, 10};
	bot[index]->hWnd_cbotxtChat = ChildWindowFromPoint(bot[index]->hWnd_cboChat, pt);
	lpfnMDIctlcbo = (WNDPROC)SetWindowLong(bot[index]->hWnd_cbotxtChat, GWL_WNDPROC, (LONG)MDIcboProc);
	*/												 

	/////////////////////////////TAB///////////////////////////////////////
	bot[index]->hWnd_tab = CreateWindow(WC_TABCONTROL, NULL,
		WS_VISIBLE | WS_CHILD | TCS_BOTTOM | TCS_HOTTRACK | TCS_RIGHTJUSTIFY | TCS_MULTILINE, 
		0, 0, 0, 0,	hwnd, (HMENU)55, g_hInst, 0);
	SendMessage(bot[index]->hWnd_tab, WM_SETFONT, (WPARAM)hFont, 0);
	TCITEM tci;
	tci.mask = TCIF_TEXT;
	tci.pszText = "Channel";
	TabCtrl_InsertItem(bot[index]->hWnd_tab, LVTAB_CHANNEL, &tci);
	tci.pszText = "Friends";
	TabCtrl_InsertItem(bot[index]->hWnd_tab, LVTAB_FRIENDS, &tci);
	tci.pszText = "Clan";
	TabCtrl_InsertItem(bot[index]->hWnd_tab, LVTAB_CLAN, &tci);

	/////////////////////////////LBL///////////////////////////////////////
	bot[index]->hWnd_lblChannel = CreateWindow("STATIC", NULL,
		WS_VISIBLE | WS_CHILD | SS_CENTER | SS_OWNERDRAW, 
		400, 0, 200, 20, hwnd, (HMENU)50, g_hInst, 0);
	SendMessage(bot[index]->hWnd_lblChannel, WM_SETFONT, (WPARAM)hFont, 0);
	lpfnMDIctllbl = (WNDPROC)SetWindowLong(bot[index]->hWnd_lblChannel, GWL_WNDPROC, (LONG)MDIlblProc);

	/////////////////////////////LVW///////////////////////////////////////
	bot[index]->hWnd_lvwChannel = CreateWindowEx(
		/*LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_BORDERSELECT | LVS_EX_UNDERLINEHOT/*LVS_EX_COLUMNOVERFLOW*/
		0, "SysListView32", NULL,
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER, 
		400, 20, 200, 326, hwnd, (HMENU)60, g_hInst, 0);

	SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETBKCOLOR, 0, 0);
	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hWnd_main;
	ti.uFlags = TTF_IDISHWND;// | TTF_SUBCLASS; 
	ti.uId = (UINT_PTR)bot[index]->hWnd_lvwChannel; 
	ti.lpszText = "hm";
	ti.hinst = g_hInst;
	SendMessage(hWnd_Tooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	
	//	SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETEXTENDEDLISTVIEWSTYLE,
	//		/*LVS_EX_FULLROWSELECT | */LVS_EX_INFOTIP,  LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	ListView_SetExtendedListViewStyle(bot[index]->hWnd_lvwChannel, LVS_EX_SUBITEMIMAGES);
	
	lpfnMDIctllvw = (WNDPROC)SetWindowLong(bot[index]->hWnd_lvwChannel, GWL_WNDPROC, (LONG)MDIlvwProc);
			
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)lvwIML);
	//SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETBKCOLOR, 0, 0);
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETTEXTBKCOLOR, 0, CLR_NONE);
			
	LVCOLUMNA lvcolumn;
	lvcolumn.mask = LVCF_TEXT;
	for (int i = 0; i != 4; i++) {
		lvcolumn.pszText = columntxts[i];
		SendMessage(bot[index]->hWnd_lvwChannel, LVM_INSERTCOLUMN, i, (LPARAM)&lvcolumn);
		SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETCOLUMNWIDTH, i, (LPARAM)i ? 50 : 150);
	}

	////////////////////////////////////STB/////////////////////////////////////////////////
	bot[index]->hWnd_Statusbar = CreateWindow(STATUSCLASSNAME, NULL,
		SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE, 
		0, 380, 600, 20, hwnd, (HMENU)60, g_hInst, 0);
	SendMessage(bot[index]->hWnd_Statusbar, WM_SETFONT, (WPARAM)hFont, 0);
	lpfnMDIctlstb = (WNDPROC)SetWindowLong(bot[index]->hWnd_Statusbar, GWL_WNDPROC, (LONG)MDIstbProc);
	int lpParts[NUM_SBPARTS] = {150, 325, 425, 525, 650};
	SendMessage(bot[index]->hWnd_Statusbar, SB_SETPARTS, NUM_SBPARTS, (LPARAM)lpParts);
}


#if 0
void UpdateLVDrag(HWND hwnd) {
	POINT point;
	RECT rect;
	int index = GetWindowLong(hwnd, GWL_USERDATA);
	GetCursorPos(&point);
	GetWindowRect(hwnd, &rect);
	bot[index]->lvsize = rect.right - point.x + 10;
	MoveWindow(bot[index]->hWnd_lvwChannel, point.x - rect.left - 10, 20, bot[index]->lvsize, rect.bottom - rect.top - 70, true);
	MoveWindow(bot[index]->hWnd_lblChannel, point.x - rect.left - 10, 0, bot[index]->lvsize, 20, true);
	MoveWindow(bot[index]->hWnd_rtfChat, 0, 0, point.x - rect.left - 10, rect.bottom - rect.top - 70, true);
	MoveWindow(bot[index]->hWnd_txtChat, 0,	rect.bottom - rect.top - 70, rect.right - rect.left - bot[index]->lvsize, 16, true);
}
#endif


void ContextLVTooltip(int i, LPMSG lpMsg) {
	char clientstr[8], tmp[64];
	HWND hwnd = lpMsg->hwnd;
	int index = GetWindowLong(GetParent(hwnd), GWL_USERDATA), color;
	TOOLINFO ti;
	LVITEM lvi;
	lvi.mask	   = LVIF_IMAGE | LVIF_TEXT;
	lvi.iItem	   = i;
	lvi.iSubItem   = 0;
	lvi.pszText	   = pszTooltipTitle;
	lvi.cchTextMax = sizeof(pszTooltipTitle);
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_GETITEM, 0, (LPARAM)&lvi);
	SendMessage(hWnd_Tooltip, TTM_SETTITLE, 0, (LPARAM)pszTooltipTitle);	 
	if (lvi.iImage == -1)
		hTooltipTmp = NULL;
	else 
		hTooltipTmp = bnicons[lvi.iImage]; //ImageList_GetIcon(lvwIML, lvi.iImage, ILD_NORMAL);	
	ti.cbSize   = sizeof(TOOLINFO);
	ti.hwnd     = hWnd_main;
	ti.uId      = (UINT_PTR)hwnd;
	ti.hinst    = g_hInst;
	ti.lpszText = pszTooltipText;

	if (!bot[index]->curlvtab) {
		LPUSER lpUser = (LPUSER)GetValue(pszTooltipTitle, bot[index]->users, TABLESIZE_USERS);
		if (!lpUser)
			return;
		*(__int32 *)clientstr = lpUser->client;
		fastswap32((unsigned __int32 *)clientstr);
		clientstr[4] = 0;
		GetSmallUptimeString(GetTickCount() - lpUser->lastspoke, tmp);
		if (lpUser->clan) {											
			char tmpclan[8];
			*(int *)tmpclan = lpUser->clan;
			tmpclan[4] = 0;
			sprintf(pszTooltipText, "Client: %s\r\nFlags: 0x%02x\r\nPing: %ums\r\nClan: %s\r\nSpoke: %d lines\r\nLast: %s ago\r\n",
				clientstr, lpUser->flags, lpUser->ping, tmpclan, lpUser->speak, tmp);
		} else {
			if (*lpUser->charname) {
				sprintf(pszTooltipText, "Client: %s\r\nFlags: 0x%02x\r\nPing: %ums\r\nChar: %s\r\nSpoke: %d lines\r\nLast: %s ago\r\n",
					clientstr, lpUser->flags, lpUser->ping, lpUser->charname, lpUser->speak, tmp);
			} else {
				sprintf(pszTooltipText, "Client: %s\r\nFlags: 0x%02x\r\nPing: %ums\r\nSpoke: %d lines\r\nLast: %s ago\r\n",
					clientstr, lpUser->flags, lpUser->ping, lpUser->speak, tmp);
			}
		}
		color = GetLVColor(lpUser->flags);
	} else {
		LPLVCOLOR blah = (LPLVCOLOR)GetItemlParam(i, bot[index]->hWnd_lvwChannel);
		ti.lpszText = (char *)blah->cItem3;
		strncpy(pszTooltipText, (char *)blah->cItem3, sizeof(pszTooltipText));
		color = vbWhite;
	}

	SendMessage(hWnd_Tooltip, TTM_SETTIPTEXTCOLOR, color, 0);
	SendMessage(hWnd_Tooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);								  
	SendMessage(hWnd_Tooltip, TTM_RELAYEVENT, 0, (LPARAM)lpMsg);
	//SendMessage(hWnd_Tooltip, TTM_POPUP, 0, 0);
}


int TooltipCustDraw(LPNMTTCUSTOMDRAW lpttcd) {
	if (lpttcd->nmcd.dwDrawStage == CDDS_PREPAINT) {

		if (gfstate & GFS_UIGRADIENT) {
			DrawGradientFill(lpttcd->nmcd.hdc, &lpttcd->nmcd.rc, gradclr1, gradclr2);
		} else {
			SetBkColor(lpttcd->nmcd.hdc, 0);
			FillRect(lpttcd->nmcd.hdc, &lpttcd->nmcd.rc, hBrushclr);
		}

		//ImageList_DrawEx(lvwIML, 16, lpttcd->nmcd.hdc, lpttcd->nmcd.rc.left + 6, lpttcd->nmcd.rc.top + 6,
		//	36, 18, CLR_DEFAULT, CLR_DEFAULT, ILD_NORMAL);
		DrawIconEx(lpttcd->nmcd.hdc, lpttcd->nmcd.rc.left + 6, lpttcd->nmcd.rc.top + 6,
			hTooltipTmp, 36, 18, 0, NULL, DI_NORMAL);
		SetBkMode(lpttcd->nmcd.hdc, TRANSPARENT);
		HGDIOBJ hOld = SelectObject(lpttcd->nmcd.hdc, hFontBold);						
		ExtTextOut(lpttcd->nmcd.hdc, lpttcd->nmcd.rc.left + 50, lpttcd->nmcd.rc.top + 6,
			ETO_CLIPPED, &lpttcd->nmcd.rc, pszTooltipTitle, strlen(pszTooltipTitle), NULL);
		SelectObject(lpttcd->nmcd.hdc, hFont);
		lpttcd->nmcd.rc.left += 6;
		lpttcd->nmcd.rc.top  += 26;
		DrawText(lpttcd->nmcd.hdc, pszTooltipText, strlen(pszTooltipText), &lpttcd->nmcd.rc, 0);
		SelectObject(lpttcd->nmcd.hdc, hOld);
		return CDRF_SKIPDEFAULT;
	}
	return CDRF_DODEFAULT;
}


void SetNewFont(char *fontname, int size) {
	int height = PixelSizeToLogicalSize(size);
	int twipsize = size * 2000 / LogPixelsY;
	if (strcmp(cfFormat.szFaceName, fontname) && twipsize != cfFormat.yHeight) {
		if (hFont)
			DeleteObject(hFont);
		if (hFontBold)
			DeleteObject(hFontBold);
		hFont = CreateFont(height, 0, 0, 0, FW_REGULAR, false, false, false, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, fontname);
		hFontBold = CreateFont(height, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, fontname);
		for (int i = 0; i != numbots; i++) {
			if (bot[i])	{
				CHARRANGE cr = {0, -1};
				CHARFORMAT cf;
				cf.cbSize = sizeof(CHARFORMAT);
				cf.dwMask = CFM_FACE | CFM_SIZE;
				cf.yHeight = twipsize;
				strcpy(cf.szFaceName, fontname);
				SendMessage(bot[i]->hWnd_rtfChat, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(bot[i]->hWnd_rtfChat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				cr.cpMin = -1;
				SendMessage(bot[i]->hWnd_rtfChat, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(bot[i]->hWnd_Statusbar, WM_SETFONT, (WPARAM)hFont, true);
				SendMessage(bot[i]->hWnd_txtChat, WM_SETFONT, (WPARAM)hFont, true);
				SendMessage(bot[i]->hWnd_lblChannel, WM_SETFONT, (WPARAM)hFont, true);
			}
		}
		SendMessage(hWnd_Tab, WM_SETFONT, (WPARAM)hFont, true);
		strcpy(cfFormat.szFaceName, fontname);
	}
}


int PixelSizeToLogicalSize(int pixelsize) {
	HDC hdc = GetDC(0);
	int ratio = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(0, hdc);
	return -MulDiv(pixelsize, ratio, 72);
}


void InitGDIItems() {
	char tmp[64];
	GetStuffConfig("Font", "Size", tmp);
	int pxsize = atoi(tmp);
	int height = PixelSizeToLogicalSize(pxsize);
	GetStuffConfig("Font", "FaceName", tmp);
					   //15 == 8px
	hFont = CreateFont(height, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_RASTER_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, tmp);
	hFontBold = CreateFont(height, 0, 0, 0, 700, 0, 0, 0, DEFAULT_CHARSET, OUT_RASTER_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, tmp);
	hFontBig = CreateFont(23, 0, 0, 0, 700, true, 0, 0, DEFAULT_CHARSET, OUT_RASTER_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Veranda");
	//hPen = CreatePen(PS_SOLID, 4, 0x777777);

	HDC hdc = GetDC(0);
	LogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(0, hdc);

	cfFormat.cbSize = sizeof(CHARFORMAT);
	cfFormat.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE; //| CFM_CHARSET;
	cfFormat.yHeight = (pxsize * 2000) / LogPixelsY; //MulDiv(height, 1440, LogPixelsY); //0xAA;
	strcpy(cfFormat.szFaceName, tmp);
}


HTREEITEM AddItemToTree(HWND hwndTV, LPSTR lpszItem, int nLevel) { 
    TVITEM tvi; 
    TVINSERTSTRUCT tvins; 
    static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST; 
    static HTREEITEM hPrevRootItem = NULL; 
    static HTREEITEM hPrevLev2Item = NULL; 
    tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE; 
    tvi.pszText = lpszItem;
	tvi.state = nLevel == 1 ? TVIS_EXPANDED | TVIS_BOLD : TVIS_EXPANDED;
	tvi.stateMask = TVIS_EXPANDED | TVIS_BOLD;
    tvi.lParam = (LPARAM)nLevel; 
    tvins.item = tvi; 
    tvins.hInsertAfter = hPrev; 
    if (nLevel == 1) 
        tvins.hParent = TVI_ROOT; 
    else if (nLevel == 2) 
        tvins.hParent = hPrevRootItem; 
    else 
        tvins.hParent = hPrevLev2Item; 
    hPrev = (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins); 
    if (nLevel == 1) 
        hPrevRootItem = hPrev; 
    else if (nLevel == 2) 
        hPrevLev2Item = hPrev; 
    return hPrev; 
} 


void AlphaBlendWallpaperInit(int alpha) {
	char asdf[MAX_PATH];
	BLENDFUNCTION bf;
	BITMAP bmp;
	RECT rc;

	if (wallmemdc) {
		SelectObject(wallmemdc, hWallMemDCOld);
		DeleteObject(hBmpShade);
		DeleteDC(wallmemdc);
	}

	SystemParametersInfo(SPI_GETDESKWALLPAPER, sizeof(asdf), asdf, 0);
	
	//TODO with psuedotransparent glass windowing system:
	// - Add config code for save/loading transparency bar
	//     *ON CONFIG SAVE CALL THIS AGAIN PASSING THE NEW VALUE
	// - make these options actually do something

	HBITMAP hBmpWall = (HBITMAP)LoadImage(NULL, asdf, IMAGE_BITMAP,
		GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), LR_LOADFROMFILE);
	if (!hBmpWall) {
		AddChat(vbRed, "Cannot load wallpaper!", hWnd_status_re);
		return;
	}

	GetObject(hBmpWall, sizeof(bmp), &bmp);

	HDC hdc = GetDC(0);
	HDC tmpmemdc = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(tmpmemdc, hBmpWall);
	
	rc.left   = 0;
	rc.top    = 0;
	rc.right  = bmp.bmWidth;
	rc.bottom = bmp.bmHeight;

	wallmemdc	  = CreateCompatibleDC(hdc);
	hBmpShade	  = CreateCompatibleBitmap(hdc, bmp.bmWidth, bmp.bmHeight);
	hWallMemDCOld = SelectObject(wallmemdc, hBmpShade);

	FillRect(wallmemdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

	bf.AlphaFormat		   = AC_SRC_ALPHA;
	bf.BlendOp			   = AC_SRC_OVER;
	bf.BlendFlags		   = 0;
	bf.SourceConstantAlpha = alpha;
	AlphaBlend(wallmemdc, 0, 0, bmp.bmWidth, bmp.bmHeight,
		tmpmemdc, 0, 0, bmp.bmWidth, bmp.bmHeight, bf);

	SelectObject(tmpmemdc, hOld);
	DeleteObject(hBmpWall);
	DeleteDC(tmpmemdc);
	ReleaseDC(0, hdc);
}

