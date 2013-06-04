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
#include "packetbuffer.h"
#include "fxns.h"
#include "profile.h"

char *accinfokeys[] = { 
	"Account Created",
	"Last Logon",
	"Last Logoff"
};


///////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK ProfileProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	int index;
	switch (message) {
		case WM_INITDIALOG:
			LPPROFILEINIT lppi;
			lppi = (LPPROFILEINIT)lParam;
			SetWindowText(GetDlgItem(hDialog, PROFILE_TXTDESCRIPTION), "loading...");
			SetWindowText(GetDlgItem(hDialog, PROFILE_LBLUSERNAME), lppi->username);
			SendMessage(GetDlgItem(hDialog, PROFILE_LBLUSERNAME), WM_SETFONT, (WPARAM)hFontBig, 0);
			if (bot[((LPPROFILEINIT)lParam)->index]->connected)
				Send0x26((char *)lppi->username, lppi->index);
			if (!stricmp_(bot[lppi->index]->realname, lppi->username))
				EnableWindow(GetDlgItem(hDialog, PROFILE_CMDOK), true);
			SetWindowLong(hDialog, GWL_USERDATA, lppi->index);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case PROFILE_CMDOK:
					index = GetWindowLong(hDialog, GWL_USERDATA);
					Send0x27(index);
					AddChat(vbGreen, "[BNET] Updated profile!", bot[index]->hWnd_rtfChat);
				case PROFILE_CMDCANCEL:
					DestroyWindow(hDialog);
			}	        
			break;
		case WM_DESTROY:
			hWnd_Profile = NULL;
			break;
		case WM_CLOSE:
			DestroyWindow(hDialog);
			break;
		default:
			DefWindowProc(hDialog, message, wParam, lParam);
    }
	return false;
}


void Send0x26(char *username, int index) {
	InsertDWORD(0x01);
	InsertDWORD(0x03);
	InsertDWORD(0x3713);
	InsertNTString(username);
	InsertNTString("profile\\sex");
	InsertNTString("profile\\location");
	InsertNTString("profile\\description");
	SendPacket(0x26, index);				  
}

									 
void Send0x26AccInfo(int index) {
	InsertDWORD(0x01);
	InsertDWORD(0x04);
	InsertDWORD(0x3713);
	InsertNTString(bot[index]->realname);
	InsertNTString("System\\Account Created");
	InsertNTString("System\\Last Logon");
	InsertNTString("System\\Last Logoff");
	InsertNTString("System\\Time Logged");
	SendPacket(0x26, index);				  
}


void Parse0x26(char *data, int index) {
	char *current = data + 16;
	if (bot[index]->fstate & BFS_REQACCINFO) {
		char buf[128];
		SYSTEMTIME st;
		FILETIME ft;
		bot[index]->fstate &= ~BFS_REQACCINFO;
		for (int i = 0; i != 3; i++) {
			char *tmp = strchr(current, ' ');
			if (tmp) {
				*tmp++ = 0;
				ft.dwHighDateTime = atol(current);
				ft.dwLowDateTime  = atol(tmp);
				FileTimeToSystemTime(&ft, &st);
				sprintf(buf, "%s: %d/%d/%d %d:%02d:%02d.%04d",
					accinfokeys[i], st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute,
					st.wSecond, st.wSecond, st.wMilliseconds);
				AddChat(vbRDBlue, buf, bot[index]->hWnd_rtfChat);
				current += strlen(current) + strlen(tmp) + 2;
			}
		}
		*(int *)buf       = 'emiT';
		*(int *)(buf + 4) = 'goL ';
		*(int *)(buf + 8) = ':deg';
		*(int *)(buf + 12) = ' ';
		GetLngUptimeString(atol(current), buf + 13);
		AddChat(vbRDBlue, buf, bot[index]->hWnd_rtfChat);
	} else {
		SetWindowText(GetDlgItem(hWnd_Profile, PROFILE_TXTSEX), current);
		current += strlen(current) + 1;
		SetWindowText(GetDlgItem(hWnd_Profile, PROFILE_TXTLOCATION), current);
		current += strlen(current) + 1;
		SetWindowText(GetDlgItem(hWnd_Profile, PROFILE_TXTDESCRIPTION), current);
	}
}


void Send0x27(int index) {
	char asdf[512];
	InsertDWORD(0x01);
	InsertDWORD(0x03);
	InsertByte(0x00);
	InsertNTString("profile\\sex");
	InsertNTString("profile\\location");
	InsertNTString("profile\\description");
	GetWindowText(GetDlgItem(hWnd_Profile, PROFILE_TXTSEX), asdf, sizeof(asdf));
	InsertNTString(asdf);
	GetWindowText(GetDlgItem(hWnd_Profile, PROFILE_TXTLOCATION), asdf, sizeof(asdf));
	InsertNTString(asdf);
	GetWindowText(GetDlgItem(hWnd_Profile, PROFILE_TXTDESCRIPTION), asdf, sizeof(asdf));
	InsertNTString(asdf);
	SendPacket(0x27, index);
}
						
