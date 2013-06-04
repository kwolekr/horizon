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
#include "fxns.h"
#include "resource.h"
#include "botprofiles.h"
#include "botdisplay.h"

#define NUM_COLUMNNAMES 8

char *columntxt[] = {
	"Profile",
	"Battle.net username",
	"Server",
	"Channel",
	"Flags",
	"Ping",
	"Send",
	"Recv"
};


///////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK BotProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	int i;
	HWND tmphwnd;
	switch (message) {
		case WM_INITDIALOG:
			LVCOLUMNA lvcolumn;
			lvcolumn.mask = LVCF_TEXT;
			tmphwnd = GetDlgItem(hDialog, BOTS_LVWDISPLAY);
			for (i = 0; i != NUM_COLUMNNAMES; i++) {
				lvcolumn.pszText = columntxt[i];
				SendMessage(tmphwnd, LVM_INSERTCOLUMN, i, (LPARAM)&lvcolumn);
				SendMessage(tmphwnd, LVM_SETCOLUMNWIDTH, i, (LPARAM)i < 4 ? 125 : 50);
			}
			PopulateBotInfoLV(hDialog, tmphwnd);
			SendMessage(tmphwnd, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)lvwIML);
			SetTimer(hDialog, 1, 5000, UpdateStatusLVTimerProc); 
			break;
		case WM_NOTIFY:
			if (LOWORD(wParam) == BOTS_LVWDISPLAY) {
				if (((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
					HandleDisplayLVCustDraw(hDialog, (LPNMLVCUSTOMDRAW)lParam);
					return true;
				} else if (((LPNMHDR)lParam)->code == NM_RCLICK) {
					if (((LPNMITEMACTIVATE)lParam)->iItem != -1)
						ProfilePopupMenu(GetDlgItem(hDialog, BOTS_LVWDISPLAY), ((LPNMITEMACTIVATE)lParam)->iItem);
				}
			}
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == BOTS_CMDOK)
				DestroyWindow(hDialog);	        
			break; 
		case WM_DESTROY:
			hWnd_Bots = NULL;
			tmphwnd = GetDlgItem(hDialog, BOTS_LVWDISPLAY);
			SendMessage(tmphwnd, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)NULL);
			KillTimer(hDialog, 1);
			for (i = 0; i != SendMessage(tmphwnd, LVM_GETITEMCOUNT, 0, 0); i++)
				free((void *)GetItemlParam(i, tmphwnd));
			break;
		case WM_CLOSE:
			DestroyWindow(hDialog);
			break;
		default:
			DefWindowProc(hDialog, message, wParam, lParam);
    }
	return false;
}	 


void PopulateBotInfoLV(HWND hDialog, HWND hwndlvw) {
	LVITEMA lviItem;
	char asdf[64];
	for (int i = 0; i != numprofiles; i++) {
		bool tloaded = bot[i]->ploaded;
		LPLVCOLOR lpColor = (LPLVCOLOR)malloc(sizeof(LPLVCOLOR));
		lpColor->cItem1 = tloaded ? vbWhite : 0x505050;
		lviItem.mask =  LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
		lviItem.iItem = i;
		lviItem.iSubItem = 0;
		lviItem.pszText = bot[i]->pname;
		lviItem.lParam = (long)lpColor;
		lviItem.iImage = (tloaded && bot[i]->connected) ? GetIcon(bot[i]->client, 0) : -1;
		//if (sdfg2 && bot[i]->connected)
		//	lviItem.iImage = GetIcon(bot[i]->client, 0);
		//else 
		//	lviItem.iImage = -1;
		SendMessage(hwndlvw, LVM_INSERTITEM, 0, (LPARAM)&lviItem);
		if (tloaded) {
			if (bot[i]->connected) {
				lviItem.mask = LVIF_TEXT;
				lviItem.pszText = bot[i]->realname;
				lviItem.iItem = i;
				lviItem.iSubItem = 1;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lviItem);
				lviItem.iSubItem++;
				lviItem.pszText = bot[i]->server;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lviItem);
				lviItem.iSubItem++;
				lviItem.pszText = bot[i]->currentchannel;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lviItem);
				lviItem.iSubItem++;
				sprintf(asdf, "0x%02x", bot[i]->bnetflags);
				lviItem.pszText = asdf;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lviItem);
				lviItem.iSubItem++;
				sprintf(asdf, "%d", bot[i]->ping);
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lviItem);
				lviItem.iSubItem++;
				sprintf(asdf, "%d", bot[i]->sendcount);
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lviItem);
				lviItem.iSubItem++;
				sprintf(asdf, "%d", bot[i]->recvcount);
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lviItem);
			}
		}
	}
}


