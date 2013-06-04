/**************************************************************
 *															  *
 *  dP                         oo							  *
 *  88														  *
 *  88d888b. .d8888b. 88d888b. dP d888888b .d8888b. 88d888b.  *
 *  88'  `88 88'  `88 88'  `88 88    .d8P' 88'  `88 88'  `88  *
 *  88    88 88.  .88 88       88  .Y8P    88.  .88 88    88  *
 *  dP    dP `88888P' dP       dP d888888P `88888P' dP    dP  *
 *															  *
 **************************************************************/

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

/*
	TODO
	//////////////////////~ add warcraft 3 support
	//////////////////////~ Fix the RTB stick on sb create
	~ Fix the inserted number from numpad for hotkey when switching profiles
	//////////////////////~ Fix crash on tab change after unload - Tab is unsynced with profile number.
	///////////////~ quickswitch
	//////////////////////~ friends/clan tab
	//////////~ Get icons from battle.net
	//////////////////////~ clan creation support
	~ statusbar menu
	///////////////~ Auto-stay alive ping
	//////////////////////~ Join/leave notification toggle
	//////////////////////~ Lock chat toggle
	//////////////////////~ Use a better profiling system	
	//////////////////////~ Proxy support
	//////////////////////~ Set self- speaking attributes (find self in hash table, add on send0x0e)
	~ Improve current native lockdown checkrevision implementation
	~ Activate native SRP system
	~ Custom external DLL call
	/////////////////////~ Fix up favorite tab resizing
	/////////////////////~ Dobule buffer about dialog
	/////////////////////~ Improve fade algorithm
	/////////////////////~ Make tab index searching O(1)
*/

#define DEBUG

#include "optimize.h"
#include "main.h"
#include "packetbuffer.h"
#include "fxns.h"
#include "packets.h"					
#include "resource.h"
#include "hashtable.h"
#include "config.h"
#include "profile.h"
#include "loaddll.h"
#include "gui.h"
#include "botprofiles.h"
#include "botdisplay.h"
#include "chat.h"
#include "queue.h"
#include "clan.h"
#include "checkrevision.h"
#include "atmosphere.h"
#include "tray.h"
#include "bni.h"
#include "connection.h"
#include <dbghelp.h>
#include <ole2.h>
#include <commctrl.h>
#ifdef DEBUG
	#include <crtdbg.h>
#endif

#pragma comment(lib, "OLE32.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "Dbghelp.lib")

#define NUM_GFKEYS 18
//#define NUM_GFKEYS (sizeof(gfkeys) / sizeof(char *))

unsigned long gfstate;

char *gfkeys[] = {
	//interface
	"Gradient",				
	"MergeLVandRTB",		
	"WarnClosingApp",		
	"WarnClosingProfile",	
	"TextPing",								 
	"UseAtmosphere",				
	"UseGlass",
	
	//misc
	"UseAntiflood",						
	"UseFloodProtection",				
	"FilterNumber",						  
	"ShowAccInfo",						 
	"JoinPubChan",						
	"LogChat",						
	"RTFLog",						  
	"ShowVoidUL",					  
	"URLDetect", 					
	"Decode",
	"GetIcons"
};

char *menufiles[] = {
	"Help.txt", 
	"notepad.txt",	
	"Servers.txt",
	"Proxies.txt",	
	"CDKeys.txt",
	"config.ini"	
};

char *menucmds[] = {
	"/unsquelch %c%s",
	"/squelch %c%s",
	"/ban %c%s",
	"/kick %c%s"
};

const char *buttontext[] = {"¬", "^", "×"};
char CurrDir[MAX_PATH];
char countryabbriv[16];
char countryname[32];
char quickchans[10][64];
int perpacket, perbyte, numfilter, maxrtblen, lentoclear;
int actprof, curnewprof, tabpos, rcrate, xrfactor, xrchannel;
int glassalpha;
bool favexpanded, quitting;

int loglevel;

HWND hWnd_main, hWnd_Client, hWnd_Tab, hWnd_mdiactive, hWnd_MDItask[3], 
	hWnd_Config, hWnd_Profile, hWnd_Bots, snaptopwindow, hWnd_Tooltip,
	hWnd_About, hWnd_favTree, hWnd_QS, hWnd_ClanCreate;
HWND hWnd_status, hWnd_status_re;
HINSTANCE g_hInst;
HMODULE hRichEdit;
HFONT hFont, hFontBold, hFontBig; 
//HPEN hPen;
HICON g_hIcon;					
HBRUSH hBrushclr;													

WNDPROC lpfnOldClientProc, lpfnMDIctllvw, lpfnMDIctlrtb,
	lpfnMDIctllbl, lpfnMDIctlstb, lpfnMDIctltxt, lpfnOldTabProc;
HMENU hMenu_main, hMenu_main_popup, hMenu_tab_popup, hMenu_windows_popup,
   	  hMenu_open_popup, hMenu_rtf, hMenu_lvw, hMenu_client, hMenu_lvwColumns,
	  hMenu_lvwBots, hMenu_txtExt, hMenu_lvwClan;
CHARFORMAT cfFormat;
NOTIFYICONDATA note;
HIMAGELIST lvwIML;

HHOOK hook;
RECT qsrect;

bool wpc;

LPCHAIN channels[64];

LPBOT *bot;
int numbots;

//LPPROFILES *profiles;
int numprofiles;
				 
UINT wm_taskbar;

char *lpModuleImage[NUM_LOADED_MODULES];


///////////////////////////////////////////////////////////////////////////////
 

#ifdef DEBUG
void SetDebugFlags() {
	unsigned int flag;
	flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_ALLOC_MEM_DF;
	_CrtSetDbgFlag(flag);
}
#endif


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	MSG msg;

#ifdef DEBUG
	SetDebugFlags();
#endif

	g_hInst = hInstance;
	SetUnhandledExceptionFilter(ExceptionHandler);
#ifndef DEBUG					
	CloseHandle(CreateThread(NULL, 0, SplashScreenProc, NULL, 0, NULL));
#endif
	RegisterWindows();
	hWnd_main = CreateWindow(szAppName, szAppName, 
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		120, 200, 800, 500, NULL, NULL, hInstance, NULL);
	//SetTimer(hWnd_main, 0, 100, TimerProc);
	ShowWindow(hWnd_main, SW_SHOW);
	UpdateWindow(hWnd_main);	
	while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
		if (!TranslateMDISysAccel(hWnd_Client, &msg)) {
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	}
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	int i;
	char asdf[MAX_PATH];
	//static unsigned int lastlen = 0;	
	switch (iMsg) {
		case WM_CREATE:
			hWnd_main = hwnd;
			HorizonInitialize(hwnd);
			break;
		case WM_GETMINMAXINFO:
			LPMINMAXINFO minmax;
			minmax = (LPMINMAXINFO)lParam;
			minmax->ptMinTrackSize.x = 480;
			minmax->ptMinTrackSize.y = 300;
			break;
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED) {
				TrayAdd(); 								
				return 0;
			}
			MoveMainWindowControls(lParam);
			break;
		case WM_SHELLNOTIFY:
			if ((lParam == WM_RBUTTONDOWN) || (lParam == WM_LBUTTONDBLCLK))
				TrayRestore();
			break;
		case WM_COMMAND:
			i = 0;
			switch (wParam) {
				case 90: //minimize
					ShowWindow(hWnd_mdiactive, IsIconic(hWnd_mdiactive) ? SW_RESTORE : SW_MINIMIZE);
					break;
				case 91: //maximize
					ShowWindow(hWnd_mdiactive, IsZoomed(hWnd_mdiactive) ? SW_RESTORE : SW_MAXIMIZE);
					break;
				case 92: //close
					SendMessage(hWnd_mdiactive, WM_CLOSE, 0, 0);
					break;
				case 10000: //config
					if (!hWnd_Config) {
						hWnd_Config = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONFIG), hwnd,
							(DLGPROC)ConfigProc, GetTabItemParam(SendMessage(hWnd_Tab, TCM_GETCURSEL, 0, 0)));
						ShowWindow(hWnd_Config, SW_SHOW);
					}
					break;
				case 10001:
					if (!hWnd_Bots) {
						hWnd_Bots = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_BOTS), hwnd, (DLGPROC)BotProc);
						ShowWindow(hWnd_Bots, SW_SHOW);
					}
					break;
				case 10002:
					favexpanded = !favexpanded;
					RECT rect;
					GetClientRect(hWnd_main, &rect);
					if (favexpanded) {
						MoveWindow(hWnd_Client, 150, 20, rect.right - 150, rect.bottom - 20, true);
						MoveWindow(hWnd_Tab, 150, 0, rect.right - 150, 20, true);
						ShowWindow(hWnd_favTree, SW_SHOW);
					} else {
						MoveWindow(hWnd_Client, 0, 20, rect.right, rect.bottom - 20, true);
						MoveWindow(hWnd_Tab, 0, 0, rect.right, 20, true);
						ShowWindow(hWnd_favTree, SW_HIDE);
					}
					break;
				case 10003: //about
					if (!hWnd_About) {
						hWnd_About = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)AboutProc);
						ShowWindow(hWnd_About, SW_SHOW);
					}
					break;
				case 10020:	//tile vert
				case 10021: //tile horiz
				case 10022:	//cascade
					HWND *hWnd_sdfg;
					hWnd_sdfg = (HWND *)malloc(numbots * sizeof(HWND));
					for (i = 0; i != numbots; i++)
						hWnd_sdfg[i] = bot[i]->hWnd_main;
					if (wParam == 10022)
						CascadeWindows(hWnd_Client, MDITILE_ZORDER, NULL, numbots, hWnd_sdfg);
					else
						TileWindows(hWnd_Client, wParam == 10020 ? MDITILE_VERTICAL : MDITILE_HORIZONTAL, NULL, numbots, hWnd_sdfg);						
					free(hWnd_sdfg);
					break;
				case 10030:
					strcpy(asdf, CurrDir);
					*(short *)(asdf + strlen(asdf)) = '\\';
					ShellEx(asdf);
					break;
				case 10031:
					i++;
				case 10032:
					i++;
				case 10033:
					i++;
				case 10034:
					i++;
				case 10035:
					i++;
				case 10036:
					ShellEx(menufiles[i]);
					break;
				default:
					if ((wParam >= 11000) && (wParam <= 11100)) {
						i = wParam - 11000;
						//hMenu_windows_popup
						POINT pt;
						GetCursorPos(&pt);
						if (MenuItemFromPoint(hWnd_main, hMenu_windows_popup, pt) != -1) {
							if (bot[i]->connected)
								DisconnectProfile(i, true);
							else
								ConnectProfile(i);
						}					
						BringWindowToTop(bot[i]->hWnd_main);
					}
			}
			break;
		case WM_NOTIFY:
			if (LOWORD(wParam) == 50) {
				switch (((LPNMHDR)lParam)->code) {
					case TCN_SELCHANGE:
						i = GetTabItemParam(SendMessage(hWnd_Tab, TCM_GETCURSEL, 0, 0));
						BringWindowToTop(i == -1 ? hWnd_status : bot[i]->hWnd_main);
						SetFocus(hWnd_Tab);
						break;
					case NM_RCLICK:	
						ProfilePopupMenu(0, GetTabItemParam(SendMessage(hWnd_Tab, TCM_GETCURSEL, 0, 0)));
				}
			} else if (((LPNMHDR)lParam)->hwndFrom == hWnd_Tooltip) {
				if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW)
					return TooltipCustDraw((LPNMTTCUSTOMDRAW)lParam);
			} else if (LOWORD(wParam) == 53) {
				if (((LPNMHDR)lParam)->code == NM_DBLCLK) {
					int index;
					index = actprof; //GetWindowLong(hWnd_mdiactive, GWL_USERDATA);
					if (bot[index]->connected) {
						TVITEMEX tvi;
						*(int *)asdf = ' j/';
						tvi.hItem	   = TreeView_GetSelection(hWnd_favTree);
						tvi.mask	   = TVIF_TEXT;
						tvi.pszText    = asdf + 3;
						tvi.cchTextMax = sizeof(asdf) - 3;
						SendMessage(hWnd_favTree, TVM_GETITEM, 0, (LPARAM)&tvi);
						AddQueue(asdf, index);
					}
				} else if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
					LPNMTVCUSTOMDRAW lpnmcd;
					lpnmcd = (LPNMTVCUSTOMDRAW)lParam;
					if (lpnmcd->nmcd.dwDrawStage == CDDS_PREPAINT) {
						return CDRF_NOTIFYITEMDRAW;
					} else if (lpnmcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
						if (lpnmcd->iLevel) {
							lpnmcd->clrText = 0x303030 << lpnmcd->iLevel;
							return CDRF_NEWFONT;
						}
					}
					return CDRF_DODEFAULT;
				}
			}
			break;
		case WM_CLOSE:
			return HorizonShutdown();
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			if (iMsg == wm_taskbar) {
				if (IsIconic(hwnd))
					TrayAdd();
			}
			return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}									
	return 0;
}