void HandleDisplayLVCustDraw(HWND hDialog, LPNMLVCUSTOMDRAW nmCustDraw) {
	int tmp;
	switch (nmCustDraw->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			tmp = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:		
			tmp = CDRF_NOTIFYSUBITEMDRAW;
			break;
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			LPLVCOLOR lplvColor;
			lplvColor = (LPLVCOLOR)nmCustDraw->nmcd.lItemlParam; 
			nmCustDraw->clrText = lplvColor->cItem1;
			nmCustDraw->clrTextBk = -1;
			tmp = CDRF_NEWFONT;
			break;
		default:
			tmp = CDRF_DODEFAULT;
	}
	SetWindowLong(hDialog, DWL_MSGRESULT, tmp);
}


void CALLBACK UpdateStatusLVTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	char asdfg[64];
	LVITEM lvi;
	HWND hwndlvw = GetDlgItem(hwnd, BOTS_LVWDISPLAY);
	for (int i = 0; i != numprofiles; i++) {
		LPLVCOLOR lplvColor = (LPLVCOLOR)GetItemlParam(i, hwndlvw);
		if (bot[i]->ploaded) {
			lvi.iItem = i;
			if (lplvColor->cItem1 = 0x505050)
				lplvColor->cItem1 = vbWhite;
			if (bot[i]->connected) {
				lvi.mask = LVIF_IMAGE;
				lvi.iSubItem = 0;
				SendMessage(hwndlvw, LVM_GETITEM, 0, (LPARAM)&lvi);
				if (lvi.iImage == -1) {
					lvi.iImage = GetIcon(bot[i]->client, 0);
					SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
				}
				lvi.mask = LVIF_TEXT;
				lvi.pszText = bot[i]->realname;
				lvi.iSubItem = 1;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
				lvi.iSubItem++;
				lvi.pszText = bot[i]->server;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
				lvi.iSubItem++;
				lvi.pszText = bot[i]->currentchannel;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
				lvi.iSubItem++;
				sprintf(asdfg, "0x%02x", bot[i]->bnetflags);
				lvi.pszText = asdfg;
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
				lvi.iSubItem++;
				sprintf(asdfg, "%d", bot[i]->ping);
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
				lvi.iSubItem++;
				sprintf(asdfg, "%d", bot[i]->sendcount);
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
				lvi.iSubItem++;
				sprintf(asdfg, "%d", bot[i]->recvcount);									   
				SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
			} else {
				lvi.mask = LVIF_IMAGE;
				lvi.iSubItem = 0;
				SendMessage(hwndlvw, LVM_GETITEM, 0, (LPARAM)&lvi);
				if (lvi.iImage) {
					lvi.mask = LVIF_IMAGE;
					lvi.iImage = -1;
					SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
					lvi.mask = LVIF_TEXT;
					lvi.pszText = NULL;
					for (int i2 = 0; i2 != 7; i2++) {
						lvi.iSubItem++;
						SendMessage(hwndlvw, LVM_SETITEM, 0, (LPARAM)&lvi);
					}
				}
			}
		} else {
			if (lplvColor->cItem1 = vbWhite)
				lplvColor->cItem1 = 0x505050;
		}
	}
}