#if 0
int GetWindowRegion(HWND hwnd, LPPOINT lppt) {
	RECT rect;
	GetWindowRect(hwnd, &rect);	 
	if (lppt->y < 40) {
		return TABPOS_TOP;
	} else if (lppt->y > ((rect.bottom - rect.top) - 100)) {
		return TABPOS_BOTTOM;
	} else {
		return lppt->x < ((rect.right - rect.left) >> 1) ? TABPOS_LEFT : TABPOS_RIGHT; 
	}
}
#endif


LRESULT CALLBACK TabProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	static int grabtab = -1;
	//RECT rect;
	//POINT pt;
	TCHITTESTINFO cthti;
	int tmp;
	char asdf[64], sdfg[64];
	TCITEM tci1, tci2;

	switch (iMsg) {
		case WM_LBUTTONDOWN:
			cthti.pt.x = LOWORD(lParam);
			cthti.pt.y = HIWORD(lParam);
			tmp = SendMessage(hWnd_Tab, TCM_HITTEST, 0, (LPARAM)&cthti);
			grabtab = (cthti.flags & TCHT_ONITEM) ? tmp : -1;
			goto gotodef;
		case WM_LBUTTONUP:
			if (grabtab != -1) {
				cthti.pt.x = LOWORD(lParam);
				cthti.pt.y = HIWORD(lParam);
				tmp = SendMessage(hWnd_Tab, TCM_HITTEST, 0, (LPARAM)&cthti);
				if (tmp != grabtab) {
					if (cthti.flags & TCHT_ONITEM) {
						//AddChatf(asdf, hWnd_status_re, vbGreen, "tmp == %d", tmp);
						tci1.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_STATE | TCIF_TEXT;
						tci2.mask = tci1.mask;
						tci1.cchTextMax = sizeof(asdf);
						tci2.cchTextMax = sizeof(sdfg);
						tci1.pszText = asdf;
						tci2.pszText = sdfg;
						SendMessage(hWnd_Tab, TCM_GETITEM, grabtab, (LPARAM)&tci1);
						SendMessage(hWnd_Tab, TCM_GETITEM, tmp, (LPARAM)&tci2);
						SendMessage(hWnd_Tab, TCM_SETITEM, grabtab, (LPARAM)&tci2);
						SendMessage(hWnd_Tab, TCM_SETITEM, tmp, (LPARAM)&tci1);
						bot[tci1.lParam]->tabindex = tmp;
						bot[tci2.lParam]->tabindex = grabtab;
					}
				}
			}
			grabtab = -1;
#if 0
			goto gotodef;
		case WM_MOUSEMOVE:

			bool dragging;
			dragging = false;
			if (dragging) {
				//RedrawWindow(0, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
				
				GetWindowRect(hWnd_main, &rect);
				//InvalidateRect(hWnd_main, &rect, false);
				HDC hdc;
				RedrawWindow(hWnd_main, NULL, NULL, RDW_ALLCHILDREN);
				hdc = GetWindowDC(hWnd_main);
				//ClientToScreen(hwnd, &pt);
				//InvalidateRect(hwnd, NULL, true);
				if (pt.y < 40)	{
					rect.left  += 2;
					rect.right -= 2;
					rect.top   += 40;
					rect.bottom = rect.top + 20;
				} else if (pt.y > ((rect.bottom - rect.top) - 100)) {
					rect.left  += 2;
					rect.right -= 2;
					rect.top    = rect.bottom - 20;
					rect.bottom -= 2;
				} else {
					if (pt.x < ((rect.right - rect.left) >> 1)) {
						rect.left += 2;
						rect.right = rect.left + 80;
						rect.top += 40;
						rect.bottom -= 2;
					} else {
						rect.left   = rect.right - 80;
						rect.right -= 2;
						rect.top   += 40;
						rect.bottom -= 2;
					}
					
				}
				FrameRect(hdc, &rect, (HBRUSH)GetStockObject(LTGRAY_BRUSH)); 
				ReleaseDC(hWnd_main, hdc);
				//rect.left   = pt.x - 10;
				//rect.right  = pt.x + 10;
				//rect.top    = pt.y - 10;
				//rect.bottom = pt.y + 10;
				//FrameRect(GetDC(0), &rect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
			}
			break;
#endif
		default:
gotodef:
			return CallWindowProc(lpfnOldTabProc, hwnd, iMsg, wParam, lParam); 
	}									
	return 0;
}


LRESULT CALLBACK QuickSwitchProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	int i;

	switch (iMsg) {
		case WM_ERASEBKGND:
			for (i = 0; i != numbots; i++)
				RedrawWindow(bot[i]->hWnd_main, NULL, NULL, 0);
			//AddChat(vbGreen, "erasebkgnd", bot[actprof]->hWnd_rtfChat);
			return 1;
			//break;
		case WM_PAINT:
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hwnd, &ps);
			/*
			RECT rect;
			rect.bottom = 100;
			rect.top = 0;
			rect.left = 0;
			rect.right = 100;
			GetWindowRect(hwnd, &rect);
			*/
			/*
			hdc = GetDC(0);
			GetWindowRect(hwnd, &ps.rcPaint);
			InvalidateRect(hwnd, &ps.rcPaint, true);
			ShowWindow(hwnd, SW_HIDE);
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			Sleep(0);
			ShowWindow(hwnd, SW_SHOW);
			*/
			PaintTransparentRect(hdc, &ps.rcPaint, 0xFF0000, 60);
				
			HBRUSH hBrush;
			hBrush = CreateSolidBrush(0xff0000);
			FrameRect(hdc, &ps.rcPaint, hBrush); ////////FIXME
			DeleteObject(hBrush);
			//GetWindowRect(hwnd, &ps.rcPaint);
			//hdc = GetDC(0);
			HGDIOBJ hOld;
			hOld = SelectObject(hdc, hFontBold);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, 0xD0D0D0);
			TextOut(hdc, ps.rcPaint.left + 30, ps.rcPaint.top + 7, "QuickSwitch", 11);
			//SelectObject(hdc, hFont);
			for (i = 0; i != numbots; i++) {
				DrawIconEx(hdc, ps.rcPaint.left + 30, ps.rcPaint.top + 30 + (i * 22), 
					stateicons[0], 16, 16, 0, NULL, DI_NORMAL); 
				if (curnewprof == i) {
					SelectObject(hdc, hFontBold);
					SetTextColor(hdc, vbWhite);
				} else {
					SelectObject(hdc, hFont);
					SetTextColor(hdc, 0xF0A0A0);
				}
				TextOut(hdc, ps.rcPaint.left + 52, ps.rcPaint.top + 30 + (i * 22),
					bot[i]->pname, strlen(bot[i]->pname));
			}
			SelectObject(hdc, hOld);
			ReleaseDC(0, hdc);
			//FillRect(hdc, &ps.rcPaint, hBrushclr);
			EndPaint(hwnd, &ps);
			break;
		default:
			return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}									
	return 0;
}


LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	char sdfg[128];
	static bool showing = false;

	if (nCode >= 0) {
		if (nCode == HC_ACTION) {
			//sprintf(sdfg, "wParam: 0x%08x, lParam: 0x%08x", wParam, lParam);
			//AddChat(vbGreen, sdfg, bot[actprof]->hWnd_rtfChat);
			switch (wParam) {
				case 0x11: //ctrl
					if (showing) {
						if (lParam & 0x80000000) {
							ShowWindow(hWnd_QS, SW_HIDE);
							showing = false;
							BringWindowToTop(bot[curnewprof]->hWnd_main);
							SetFocus(bot[curnewprof]->hWnd_txtChat);
							//InvalidateRect(0, &qsrect, true);
						}
					}
					break;
				case VK_TAB:
					if (!(lParam & 0x40000000)) {
						if (GetKeyState(0x11) & 0xFF00) {
							if (!showing) {
								showing = true;
								curnewprof = actprof;
								ShowWindow(hWnd_QS, SW_SHOW);
								//PaintTransparentRect(&qsrect, 0xFF0000, 0x7F);
							}	
						} else {
							if (actprof != -1)
								SetFocus(bot[actprof]->hWnd_txtChat);
							return 0;
						}
					} else if (lParam & 0x40000000) {
						if (GetKeyState(0x11) & 0xFF00) {
							curnewprof++;
							if (curnewprof == numbots)
								curnewprof = 0;
							ShowWindow(hWnd_QS, SW_HIDE);
							//RedrawWindow(hWnd_QS, NULL, NULL, RDW_INTERNALPAINT);
							RedrawWindow(hWnd_main, NULL, NULL, RDW_ERASE | RDW_INTERNALPAINT | RDW_ALLCHILDREN);
						
							ShowWindow(hWnd_QS, SW_SHOW);
							//PaintTransparentRect(&qsrect, 0xFF0000, 0x7F);						
						}
					} else if (lParam & 0x80000000) {
						SetFocus(bot[curnewprof]->hWnd_txtChat);
					}
					break;
				case VK_F11:
					if (!(lParam & 0xFF000000))
						ConnectProfile(actprof);
					break;
				case VK_F12:
					if (!(lParam & 0xFF000000))
						DisconnectProfile(actprof, true);
					break;
				case 0x91:
					if (!(lParam & 0xFF000000)) {
						bot[actprof]->scrllock = !bot[actprof]->scrllock;
						sprintf(sdfg, "Scroll lock has been %sabled.", bot[actprof]->scrllock ? "en" : "dis");
						AddChat(vbRDBlue, sdfg, bot[actprof]->hWnd_rtfChat);
					}
					break;
				default:
					if (!(lParam & 0xFF000000)) {
						if (wParam > 0x5F && wParam < 0x6A) {
							int i = wParam - 0x60;
							if (i < numbots) {
								BringWindowToTop(bot[i]->hWnd_main);
								SetFocus(bot[i]->hWnd_txtChat);
								return 0;
							}
						} else if (wParam >= VK_F1 && wParam <= VK_F10) {
							AddQueue(quickchans[wParam - VK_F1], actprof);	
						}
					}
			}
		}
	}
	return CallNextHookEx(hook, nCode, wParam, lParam);
}


LRESULT CALLBACK MDIClientProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg) {
		case WM_PAINT:
			PAINTSTRUCT paint;
			HDC hdc;
			hdc = BeginPaint(hwnd, &paint);
			PaintDesktop(hdc);
			EndPaint(hwnd, &paint);
			break;
		case WM_CONTEXTMENU:
			unsigned long lngRet;
			lngRet = TrackPopupMenuEx(hMenu_client, 
				TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
				LOWORD(lParam), HIWORD(lParam), hWnd_Client, NULL);
			break;
		default:
			return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
	return 0;
}


LRESULT CALLBACK MDIChildProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	int index, i;
	HDC hdcpic, hdc;
	RECT rect;
	char asdf[128];
	char sdfg[512];
	char *tmp;
	switch (iMsg) {
		case WM_CREATE:
			/*__asm {
				mov ecx, hwnd 
				mov eax, 0x77D484D0	   
				call eax			   
			} */		
			index = ((LPMDICREATESTRUCT)((LPCREATESTRUCT)lParam)->lpCreateParams)->lParam;
			if (index != -1)
				CreateMDIChildControls(hwnd, index);
			SetWindowLong(hwnd, GWL_USERDATA, index);
			SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_COMPOSITED);
			break;
		case WM_SIZE:
			index = GetWindowLong(hwnd, GWL_USERDATA);
			if (index != -1) {
				wpc = true;
				MoveWindow(bot[index]->hWnd_rtfChat, 0, 0, LOWORD(lParam) - bot[index]->lvwidth, HIWORD(lParam) - 65, false);					
				MoveWindow(bot[index]->hWnd_lblChannel, LOWORD(lParam) - bot[index]->lvwidth, 0, bot[index]->lvwidth, 20, false);
				MoveWindow(bot[index]->hWnd_lvwChannel, LOWORD(lParam) - bot[index]->lvwidth, 20, bot[index]->lvwidth, HIWORD(lParam) - 45 - 20, false);
				MoveWindow(bot[index]->hWnd_Statusbar, 0, HIWORD(lParam) - 20, LOWORD(lParam), 20, false);
				//MoveWindow(bot[index]->hWnd_cboChat, 0, HIWORD(lParam) - 45, LOWORD(lParam) - bot[index]->lvsize, 200, true);
				MoveWindow(bot[index]->hWnd_txtChat, 0, HIWORD(lParam) - 65, LOWORD(lParam) - bot[index]->lvwidth, 40, false);
				MoveWindow(bot[index]->hWnd_tab, LOWORD(lParam) - bot[index]->lvwidth, HIWORD(lParam) - 45, bot[index]->lvwidth, 20, false);
				UpdateWindow(bot[index]->hWnd_main);
				wpc = false;
			} else {
				MoveWindow(hWnd_status_re, 0, 0, LOWORD(lParam), HIWORD(lParam), true);	
			}
			break;
		case WM_MOVE:
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case WM_GETMINMAXINFO:
			LPMINMAXINFO minmax;
			minmax = (LPMINMAXINFO)lParam;
			minmax->ptMinTrackSize.x = 320;
			minmax->ptMinTrackSize.y = 240;
			break;
		case WM_DRAWITEM:
			LPDRAWITEMSTRUCT lpdi;
			HBITMAP hOld;
			RECT rc;
			lpdi = (LPDRAWITEMSTRUCT)lParam;
			index = GetWindowLong(hwnd, GWL_USERDATA);
			if (gfstate & GFS_USEGLASS) {
				GetWindowRect(lpdi->hwndItem, &rc);
				BitBlt((HDC)lpdi->hDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
					wallmemdc, rc.left, rc.top, SRCCOPY);
			} else {
				if (bot[index]->lblPic) {
					hdcpic = CreateCompatibleDC(lpdi->hDC);
					hOld = (HBITMAP)SelectObject(hdcpic, bot[index]->lblPic);
					BitBlt(lpdi->hDC, lpdi->rcItem.left, lpdi->rcItem.top, 
						lpdi->rcItem.right - lpdi->rcItem.left,
						lpdi->rcItem.bottom - lpdi->rcItem.top, hdcpic, 0, 0, SRCCOPY);
					SelectObject(hdcpic, hOld);
					DeleteDC(hdcpic);
				} else {
					if (gfstate & GFS_UIGRADIENT)
						DrawGradientFill(lpdi->hDC, &lpdi->rcItem, gradclr1, gradclr2);
					else 
						FillRect(lpdi->hDC, &lpdi->rcItem, (HBRUSH)hBrushclr);
				}		
			}
			GetClientRect(bot[index]->hWnd_lvwChannel, &rect);
			SetBkMode(lpdi->hDC, TRANSPARENT);
			SetTextColor(lpdi->hDC, 0xFFFFFF);
			index = GetWindowText(lpdi->hwndItem, asdf, sizeof(asdf));
			TextOut(lpdi->hDC, lpdi->rcItem.left + (rect.right >> 1) - (index << 1), lpdi->rcItem.top, asdf, index);
			return true;
		case WM_ERASEBKGND:
			index = GetWindowLong(hwnd, GWL_USERDATA);
			if (gfstate & GFS_USEGLASS) {
				GetWindowRect(index == -1 ? hWnd_status_re : bot[index]->hWnd_rtfChat, &rect);
				BitBlt((HDC)wParam, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
					wallmemdc, rect.left, rect.top, SRCCOPY);
				return true;  	 
			} else {
				if (index != -1 && bot[index]->rtbPic) {
					HBITMAP hOld;
					GetWindowRect(bot[index]->hWnd_rtfChat, &rect);
					hdc = (HDC)wParam;
					hdcpic = CreateCompatibleDC(hdc);
					hOld = (HBITMAP)SelectObject(hdcpic, bot[index]->rtbPic);
					StretchBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdcpic, 0, 0, 480, 400, SRCCOPY);
					SelectObject(hdcpic, hOld);
					DeleteDC(hdcpic);			 
					return true;				////known problems:
				} else {					    ////  RE not transparent when glass is turned off, 												
					goto gotodefault;			////  should check if the rtbpic is nonzero and set trans flag accordingly
				} 								////
			}									////  separate edit control parser etc. 
		case WM_NOTIFY:							////
			switch (LOWORD(wParam)) {
				case 30:
					if (((LPNMHDR)lParam)->code == EN_LINK) {
						if (((ENLINK *)lParam)->msg == WM_LBUTTONUP) {
							index = GetWindowLong(hwnd, GWL_USERDATA);
							SendMessage(bot[index]->hWnd_rtfChat, EM_EXSETSEL, 0, (long)&((ENLINK *)lParam)->chrg);
							SendMessage(bot[index]->hWnd_rtfChat, EM_GETSELTEXT, 0, (long)sdfg);
							ShellEx(sdfg);
						}
					}
					break;
				case 55:
					BringWindowToTop(hwnd);	
					if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
						index = GetWindowLong(hwnd, GWL_USERDATA);
						if (bot[index]->curlvtab)
							ClearLVTmpToolTipText(index);
						SendMessage(bot[index]->hWnd_lvwChannel, LVM_DELETEALLITEMS, 0, 0);
						//ResetChLVContents(index);
						bot[index]->curlvtab = TabCtrl_GetCurSel(((LPNMHDR)lParam)->hwndFrom);
						if (!bot[index]->connected)
							break;
						switch (bot[index]->curlvtab) {
							case LVTAB_CHANNEL:
								ReloadChannelList(index);
								break;
							case LVTAB_FRIENDS:
								bot[index]->fstate |= BFS_REQFLIST;
								SendPacket(0x65, index);
								AddChat(vbYellow, "[BNET] Requesting friends list...", bot[index]->hWnd_rtfChat);
								break;
							case LVTAB_CLAN:
								if (bot[index]->fstate & BFS_WARCRAFT3) {
									bot[index]->fstate |= BFS_REQCLANINFO;
									Send0x7D(index);
									AddChat(vbYellow, "[BNET] Requesting clan list...", bot[index]->hWnd_rtfChat);
								}
						}
					}
					break;
				case 60:
					switch (((LPNMHDR)lParam)->code) { 
						case NM_DBLCLK:
							char sdfgtmp[512], asdftemp[64];
							int selIndex;
							selIndex = SendMessage(((NMHDR *)lParam)->hwndFrom, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
							if (selIndex != -1)	{
								index = GetWindowLong(hwnd, GWL_USERDATA);
								GetWindowText(bot[index]->hWnd_txtChat, sdfgtmp, sizeof(sdfgtmp));
								strcat(sdfgtmp, GetItemText(selIndex, asdftemp, sizeof(asdftemp), index));
								SetWindowText(bot[index]->hWnd_txtChat, sdfgtmp);
							}
							break;	
						case NM_CUSTOMDRAW:
							return ParseCustomDraw(((NMHDR *)lParam)->hwndFrom, (LPNMLVCUSTOMDRAW)lParam);
					}
			}
			break;
		case WM_MDIACTIVATE:
			hWnd_mdiactive = hwnd;
			actprof = GetWindowLong(hwnd, GWL_USERDATA);
			SendMessage(hWnd_Tab, TCM_SETCURSEL, GetTabFromIndex(actprof), 0);
			break;
		case WM_CLOSE:
			index = GetWindowLong(hwnd, GWL_USERDATA);
			if (index == -1)
				break;
			SaveWindowPos(index);
			if (!quitting) {
				if (gfstate & GFS_UIWARNEXITPROF) {
					if (MessageBox(hwnd, "Are you sure you'd like to close this profile?", "Close?", MB_YESNO) == IDNO)
						return 0;	
				}
			}
			DisconnectProfile(index, true);
			GETTEXTLENGTHEX gtl;
			gtl.codepage = CP_ACP;		///////////////////////////////////////////
			gtl.flags    = GTL_USECRLF;
			i = SendMessage(bot[index]->hWnd_rtfChat, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
			if (i) {
				tmp = (char *)malloc(i + 1);
				tmp[i] = 0;
				GetWindowText(bot[index]->hWnd_rtfChat, tmp, i);
				AddLog(tmp, index);
				free(tmp);
			}
			UnloadProfile(index);
			DestroyWindow(hwnd);					
			break;
		case WM_DESTROY:
			index = GetWindowLong(hwnd, GWL_USERDATA);
			if (index == -1)
				break;
			SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)NULL);
			break;
		case WM_CTLCOLORSTATIC:								 
		case WM_CTLCOLOREDIT:
			SetTextColor((HDC)wParam, vbWhite);
			SetBkColor((HDC)wParam, (iMsg == WM_CTLCOLOREDIT && 
				   GetWindowTextLength((HWND)lParam) >= 223) ? 0x000080 : vbBlack);
			return (long)GetStockObject(BLACK_BRUSH);
		case WM_CONTEXTMENU:
			int lngRet; 
			lngRet = TrackPopupMenuEx(hMenu_lvwColumns, 
				TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
				LOWORD(lParam), HIWORD(lParam), hWnd_main, NULL);
			MENUITEMINFO mii;
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_STATE;
			GetMenuItemInfo(hMenu_lvwColumns, lngRet, false, &mii);
			if (lngRet == 10080) {
				for (i = 0; i != numprofiles; i++) {
					if (bot[i]->ploaded) {
						index = GetWindowLong(bot[i]->hWnd_lvwChannel, GWL_STYLE);
						if (mii.fState & MFS_CHECKED)
							index &= ~LVS_NOCOLUMNHEADER;
						else
							index |= LVS_NOCOLUMNHEADER;
						SetWindowLong(bot[i]->hWnd_lvwChannel, GWL_STYLE, index);
					}
				}
			} else {
				for (i = 0; i != numprofiles; i++) {
					if (bot[i]->ploaded)
						SendMessage(bot[i]->hWnd_lvwChannel, LVM_SETCOLUMNWIDTH, lngRet - 10081,
							(LPARAM)mii.fState & MFS_CHECKED ? 0 : (lngRet - 10081 ? 50 : 150));
				}
			}
			mii.fState = mii.fState & MFS_CHECKED ? 0 : MFS_CHECKED;
			SetMenuItemInfo(hMenu_lvwColumns, lngRet, false, &mii);
			break;
		case WM_WSDATAARRIVAL:
			HandleDataRecv(GetWindowLong(hwnd, GWL_USERDATA), (SOCKET)wParam, lParam);
			break;
		case WM_LBUTTONDOWN:
			BringWindowToTop(hwnd);
		default:
gotodefault:
			return DefMDIChildProc(hwnd, iMsg, wParam, lParam);
	}
	return 0;
}


//////////////////////////////////RTB PROC///////////////////////////////////////
LRESULT CALLBACK MDIrtbProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	unsigned long lngRet;
	int index, flags;
	switch (iMsg) {
		case WM_CONTEXTMENU:
			lngRet = TrackPopupMenuEx(hMenu_rtf, 
				TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
				LOWORD(lParam), HIWORD(lParam), hWnd_main, NULL);
			switch (lngRet) {
				case 10040:
				case 10041:
				case 10042:
				case 10043:
				case 10044:
					index = GetWindowLong(hwnd, GWL_USERDATA);
					flags = 1 << (lngRet - 10040);
					bot[index]->txtsendattrib ^= flags; 
					CheckMenuItem(hMenu_txtExt,	lngRet, ((bot[index]->txtsendattrib & flags) ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
					break;
				case 10050: //copy
					SendMessage(hwnd, WM_COPY, 0, 0);
					break;
				case 10051:	//select all
					CHARRANGE cr;
					cr.cpMin = 0;
					cr.cpMax = -1;
					SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
					break;
				case 10052:	//notepad
					char path[MAX_PATH];
					sprintf(path, "%s\\notepad.txt", CurrDir);
					ShellExecute(NULL, NULL, path, NULL, "C:\\", 1);
					break;
				case 10054:	//about
					SendMessage(hWnd_main, WM_COMMAND, 10003, 0);
			}						
			break;
		
		case WM_SIZING:
			switch (wParam) {
				case WMSZ_BOTTOM:
				case WMSZ_RIGHT:
				case WMSZ_BOTTOMRIGHT:
					return false;
			}
			ReleaseCapture();
			return true;  
		case WM_GETMINMAXINFO:
			LPMINMAXINFO lpmmi;
			HWND phwnd;									  
			RECT rect;
			lpmmi = (LPMINMAXINFO)lParam;
			phwnd = GetParent(hwnd);
			if (GetWindowLong(phwnd, GWL_USERDATA) != -1) {
				GetClientRect(phwnd, &rect);
				lpmmi->ptMaxTrackSize.x = rect.right - 120;
				lpmmi->ptMaxTrackSize.y = rect.bottom - 45;
			}
			lpmmi->ptMinTrackSize.x = 300;
			lpmmi->ptMinTrackSize.y = 200;
			break;
		case WM_SIZE:
			//break;
			index = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			if (index != -1) {
				if (wpc)
					break;
				GetWindowRect(GetParent(hwnd), &rect);
				bot[index]->lvwidth = (rect.right - rect.left) - LOWORD(lParam);
				bot[index]->txtheight = (rect.bottom - rect.top) - HIWORD(lParam);
				//char asdf[64];
				//AddChatf(asdf, hwnd, vbYellow, "%d lv width, %d txt height",
				//			bot[index]->lvwidth, bot[index]->txtheight);
				break;
				bool sbexists;
				SCROLLINFO sbi;	 //////fix logging, clearchat command
				sbi.cbSize = sizeof(SCROLLINFO);
				sbi.fMask = SIF_POS;
				sbexists = GetScrollInfo(hwnd, SB_VERT, &sbi);
				//sbexists = //!(sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE);

				MoveWindow(bot[index]->hWnd_txtChat, 0, HIWORD(lParam) + 10,
					LOWORD(lParam) + 9 + (sbexists ? 18 : 1),
					rect.bottom - rect.top - HIWORD(lParam) - 62, false);

				MoveWindow(bot[index]->hWnd_lvwChannel, LOWORD(lParam) + 9 + (sbexists ? 18 : 0), 20,
					rect.right - rect.left - LOWORD(lParam) - 21 - (sbexists ? 14 : 1),
					rect.bottom - rect.top - 92, false);
				
				MoveWindow(bot[index]->hWnd_lblChannel, LOWORD(lParam) + 9 + (sbexists ? 18 : 0), 0,
					rect.right - rect.left - LOWORD(lParam) - 20 - (sbexists ? 14 : 0), 20, false);

				MoveWindow(bot[index]->hWnd_tab, LOWORD(lParam) + 9 + (sbexists ? 18 : 0),
					rect.bottom - rect.top - 72,
					rect.right - rect.left - LOWORD(lParam) - 18 - (sbexists ? 14 : 0), 20, false);

				//MoveWindow(bot[index]->hWnd_lblChannel, 0, HIWORD(lParam) + 5,
				//			LOWORD(lParam) + 4, 50, false);
			}
			break;
		case WM_LBUTTONDOWN:
			BringWindowToTop(GetParent(hwnd));						  
		default:
			return CallWindowProc(lpfnMDIctlrtb, hwnd, iMsg, wParam, lParam);
	}
	return 0;
}


//////////////////////////////////////CBO PROC///////////////////////////////////
#if 0
LRESULT CALLBACK MDIcboProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	char sdfg[512];
	int index;
	RECT rect;
	HWND cbohwnd;
	switch (iMsg) {
		case WM_CHAR:
			if (wParam == VK_RETURN) {
				GetWindowText(hwnd, sdfg, sizeof(sdfg));
				if (sdfg[0]) {
					SetWindowText(hwnd, NULL);						
					cbohwnd = GetParent(hwnd);
					index = GetWindowLong(GetParent(cbohwnd), GWL_USERDATA);
					if (sdfg[0] == '/' && ParseCommand(sdfg, index))
						goto skipsend;
					if (GetKeyState(VK_END) & 0xFF00)
						Send0x0E(sdfg, index);
					else
						AddQueue(sdfg, index);
skipsend:
					if (SendMessage(cbohwnd, CB_GETCOUNT, 0, 0) >= 32)
						SendMessage(cbohwnd, CB_DELETESTRING, 0, 0);
					SendMessage(cbohwnd, CB_ADDSTRING, 0, (LPARAM)sdfg);
				}
				return 0;
			} else if (wParam == 0x0A) {
				int start, end;
				index = GetWindowLong(GetParent(GetParent(hwnd)), GWL_USERDATA);
				GetWindowText(bot[index]->hWnd_cboChat, sdfg, sizeof(sdfg));
				SetWindowText(bot[index]->hWnd_txtChat, sdfg);
				ShowWindow(bot[index]->hWnd_cboChat, SW_HIDE);
				ShowWindow(bot[index]->hWnd_txtChat, SW_SHOW);
				GetClientRect(bot[index]->hWnd_rtfChat, &rect);	
				SetWindowPos(bot[index]->hWnd_txtChat, NULL, 0, rect.bottom, rect.right, 50, SWP_NOZORDER); 
				SendMessage(bot[index]->hWnd_cbotxtChat, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
				SendMessage(bot[index]->hWnd_txtChat, EM_SETSEL, start, end);
				SendMessage(bot[index]->hWnd_txtChat, EM_REPLACESEL, false, (LPARAM)"\r\n");
				SetFocus(bot[index]->hWnd_txtChat);
				bot[index]->expanded = true;
				return 0;
			}
			goto gotodefault;
		case WM_PASTE:
			ShiftWindowMultiClip(GetWindowLong(GetParent(GetParent(hwnd)), GWL_USERDATA), hwnd);
			break;
		case WM_LBUTTONDOWN:
			BringWindowToTop(GetParent(GetParent(hwnd)));
		default:
gotodefault:
			return CallWindowProc(lpfnMDIctlcbo, hwnd, iMsg, wParam, lParam);
	}
	return 0;
}
#endif


//////////////////////////////////TXT PROC///////////////////////////////////////
LRESULT CALLBACK MDItxtProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	char *tmp;
	char sdfg[512];
	int index;
	switch (iMsg) {
		case WM_KEYDOWN:
			if (wParam == 0x26) {
				if (GetKeyState(0x11) & 0xFF00) {
					index = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
					SetWindowText(hwnd, bot[index]->lastmsgs[bot[index]->curmsgscrollpos]);
					if (bot[index]->curmsgscrollpos == 19)
						bot[index]->curmsgscrollpos = 0;
					else 
						bot[index]->curmsgscrollpos++;
					SendMessage(bot[index]->hWnd_txtChat, EM_SETSEL, 0x7FFF, 0x7FFF);
					return 0;
				}
			} else if (wParam == 0x28) {
				if (GetKeyState(0x11) & 0xFF00) {
					index = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
					SetWindowText(hwnd, bot[index]->lastmsgs[bot[index]->curmsgscrollpos]);
					if (bot[index]->curmsgscrollpos)
						bot[index]->curmsgscrollpos--; 
					else
						bot[index]->curmsgscrollpos = 19;
					SendMessage(bot[index]->hWnd_txtChat, EM_SETSEL, 0x7FFF, 0x7FFF);
					return 0;	
				}
			}
			goto gotodefault;
		case WM_CHAR:
			switch (wParam)	{
				case VK_RETURN:
					HandleReturnKey(hwnd, GetWindowLong(GetParent(hwnd), GWL_USERDATA));				
					return 0;
				case 0x09:
					index = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
					GetWindowText(bot[index]->hWnd_txtChat, sdfg, sizeof(sdfg));
					tmp = sdfg;
					if (((*(int *)tmp) & 0xFFFFFF) == ' w/') {
						if (bot[index]->fstate & BFS_DIABLO2) {
							tmp[3] = '*';
							tmp++;
						}
						tmp += 3;
						strcpy(tmp, bot[index]->lastwhispered);
						*(short *)(tmp + strlen(bot[index]->lastwhispered)) = ' ';
						SetWindowText(bot[index]->hWnd_txtChat, sdfg);
					} else {
						SetWindowText(bot[index]->hWnd_txtChat,
							(bot[index]->fstate & BFS_DIABLO2) ? "/w *" : "/w ");
					}
					SendMessage(bot[index]->hWnd_txtChat, EM_SETSEL, 0x7FFF, 0x7FFF);
					return 0;
				//case 0x0A:
				//	SendMessage(hwnd, EM_REPLACESEL, false, (LPARAM)"\r\n");
				//	goto gotodefault;
			}
			goto gotodefault;
		case WM_LBUTTONDOWN:
			BringWindowToTop(GetParent(hwnd));
		default:
gotodefault:
			return CallWindowProc(lpfnMDIctltxt, hwnd, iMsg, wParam, lParam);
	}
	return 0;
}


void HandleReturnKey(HWND hwnd, int index) {
	char *ltxtbuf, tmpchar; //, *tmpbuf;
	int len, i;
	char *text, *tmp;
	tmpchar = 0;	
	i = 0;
	len = GetWindowTextLength(hwnd) + 4;
	ltxtbuf = (char *)malloc(len);
	*(int *)(ltxtbuf + len - 4) = 0;
	GetWindowText(hwnd, ltxtbuf, len);
	if (*ltxtbuf) {
		SetWindowText(hwnd, NULL);		
		text = ltxtbuf;
		tmp = NULL;
		do {
			if (tmpchar) {
				text--;
				*text = tmpchar;
				tmpchar = 0;
			}

			tmp = strchr(text, '\r');
			if (tmp) {
				*(short *)tmp = 0;
				tmp += 2;
			}
			bot[index]->lastmsgs[bot[index]->curmsgpos] = 
				(char *)realloc(bot[index]->lastmsgs[bot[index]->curmsgpos], 
				(strlen(text) + 1) * sizeof(char));
			strcpy(bot[index]->lastmsgs[bot[index]->curmsgpos], text);
			bot[index]->curmsgscrollpos = bot[index]->curmsgpos;
			bot[index]->curmsgpos++;
			if (bot[index]->curmsgpos == 20)
				bot[index]->curmsgpos = 0;

			if (*text == '/') {
				if (ParseCommand(text, index))
					goto nextiter;
			} else { 

			}

#if 0
			//do transforms here
			tmpbuf = text;
			text = TextTransformOutgoing(text, bot[index]->txtsendattrib);
#endif

			if (strlen(text) >= 223) {
				tmpchar = text[223];
				text[223] = 0;
			}

			if (GetKeyState(VK_END) & 0xFF00)
				Send0x0E(text, index);
			else
				AddQueue(text, index);

			//if (bot[index]->txtsendattrib & (TM_XR | TM_HEX | TM_CONVERTTABS))
			//	free(text);
			//text = tmpbuf;
nextiter:
			text += strlen(text) + 1;
		} while (text[1]);  
	}			
//freebuf:
	free(ltxtbuf);
}


////////////////////////////////////LBL PROC/////////////////////////////////////
LRESULT CALLBACK MDIlblProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg) {
		case WM_LBUTTONDOWN:
			BringWindowToTop(GetParent(hwnd));
		default:
			return CallWindowProc(lpfnMDIctllbl, hwnd, iMsg, wParam, lParam);
	}	 
	return 0;
}


////////////////////////////////////////STB PROC/////////////////////////////////
LRESULT CALLBACK MDIstbProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg) {
		case WM_LBUTTONDOWN:
			BringWindowToTop(GetParent(hwnd));
		default:
			return CallWindowProc(lpfnMDIctlstb, hwnd, iMsg, wParam, lParam);
	}
	return 0;
}


/////////////////////////////////////LVW PROC////////////////////////////////////
LRESULT CALLBACK MDIlvwProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)	{
	char sdfg[64], asdf[256], dfgh[256], *tmp;
	int i, lngRet, j;
	static bool cursorset = false;
	MENUITEMINFO mii;
	static int lastindex = -1;
	switch (iMsg) {
		case WM_CONTEXTMENU:
			i = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
			if (bot[i]->connected) {
				if (bot[i]->curlvtab == 1)
					break;
				mii.cbSize = sizeof(MENUITEMINFO); 
				mii.fMask = MIIM_STATE;
				mii.fState = (bot[i]->fstate & BFS_WARCRAFT3) ? MFS_ENABLED : MFS_DISABLED; 
				SetMenuItemInfo(hMenu_lvw, 10066, false, &mii);
				lngRet = TrackPopupMenuEx(!bot[i]->curlvtab ? hMenu_lvw : hMenu_lvwClan, 
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
					LOWORD(lParam), HIWORD(lParam), hWnd_main, NULL);
				*sdfg = 0;
				j = 0;
				GetItemText(SendMessage(hwnd, LVM_GETNEXTITEM, -1, LVNI_FOCUSED), sdfg, sizeof(sdfg), i);
				switch (lngRet) {
					case 10060:	//profile
						if (hWnd_Profile)
							DestroyWindow(hWnd_Profile);
						PROFILEINIT pi;
						pi.index = i;
						pi.username = sdfg;
						hWnd_Profile = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_PROFILE), hWnd_main, (DLGPROC)ProfileProc, (LONG)&pi); 
						ShowWindow(hWnd_Profile, SW_SHOW);
						break;														 
					case 10061:	//whisper
						GetWindowText(bot[i]->hWnd_txtChat, dfgh, sizeof(dfgh));
						sprintf(asdf, "/w %c%s %s", (bot[i]->fstate & BFS_DIABLO2) ? '*' : ' ', sdfg, dfgh);
						AddQueue(asdf, i);
						SetWindowText(bot[i]->hWnd_txtChat, NULL);
						break;
					case 10062:	//kick
						j++;
					case 10063: //ban
						j++;
					case 10064: //squelch
						j++;
					case 10065: //unsquelch
						sprintf(asdf, menucmds[j], (bot[i]->fstate & BFS_DIABLO2) ? '*' : ' ', sdfg);
						AddQueue(asdf, i);
						break;
					case 10066: //clan invite
						if (bot[i]->fstate & BFS_WARCRAFT3) {
							Send0x77(sdfg, i);
							sprintf(asdf, "Sending a clan invitation to %s.", sdfg);
							AddChat(vbYellow, asdf, bot[i]->hWnd_rtfChat);
						}
						break;
					case 10067: //web profile
						tmp = strchr(sdfg, '@');
						if (tmp)
							*tmp = 0;
						sprintf(asdf, "http://battle.net/war3/ladder/war3-"
							"player-profile.aspx?Gateway=%s&PlayerName=%s",
							GetServer(i, true), sdfg);
						ShellEx(asdf);
						break;
					case 10200: //member info
						Send0x82(sdfg, i);
						break;
					case 10201: //make chieftain
						if (MessageBox(0, "Are you sure?", "Change chieftain",
							MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
						Send0x74(sdfg, i);
						break;
					case 10202: //make shaman
						j++;                  
					case 10203: //make grunt
						j++; 
					case 10204: //make peon
						j++;
						Send0x7A(sdfg, j, i);
						break;
					case 10205: //remove member
						Send0x78(sdfg, i);
						break;
					case 10206: //create new clan
						if (!hWnd_ClanCreate) {
							hWnd_ClanCreate = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CREATECLAN),
								hwnd, (DLGPROC)ClanCreateProc, i);
							ShowWindow(hWnd_ClanCreate, SW_SHOW);
						}
				}
			}
			break;
		case WM_ERASEBKGND:
			RECT rect;
			if (gfstate & GFS_USEGLASS) {
				GetWindowRect(hwnd, &rect);
				BitBlt((HDC)wParam, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
					wallmemdc, rect.left, rect.top, SRCCOPY);
				return true;
			} else {
				goto gotodefault;
			}
		case WM_MOUSEMOVE:
			LVHITTESTINFO lvhtti;
			lvhtti.flags = LVHT_ONITEMLABEL;
			lvhtti.pt.x = lParam & 0xFFFF;
			lvhtti.pt.y = (lParam & 0xFFFF0000) >> 16;
			i = SendMessage(hwnd, LVM_HITTEST, 0, (LPARAM)&lvhtti);
			if (i != -1) {
				if (i != lastindex) {
					MSG msg;
					msg.hwnd = hwnd;
					msg.wParam = wParam;
					msg.lParam = lParam;
					msg.message = iMsg;
					ContextLVTooltip(i, &msg);
				}	
			}
			lastindex = i; 
			break;
		case WM_LBUTTONDOWN:
			BringWindowToTop(GetParent(hwnd));
		default:
gotodefault:
			return CallWindowProc(lpfnMDIctllvw, hwnd, iMsg, wParam, lParam);
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////


void LoadGlobalSettings() {
	char asdf[256];
	GetStuffConfig("Main", "CountryAbbriv", countryabbriv);
	GetStuffConfig("Main", "CountryName", countryname);
	rcrate     = atoi(GetStuffConfig("Misc", "ReconnectRate", asdf));
	perpacket  = atoi(GetStuffConfig("Misc", "PerPacket", asdf));
	perbyte    = atoi(GetStuffConfig("Misc", "PerByte", asdf));
	numfilter  = atoi(GetStuffConfig("Misc", "NumFilter", asdf));
	xrchannel  = atoi(GetStuffConfig("Misc", "XRChannel", asdf));
	xrfactor   = atoi(GetStuffConfig("Misc", "XRFactor", asdf));
	maxrtblen  = atoi(GetStuffConfig("Interface", "MaxRTBLen", asdf));
	lentoclear = atoi(GetStuffConfig("Interface", "CharsToClear", asdf));
	glassalpha = atoi(GetStuffConfig("Interface", "PsuedotransparencyAlpha", asdf));
	sscanf(GetStuffConfig("Interface", "GradientClr1", asdf), "0x%08x", &gradclr1);
	sscanf(GetStuffConfig("Interface", "GradientClr2", asdf), "0x%08x", &gradclr2);

	GetStuffConfig("Interface", "TabPos", asdf);
	
	tabpos = *asdf ? *asdf - 0x30 : TABPOS_TOP;

	LoadGlobalFlags();

	int scrwidth  = GetSystemMetrics(SM_CXFULLSCREEN) >> 1;
	int scrheight = GetSystemMetrics(SM_CYFULLSCREEN) >> 1;
	qsrect.left   = scrwidth  - 150;
	qsrect.top    = scrheight - 75;
	qsrect.right  =	scrwidth  + 150;
	qsrect.bottom = scrheight + 75;
}


void LoadGlobalFlags() {
	char tmp[64];
	gfstate = 0;
	for (int i = 0; i != NUM_GFKEYS; i++) {
		GetStuffConfig(i > 6 ? "Misc" : "Interface", gfkeys[i], tmp);
		if (*tmp) {
			if (*tmp - 0x30)
				gfstate |= (1 << i);
		}
	}
}


void SetGlobalAttributes() {
	RECT rect;
	char tmp[4];
	int hatchpatterns[] = { 
		HS_CROSS,
		HS_DIAGCROSS,
		HS_VERTICAL,
		HS_FDIAGONAL,
		HS_BDIAGONAL,
		HS_HORIZONTAL
	};

	if (hBrushclr)
		DeleteObject(hBrushclr);
	if (!(gfstate & GFS_UIGRADIENT)) {
		GetStuffConfig("Interface", "BrushHatchType", tmp);
		int htype = *tmp - 0x30;
		if (htype)
			hBrushclr = CreateHatchBrush(hatchpatterns[--htype], gradclr1);
		else
			hBrushclr = CreateSolidBrush(gradclr1);
	}			  


	if (gfstate & GFS_USEGLASS)
		AlphaBlendWallpaperInit(glassalpha);
	
	int style = GetWindowLong(hWnd_Tab, GWL_STYLE);
	style &= ~TCS_BOTTOM & ~TCS_BUTTONS & ~TCS_FLATBUTTONS;
	switch (tabpos) {
		case TABPOS_BOTTOM:
			style |= TCS_BOTTOM;
			break;
		case TABPOS_LEFT:
		case TABPOS_RIGHT:
			style |= TCS_BUTTONS | TCS_FLATBUTTONS;
	}
	SetWindowLong(hWnd_Tab, GWL_STYLE, style);

	GetClientRect(hWnd_main, &rect);
	SendMessage(hWnd_main, WM_SIZE, 0, ((rect.right - rect.left) | ((rect.bottom - rect.top) << 16))); 
	RedrawWindow(hWnd_main, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);

	//Merge LV and RTB
	for (int i = 0; i != numprofiles; i++) {
		if (bot[i]->ploaded) {
		long tmplng = GetWindowLong(bot[i]->hWnd_rtfChat, GWL_EXSTYLE);
			//tmplng ^= (-((gfstate & GFS_UIMERGERTB) != 0) ^ tmplng) & WS_EX_CLIENTEDGE;
			if (gfstate & GFS_UIMERGELVRTB)
				tmplng &= ~WS_EX_CLIENTEDGE;
			else
				tmplng |= WS_EX_CLIENTEDGE;

			if ((gfstate & GFS_USEGLASS) || bot[i]->rtbPic) {
				tmplng |= WS_EX_TRANSPARENT;
			} else {

				tmplng &= ~WS_EX_TRANSPARENT;
			}

			SetWindowLong(bot[i]->hWnd_rtfChat, GWL_EXSTYLE, tmplng);
		}

		/*tmplng = GetWindowLong(bot[i]->hWnd_lvwChannel, GWL_STYLE);
		if (gfstate & GFS_SHOWCOLUMNS)
			tmplng &= ~LVS_NOCOLUMNHEADER;
		else
			tmplng |= LVS_NOCOLUMNHEADER;
		SetWindowLong(bot[i]->hWnd_lvwChannel, GWL_STYLE, tmplng);
		*/
		//tmplng |= LVS_NOCOLUMNHEADER;
		//SetWindowLong(bot[i]->hWnd_lvwChannel, GWL_STYLE, tmplng);
		

		//ValidateRect(bot[i]->hWnd_main, &rect);
		//RedrawWindow(bot[i]->hWnd_main, NULL, NULL, 0);
		//SetWindowPos(bot[i]->hWnd_main, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////


void CreateStatWindow() { 
	TCITEM ti;
	hWnd_status = CreateMDIWindow("dsfarg", "Status Window", WS_OVERLAPPED | WS_CLIPCHILDREN, 
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, hWnd_Client, g_hInst, -1);
	if (!hWnd_status) {
		MessageBox(0, "Fatal error - Could not create status window!", 0, 0);
		return;
	}								/////
	hWnd_status_re = CreateWindowEx(WS_EX_TRANSPARENT, RICHEDIT_CLASS, NULL, 
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
		0, 0, 600, 400, hWnd_status, (HMENU)30, g_hInst, 0);
	lpfnMDIctlrtb = (WNDPROC)SetWindowLong(hWnd_status_re, GWL_WNDPROC, (LONG)MDIrtbProc);
	SendMessage(hWnd_status_re, WM_SETFONT, (WPARAM)hFont, 0);
	SendMessage(hWnd_status_re, EM_SETBKGNDCOLOR, 0, 0);
	SendMessage(hWnd_status_re, EM_SETEVENTMASK, 0, ENM_LINK); 
	SendMessage(hWnd_status_re, EM_AUTOURLDETECT, true, 0);
	ti.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
	ti.iImage = -1;
	ti.lParam = -1;
	ti.pszText = "Status Window";
	TabCtrl_InsertItem(hWnd_Tab, TabCtrl_GetItemCount(hWnd_Tab), &ti);

}


///////////////////////////////////////////////////////////////////////////////////////////////////////


void LoadFavorites() {
	char buf[64];
	*buf = 0;
	int tmp, i = 0;
	FILE *file = fopen("favorites.dat", "r");
	if (file) {
		AddItemToTree(hWnd_favTree, "Favorites", 1);
		while (!feof(file)) {
			fgets(buf, sizeof(buf), file);
			tmp = strlen(buf) - 1;
			if (buf[tmp] == 0x0A) 
				buf[tmp] = 0;
			if (*buf == 0x09) {
				AddItemToTree(hWnd_favTree, buf + 1, 3);
				if (i < 10) {
					*(int *)quickchans[i] = ' j/';
					strncpy(quickchans[i] + 3, buf + 1, 60);
				}
				i++;
			} else {
				AddItemToTree(hWnd_favTree, buf, 2);
			}
		}
		fclose(file);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////


void AddChat(COLORREF color, char *text, HWND hwnd) {
	char buf[16];
	
	if (GetWindowTextLength(hwnd) >= maxrtblen)
		ChopRTBText(hwnd);
	cfFormat.crTextColor = 0xD0D0D0;
	/*
	SCROLLINFO sci;
	sci.cbSize = sizeof(SCROLLINFO);
	sci.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
	GetScrollInfo(hwnd, SB_VERT, &sci);
	 */
	//char asdf[128];
	//sprintf(asdf, "pos %d, page %d, max %d (pos + page = %d)\n",
	//			sci.nPos, sci.nPage, sci.nMax, sci.nPos + sci.nPage);
	//AppendText(vbGreen, asdf, bot[0]->hWnd_rtfChat);

	CHARRANGE cr = {-1, -1};
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfFormat);
	SendMessage(hwnd, EM_REPLACESEL, false, (LPARAM)TimeStamp(buf));
	cfFormat.crTextColor = color;
	SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfFormat);
	SendMessage(hwnd, EM_REPLACESEL, false, (LPARAM)text);
	SendMessage(hwnd, EM_REPLACESEL, false, (LPARAM)"\r\n");
	
	//if ((unsigned int)(sci.nPos + sci.nPage) == (unsigned int)sci.nMax)
	int index = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
	if (index == -1 || !bot[index]->scrllock) //FIXME
		SendMessage(hwnd, EM_SCROLL, SB_BOTTOM, 0);
}


void AppendText(COLORREF Color, char *szFmt, HWND hwnd) {
	CHARRANGE cr = {-1, -1};
	if (GetWindowTextLength(hwnd) >= maxrtblen)
		ChopRTBText(hwnd);
	cfFormat.crTextColor = Color;
	/*
	SCROLLINFO sci;
	sci.cbSize = sizeof(SCROLLINFO);
	sci.fMask  = SIF_PAGE | SIF_POS | SIF_RANGE;
	GetScrollInfo(hwnd, SB_VERT, &sci);
	*/
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfFormat);
	SendMessage(hwnd, EM_REPLACESEL, false, (LPARAM)szFmt);
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	//if ((int)(sci.nPos + sci.nPage) == (int)sci.nMax)
	int index = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
	if (index == -1 || !bot[index]->scrllock)
		SendMessage(hwnd, EM_SCROLL, SB_BOTTOM, 0);
}


void ChopRTBText(HWND hwnd) {
	CHARRANGE cr = {0, lentoclear};
	int index = GetWindowLong(GetParent(hwnd), GWL_USERDATA);
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	if (gfstate & GFS_LOGCHAT) {
		/*GETTEXTLENGTHEX gtl;
		gtl.codepage = CP_ACP;
		gtl.flags    = GTL_USECRLF;
		i = SendMessage(bot[index]->hWnd_rtfChat, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
		if (i) {
			tmp = (char *)malloc(i + 1);
			tmp[i] = 0;
			GetWindowText(bot[index]->hWnd_rtfChat, tmp, i);
			AddLog(tmp, index);
			free(tmp);
		} */

		int len = GetWindowTextLength(hwnd);
		char *asdf = (char *)malloc(len);
		GETTEXTEX gte;
		gte.cb = len;
		gte.codepage = CP_ACP;
		gte.flags = GT_SELECTION | GT_USECRLF;
		gte.lpDefaultChar = "?";
		gte.lpUsedDefChar = NULL;
		SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)asdf);
		//SendMessage(hwnd, EM_GETSELTEXT, 0, (LPARAM)asdf);
		AddLog(asdf, index);
		free(asdf);
	}
	SendMessage(hwnd, EM_REPLACESEL, false, (LPARAM)NULL);

	if (index == -1 || !bot[index]->scrllock)
		SendMessage(hwnd, EM_SCROLL, SB_BOTTOM, 0);
}


int except_addlog(int exceptcode, LPEXCEPTION_POINTERS exceptinfo) {
	char buf[256];
	AddChatf(buf, hWnd_status_re, vbRed,
		"WARNING: Exception (code 0x%08x) occurred at 0x%08x in horizon!AddLog()",
		exceptcode, exceptinfo->ExceptionRecord->ExceptionAddress);
	return EXCEPTION_EXECUTE_HANDLER;
}


void AddLog(char *text, int index) {
	char filename[64];
	SYSTEMTIME st;
	FILE *file;
	__try {
		GetSystemTime(&st);
		sprintf(filename, "logs\\%s_%d-%d-%d.", index != -1 ? bot[index]->pname :
				 "STATUS WINDOW", st.wMonth, st.wDay, st.wYear);
		*(int *)(filename + strlen(filename)) = gfstate & GFS_RTFLOG ? 'ftr' : 'txt';
		file = fopen(filename, "a");
		if (gfstate & GFS_RTFLOG) {	
			//Feature not yet implemented!						  
		} else {
			fputs(text, file);
		}
		fclose(file);
	} __except (except_addlog(GetExceptionCode(), GetExceptionInformation())) {
		if (file)
			fclose(file);
	}
}


void AddChatf(char *buf, HWND hwnd, COLORREF clr, char *text, ...) {
	va_list val;
	va_start(val, text);
	vsprintf(buf, text, val);
	AddChat(clr, buf, hwnd);
	va_end(val);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK AboutProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	static int clrto = gradclr1, clrfrom = gradclr2;
	static HDC hdcwnd, memhdc;
	static HBITMAP hmembmp;
	static HGDIOBJ hOldBitmap;
	RECT rc;   												 
	switch (message) {
		case WM_INITDIALOG:
			hdcwnd = GetDC(hDialog);
			memhdc = CreateCompatibleDC(hdcwnd);
			GetClientRect(hDialog, &rc);
			hmembmp = CreateCompatibleBitmap(hdcwnd, rc.right, rc.bottom);
			hOldBitmap = SelectObject(memhdc, hmembmp);
			SetTimer(hDialog, 12345, 100, NULL);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ABOUT_OK)
				DestroyWindow(hDialog);
			break; 
		case WM_TIMER:
			clrto   += (0x040404) | (rand() & 0x070707);
			clrto   &= 0x00FFFFFF;
			clrfrom -= (0x040404) | (rand() & 0x070707);
			clrfrom &= 0x00FFFFFF;
			HGDIOBJ hOld;
			GetClientRect(hDialog, &rc);
			DrawGradientFill(memhdc, &rc, clrto, clrfrom);
			DrawIcon(memhdc, 10, 10, g_hIcon);
			SetBkMode(memhdc, TRANSPARENT);
			hOld = SelectObject(memhdc, hFontBig);
			SetTextColor(memhdc, vbWhite);
			TextOut(memhdc, 50, 15, szAppName, sizeof(szAppName) - 1);
			SelectObject(memhdc, hFontBold);
			SetTextColor(memhdc, ~clrfrom & 0xFFFFFF); 
			rc.left = 0;
			rc.top  = 50;
			rc.right = 220;
			rc.bottom = 220;
			DrawText(memhdc, HORIZON_ABOUTSTR, sizeof(HORIZON_ABOUTSTR) - 1, &rc, DT_CENTER);
			SelectObject(memhdc, hOld);
			RedrawWindow(hDialog, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
			break;
		case WM_PAINT:
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hDialog, &ps);
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
				ps.rcPaint.right - ps.rcPaint.left,
				ps.rcPaint.bottom - ps.rcPaint.top,
				memhdc, 0, 0, SRCCOPY);
			EndPaint(hDialog, &ps);
			break;
		case WM_DESTROY:
			hWnd_About = NULL;
			KillTimer(hDialog, 12345);
			SelectObject(memhdc, hOldBitmap);
			DeleteObject(hmembmp);
			DeleteDC(memhdc);
			ReleaseDC(hDialog, hdcwnd);
			break;
		case WM_CLOSE:
			DestroyWindow(hDialog);
			break;
		default:
			DefWindowProc(hDialog, message, wParam, lParam);
    }
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void OutputStartupStatusMessages() {
	char asdf[256];
	MEMORYSTATUS ms;
	AddChat(0xF0A0A0, "Horizon " HORIZON_VERSION " by BreW", hWnd_status_re);
	sprintf(asdf, " >> %d/%d profiles loaded.", numbots, numprofiles);
	AddChat(0xD08080, asdf, hWnd_status_re);
	GlobalMemoryStatus(&ms);
	sprintf(asdf, " >> %d/%d MB of RAM used", (ms.dwTotalPhys - ms.dwAvailPhys) >> 20, ms.dwTotalPhys >> 20);
	AddChat(0xA06060, asdf, hWnd_status_re);
}


void HorizonInitialize(HWND hwnd) {
	CLIENTCREATESTRUCT ccs;
	RECT rect;
	WSADATA wsadata;
	int i;
	
	GetCurrentDirectory(MAX_PATH, CurrDir);
	hRichEdit = LoadLibrary("riched32.dll");
	InitCommonControls();
	OleInitialize(NULL);
	wm_taskbar = RegisterWindowMessage("TaskbarCreated");
	InitGDIItems();
	GetWindowRect(hwnd, &rect);	
	//////system dependent

	hWnd_Tab = CreateWindow(WC_TABCONTROL, NULL,
		WS_CHILD | WS_VISIBLE | TCS_HOTTRACK | TCS_MULTILINE | 
		TCS_FOCUSONBUTTONDOWN | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT | TCS_BUTTONS | TCS_FLATBUTTONS, 
		0, 0, 300, 20, hwnd, (HMENU)50, g_hInst, 0);			
	SendMessage(hWnd_Tab, WM_SETFONT, (WPARAM)hFont, 0);
	hWnd_favTree = CreateWindow(WC_TREEVIEW, "Favorites", WS_CHILD | WS_BORDER | TVS_HASLINES, 
		0, 0, 150, rect.bottom, hwnd, (HMENU)53, g_hInst, 0);	
	SendMessage(hWnd_favTree, WM_SETFONT, (WPARAM)hFont, 0);
	ccs.hWindowMenu = NULL;
	ccs.idFirstChild = 200;
	hWnd_Client  = CreateWindowEx(WS_EX_CLIENTEDGE, "MDICLIENT", NULL,
		WS_CHILD | WS_CLIPCHILDREN,
		0, 100, rect.right - rect.left - 4, rect.bottom - rect.top - 24, hwnd, NULL, g_hInst, &ccs);
	ShowWindow(hWnd_Client, SW_SHOW);	
	lpfnOldClientProc = (WNDPROC)SetWindowLong(hWnd_Client, GWL_WNDPROC, (LONG)MDIClientProc);
	lpfnOldTabProc = (WNDPROC)SetWindowLong(hWnd_Tab, GWL_WNDPROC, (LONG)TabProc);

	CreateStatWindow();								     
	CheckMissingFiles();

	LoadGlobalSettings();

	InitIML();	
	for (i = 0; i != 3; i++) {
		hWnd_MDItask[i] = CreateWindow("BUTTON", buttontext[i], 
			WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, 
			rect.right + (17 * i), 0, 16, 16, hwnd, (HMENU)(90 + i), g_hInst, 0);
	}
	hWnd_Tooltip = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP, //| TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, g_hInst, NULL);
	//SendMessage(hWnd_Tooltip, TTM_SETTIPBKCOLOR, 0x000000, 0);
	//SendMessage(hWnd_Tooltip, TTM_SETTIPTEXTCOLOR, 0xFFFFFF, 0);
	//SendMessage(hWnd_Tooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 10000);
	///////////////////////////////////////////////////////////////////////////////////////////
	note.cbSize = sizeof(NOTIFYICONDATA);
	note.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
	note.hWnd = hwnd;
	note.uCallbackMessage = WM_SHELLNOTIFY;
	strcpy(note.szTip, szAppName);
	strcpy(note.szInfoTitle, "Horizon");
	note.uID = 200;
	note.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			
	LoadStatusWindowPos();
	InitalizeExternalLibs();
	InitalizeMenus();
	LoadAtmosphere();			
	LoadProfiles();
	//LoadProfileNames();
	//LoadDefaultProfiles();
	LoadFavorites();

	hWnd_QS = CreateWindowEx(0, "quiksw", NULL,
		WS_POPUP, qsrect.left, qsrect.top, 150,  30 + (numbots * 22),
		//qsrect.right - qsrect.left, qsrect.bottom - qsrect.top,
		hwnd, (HMENU)NULL, g_hInst, NULL);
	//ShowWindow(hWnd_QS, SW_SHOW);
	//SetWindowLong(hWnd_QS, GWL_EXSTYLE, WS_EX_LAYERED);
	//SetLayeredWindowAttributes(hWnd_QS, 0, 120, LWA_ALPHA);

	WSAStartup(0x0202, &wsadata);
	hook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)HookProc, NULL, GetCurrentThreadId());
	SetGlobalAttributes();
	for (i = -1; i != numprofiles; i++) {
		if (i < 0 || bot[i]->ploaded)
			LoadWindowPos(i);
	}

	//LoadBinaryImages();

	OutputStartupStatusMessages();
	//ReadBNIToBitmap("icons.bni");

	//SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	//SetLayeredWindowAttributes(hwnd, 0, 200, 2);
}


int HorizonShutdown() {
	int i;
	SaveWindowPos(-1);
	snaptopwindow = GetTopWindow(hWnd_Client);
	if (gfstate & GFS_UIWARNEXITAPP) {
		if (MessageBox(hWnd_main, "Are you sure you'd like to exit?", "Close?", MB_YESNO) == IDNO)
			return 1;
	}
	quitting = true;
	AnimateWindow(hWnd_main, 300, AW_BLEND | AW_HIDE);
	for (i = 0; i != numprofiles; i++) {
		if (bot[i]->ploaded)
			SendMessage(bot[i]->hWnd_main, WM_CLOSE, 0, 0);
	}
	if (hWnd_status) {
		SaveStatusWindowPos();
		DestroyWindow(hWnd_status);
	}
	DestroyWindow(hWnd_main);
	DeleteObject(hFont);
	DeleteObject(hFontBold);
	DeleteObject(hFontBig);
	//DeleteObject(hPen);
	FreeExternalLibs();
	FreeLibrary(hRichEdit);
	OleUninitialize();
	UnhookWindowsHookEx(hook);
	
	// this is a disaster waiting to happen...
	// the low 16 bits of the handle on load are stripped
	for (i = 0; i != NUM_LOADED_MODULES; i++) {
		if (lpModuleImage[i])
			FreeLibrary((HMODULE)lpModuleImage[i]);
	}
	return 0; 
}


void MoveMainWindowControls(LPARAM lParam) {
	int tabx, taby, tcx, tcy;
	int clientx, clienty, ccx, ccy;
	int i;
	switch (tabpos) {
		case TABPOS_BOTTOM:
			tabx = 0;
			taby = HIWORD(lParam) - 20;
			tcx  = LOWORD(lParam);
			tcy  = 20;
			ccx  = LOWORD(lParam);
			ccy  = HIWORD(lParam) - 20;
			clientx = 0;
			clienty = 0;
			break;
		case TABPOS_LEFT:
			tabx = 0;
			taby = 0;
			tcx  = 80;
			tcy  = HIWORD(lParam);
			ccx  = LOWORD(lParam) - 80;
			ccy  = HIWORD(lParam);
			clientx = 80;
			clienty = 0;
			break;
		case TABPOS_RIGHT:
			tabx = LOWORD(lParam) - 80;
			taby = 0;
			tcx  = 80;
			tcy  = HIWORD(lParam);
			ccx  = LOWORD(lParam) - 80;
			ccy  = HIWORD(lParam);
			clientx = 0;
			clienty = 0;
			break;
		default:
			tabx = 0;
			taby = 0;
			tcx  = LOWORD(lParam);
			tcy  = 20;
			ccx  = LOWORD(lParam);
			ccy  = HIWORD(lParam) - 20;
			clientx = 0;
			clienty = 20;
	}
	MoveWindow(hWnd_Tab, favexpanded ? tabx + 150 : tabx, taby, favexpanded ? tcx - 150 : tcx, tcy, false);
	MoveWindow(hWnd_Client, favexpanded ? clientx + 150 : clientx, clienty, favexpanded ? ccx - 150 : ccx, ccy, false);
	MoveWindow(hWnd_favTree, 0, 0, 150, HIWORD(lParam), false);
	//MoveWindow(hWnd_Tab, 0, /*gfstate & GFS_UITABBOTTOM ? HIWORD(lParam) - 20 :*/ 0,
	//	LOWORD(lParam), 20, true);
	//MoveWindow(hWnd_Client, 0, /*gfstate & GFS_UITABBOTTOM ? 0 :*/ 20,
	//	LOWORD(lParam), HIWORD(lParam) - 20, true);
	for (i = 0; i != 3; i++)
		MoveWindow(hWnd_MDItask[i],
		(tabpos == TABPOS_LEFT ? 0 : LOWORD(lParam) - 53) + (i * 17),
		tabpos ? HIWORD(lParam) - 18 : 0, 16, 16, false);
}

	   
///////////////////////////////////////////////////////////////////////////////////////////////////////


void Send0x0E(char *text, int index) {
	char asdf[32], *sdfg;
	if (*text) {
		if (bot[index]->connected) {
			InsertNTString(text);
			SendPacket(0x0E, index);
			if (*text != '/') {
				AppendText(0xD0D0D0, TimeStamp(asdf), bot[index]->hWnd_rtfChat);
				sprintf(asdf, "<%s> ", bot[index]->realname);
				AppendText(vbCyan, asdf, bot[index]->hWnd_rtfChat);
				if (bot[index]->self) {
					bot[index]->self->lastspoke = GetTickCount();
					bot[index]->self->speak++;
				}
				sdfg = text;
				while (*sdfg++);
				*(unsigned short *)(sdfg - 1) = '\n';
				AppendText(vbWhite, text, bot[index]->hWnd_rtfChat);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////


long __stdcall ExceptionHandler(struct _EXCEPTION_POINTERS *pExceptInfo) {
	char asdf[128], sdfg[64];
	MINIDUMP_EXCEPTION_INFORMATION mdei;
	GetTempFileName(".", "err", 0, sdfg + 5);
	DeleteFile(sdfg + 5);

	*(int *)sdfg = 'orre';
	*(int *)(sdfg + 4) = 'e\\sr';
	*(int *)(sdfg + strlen(sdfg) - 4) = 'pmd.';	
	HANDLE hFile = CreateFile(sdfg, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
						CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
		return EXCEPTION_EXECUTE_HANDLER;

	mdei.ClientPointers	   = NULL;
	mdei.ExceptionPointers = pExceptInfo;
	mdei.ThreadId		   = GetCurrentThreadId();
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
		MiniDumpNormal, &mdei, NULL, NULL);
	CloseHandle(hFile);
	
	sprintf(asdf, "Exception code 0x%08x occurred at 0x%08x, saved dump to %s,"
		" continue execution? (potentially dangerous!)",
		pExceptInfo->ExceptionRecord->ExceptionCode,
		pExceptInfo->ExceptionRecord->ExceptionAddress,
		sdfg);
	if (MessageBox(NULL, asdf, NULL, MB_YESNO | MB_ICONERROR) == IDNO)
		FatalAppExit(0, "Exiting now.");
	//return EXCEPTION_EXECUTE_HANDLER;									  
	return EXCEPTION_CONTINUE_EXECUTION;
}

#if 0
void my_free(void *mem, const char *filename, int line) {
	FILE *file = fopen("DEBUGLOG.LOG", "a");
	if (!file) {
		AddChat(vbRed, "Couldn't open DEBUGLOG.LOG for append!", hWnd_status_re);
		delete mem;
		return;
	}
	fprintf(file, "free(0x%08x) at %s %d\n", mem, filename, line);
	fclose(file);
	free(mem);
}
#endif

