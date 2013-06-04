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
#include "hashtable.h"
#include "resource.h"
#include "connection.h"
#include "config.h"
#include "botprofiles.h"

#pragma warning(disable : 4805)
#define NUM_LFKEYS 8

char *lfkeys[] = {
	"UsePlug",	
	"ForceCreate",	
 	"UseJoinGreet",	
	"UseLeaveGreet",
	"AutoRegEmail",	
	"ChangePass",
	"ChangeEmail",
	"UseProxy"	
};


///////////////////////////////////////////////////////////////////////////////


void LoadProfiles() {
	WIN32_FIND_DATA ffdata;
	char *sdfg, blah[8];
	int expnum = 8;
	numprofiles = 0;
	HANDLE hFile = FindFirstFile("profiles\\*.ini", &ffdata);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	bot = (LPBOT *)malloc(expnum * sizeof(LPBOT));
	do {
		sdfg = revstrchr(ffdata.cFileName, '.');
		if (!sdfg)
			continue;
		*sdfg = 0;
		bot[numprofiles] = (LPBOT)malloc(40);
		strncpy(bot[numprofiles]->pname, ffdata.cFileName, sizeof(bot[numprofiles]->pname));
		bot[numprofiles]->ploaded = false;
		bot[numprofiles]->ploadonstart = (*GetStuff(bot[numprofiles]->pname,
			"Main", "LoadOnStartup", sizeof(blah), blah) == 0x31);
		if (bot[numprofiles]->ploadonstart)
			LoadProfile(numprofiles);
		numprofiles++;
		if (numprofiles == expnum) {
			expnum <<= 1;
			bot = (LPBOT *)realloc(bot, expnum * sizeof(LPBOT));
		}
	} while (FindNextFile(hFile, &ffdata));
	bot = (LPBOT *)realloc(bot, numprofiles * sizeof(LPBOT));
	FindClose(hFile);
}


void LoadProfile(int i) {
	char asdf[64];
	MENUITEMINFO mii;
	TCITEM ti;
	if (bot[i]->ploaded)
		return;

	numbots++;
	
	bot[i] = (LPBOT)realloc(bot[i], sizeof(BOT));
	ZeroMemory((char *)(((int)bot[i]) + 40), sizeof(BOT) - 40);	

	//struct assignment
	AssignStructFromConfig(i, bot[i]->pname);
	//bot[i]->pnamehash = hash((unsigned char *)profilename, -1);
	bot[i]->lvwidth = 200;
	bot[i]->wsdata = bot[i]->wsbuffer;

	bot[i]->users = (LPCHAIN *)malloc(sizeof(void *) * 1024);
	ZeroMemory(bot[i]->users, sizeof(void *) * 1024);
	
	//profiles[GetProfileFromBot(i)]->loaded = true;
	bot[i]->ploaded = true;
	//gui
	
	ti.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM; 
	ti.iImage = 0;						
	ti.pszText = bot[i]->pname;
	ti.lParam = i;
	int tabindex = TabCtrl_GetItemCount(hWnd_Tab);
	TabCtrl_InsertItem(hWnd_Tab, tabindex, &ti);
	bot[i]->tabindex = tabindex;
	
	bot[i]->hWnd_main = CreateMDIWindow("dsfarg", bot[i]->pname, WS_OVERLAPPED | WS_CLIPCHILDREN, 
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, hWnd_Client, g_hInst, i);
	ShowWindow(bot[i]->hWnd_main, SW_SHOW);
	sprintf(asdf, "Profile %d (%s) loaded!", i, bot[i]->pname);
	AddChat(0x70F050, asdf, bot[i]->hWnd_rtfChat);
	sprintf(asdf, "%d: %s", i, bot[i]->pname);
	////////////////////////////////////////////////////////////////
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.wID = 11000 + i;
	mii.fMask = MIIM_ID | MIIM_STRING;
	mii.fType = MFT_STRING;
	mii.fState = MFS_DEFAULT;
	mii.dwTypeData = asdf;			                                   
	mii.cch = strlen(asdf);						 
	InsertMenuItem(hMenu_windows_popup, 4 + i, true, &mii);

	//AppendMenu(hMenu_windows_popup, MF_STRING, 11040, asdf);
	////////////////////////////////////////////////////////////////

	InsertMenuItem(hMenu_main_popup, GetMenuItemCount(hMenu_main_popup) - 2, true, &mii);
}


void AssignStructFromConfig(int index, char *profilename) {
	char asdf[32];
	GetStuff(profilename, "Main", "Username", 64, bot[index]->username);
	GetStuff(profilename, "Main", "Password", 64, bot[index]->password);
	GetStuff(profilename, "Main", "Server", 32, bot[index]->server);
	GetStuff(profilename, "Main", "CDKey", 32, bot[index]->cdkey);
	GetStuff(profilename, "Main", "ExpKey", 32, bot[index]->expkey);
	GetStuff(profilename, "Main", "CDKeyOwner", 32, bot[index]->cdkeyowner);
	GetStuff(profilename, "Main", "BNLSServer", 32, bot[index]->bnlsserver); 
	GetStuff(profilename, "Main", "Home", 32, bot[index]->homechannel);
	GetStuff(profilename, "Main", "Proxy", 64, bot[index]->proxy);
	GetStuff(profilename, "Main", "NewPass", 64, bot[index]->newpass);
	GetStuff(profilename, "Main", "Email", 64, bot[index]->email1);
	GetStuff(profilename, "Main", "NewEmail", 64, bot[index]->email2);
	GetStuff(profilename, "GnI", "JoinGreet", 64, bot[index]->greetmsg);
	GetStuff(profilename, "GnI", "LeaveGreet", 64, bot[index]->leavemsg);
	GetStuff(profilename, "GnI", "IdleMsg", 64, bot[index]->idlemsg);
	GetStuff(profilename, "Main", "CRType", 8, asdf);
	bot[index]->crtype = atoi(asdf);
	GetStuff(profilename, "Main", "PingType", 8, asdf);
	bot[index]->pingtype = atoi(asdf);
	GetStuff(profilename, "GnI", "IdleType", 8, asdf);
	bot[index]->idletype = atoi(asdf);
	GetStuff(profilename, "Main", "Ping", 8, asdf);
	bot[index]->spoofedping = atoi(asdf);
	GetStuff(profilename, "GnI", "IdleInterval", 8, asdf);
	bot[index]->idleinterval = atoi(asdf);
	GetStuff(profilename, "Main", "Client", 8, asdf);
	strncpy(bot[index]->clientstr, asdf, 4);
	fastswap32((unsigned __int32 *)asdf);
	bot[index]->client = *(unsigned __int32 *)asdf;
	GetStuff(profilename, "Main", "Platform", 8, asdf);
	fastswap32((unsigned __int32 *)asdf);	   
	bot[index]->platform = *(unsigned __int32 *)asdf;
	sscanf(ucase(GetStuffConfig("Verbytes", bot[index]->clientstr, asdf)), "%X", &bot[index]->verbyte);
	bot[index]->cdkeylen = strlen(bot[index]->cdkey);
	bot[index]->fstate &= bot[index]->connected ? ~BFS_STATICATTRIBUTEMASK : 0;
	for (int i = 0; i != NUM_LFKEYS; i++) {
		GetStuff(profilename, "Main", lfkeys[i], sizeof(asdf), asdf);
		if (*asdf - 0x30)
			bot[index]->fstate |= (1 << i);
	}

	/*if (bot[index]->client == 'STAR' || bot[index]->client == 'SEXP')
		bot[index]->fstate |= BFS_STARCRAFT;
	else if (bot[index]->client == 'D2DV' || bot[index]->client == 'D2XP')
		bot[index]->fstate |= BFS_DIABLO2;
	else if (bot[index]->client == 'WAR3' || bot[index]->client == 'W3XP')
		bot[index]->fstate |= BFS_WARCRAFT3;	*/
	bot[index]->loadedtick = GetTickCount();
}


void UnloadProfile(int i) {
	if (bot[i]->ploaded) {
		char asdf[64];
		KillTimer(hWnd_main, i + 1);
		KillTimer(bot[i]->hWnd_main, i | 0x4000);
		if (bot[i]->hWnd_main)
			DestroyWindow(bot[i]->hWnd_main);
		bot[i]->ploaded = false;
		numbots--;

		RemoveMenu(hMenu_main_popup, 11000 + i, MF_BYCOMMAND);
		RemoveMenu(hMenu_windows_popup, 11000 + i, MF_BYCOMMAND);
		
		TabCtrl_DeleteItem(hWnd_Tab, GetTabFromIndex(i));
		ResetTableContents(bot[i]->users, TABLESIZE_USERS);

		free(bot[i]->users);
		bot[i] = (LPBOT)realloc(bot[i], 40);
		AddChatf(asdf, hWnd_status_re, vbRDBlue, "Profile %d (%s) unloaded.", i, bot[i]->pname);
	}
}


//void LoadDefaultProfiles() {
//	for (int i = 0; i != numprofiles; i++) {
//		if (profiles[i]->defload)
//			LoadProfile(profiles[i]->name);
//	}
	/*char asdf[256] = {0}, *str;
	GetStuffConfig("Main", "ProfilesToLoad", asdf);
	int len = strlen(asdf);
	for (int i = 0; i != len; i++) {
		if (asdf[i] == ',')
			asdf[i] = 0;
	}
	str = asdf;
	do {
		LoadProfile(str);
		str += strlen(str) + 1;
	} while (*str);	*/
//}


void SaveStatusWindowPos() {
	RECT rect;
	POINT point;
	char asdf[64];
	GetWindowRect(hWnd_status, &rect);				
	point.x = rect.left;
	point.y = rect.top;
	ScreenToClient(hWnd_Client, &point);
	rect.right  -= (rect.left - point.x);
	rect.bottom -= (rect.top - point.y);
	rect.left = point.x;
	rect.top  = point.y;
	if (rect.left < 0)
		rect.left = 0;
	if (rect.top < 0)
		rect.top = 0;
	sprintf(asdf, "%d, %d, %d, %d", rect.left, rect.top, rect.right, rect.bottom);
	WriteStuffConfig("Interface", "StatusWndPos", asdf);
}


void LoadStatusWindowPos() {
	RECT rect;
	char asdf[64];
	GetStuffConfig("Interface", "StatusWndPos", asdf);
	if (*asdf) {
		sscanf(asdf, "%d, %d, %d, %d", &rect.left, &rect.top, &rect.right, &rect.bottom);
	} else {
		rect.left = 0;
		rect.top = 0;
		rect.right = 480;
		rect.bottom = 300;
	}	
	MoveWindow(hWnd_status, rect.left, rect.top, rect.right - rect.left,
		rect.bottom - rect.top, true);
}


void SaveWindowPos(int index) {
	char asdf[64];
	RECT rect;
	HWND hwndtmp = index == -1 ? hWnd_main : bot[index]->hWnd_main;
	bool maximized = IsZoomed(hwndtmp);
	if (!maximized)	{
		GetWindowRect(hwndtmp, &rect);
		/*GetWindowRect(hWnd_Client, &r2);
		rect.left   -= r2.left;
		rect.right  -= r2.left;
		rect.top    -= r2.top;
		rect.bottom -= r2.top;*/
		if (index != -1) {
			POINT point;
			point.x = rect.left;
			point.y = rect.top;
			ScreenToClient(hWnd_Client, &point);
			rect.right  -= (rect.left - point.x);
			rect.bottom -= (rect.top - point.y);
			rect.left = point.x;
			rect.top  = point.y;
		}
		if (rect.left < 0)
			rect.left = 0;
		if (rect.top < 0)
			rect.top = 0;
		sprintf(asdf, "%d, %d, %d, %d", rect.left, rect.top, rect.right, rect.bottom);
	}
	short sdfg = 0x30 + ((((index != -1) && (snaptopwindow == bot[index]->hWnd_main)) << 1) | maximized); 
	sdfg &= 0xFF; 
	if (index != -1) {
		if (!maximized)
			WriteStuff(bot[index]->pname, "Interface", "WndPos", asdf);
		WriteStuff(bot[index]->pname, "Interface", "WndState", (char *)&sdfg);
	} else {
		if (!maximized)
			WriteStuffConfig("Interface", "WndPos", asdf);
		WriteStuffConfig("Interface", "WndState", (char *)&sdfg);
	}
}


void LoadWindowPos(int index) {
	char asdf[256], sdfg[256];
	RECT rect;
	HWND hwndtmp;
	if (index != -1) {
		hwndtmp = bot[index]->hWnd_main;
		GetStuff(bot[index]->pname, "Interface", "WndPos", sizeof(asdf), asdf);
		GetStuff(bot[index]->pname, "Interface", "WndState", sizeof(sdfg), sdfg);
	} else {
		hwndtmp = hWnd_main;
		GetStuffConfig("Interface", "WndPos", asdf);
		GetStuffConfig("Interface", "WndState", sdfg);
	}
	if (*asdf) {
		sscanf(asdf, "%d, %d, %d, %d", &rect.left, &rect.top, &rect.right, &rect.bottom);
	} else {
		rect.left = 0;
		rect.top = 0;
		if (index != -1) {
			rect.right = 480;
			rect.bottom = 300;
		} else {
			rect.right = 800;
		}	rect.bottom = 600;
	}		
	MoveWindow(hwndtmp, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
	if (*sdfg) {
		int tmp = *sdfg - 0x30;
		if (tmp & 0x01)
			ShowWindow(hwndtmp, SW_MAXIMIZE);
		if (tmp & 0x02)
			BringWindowToTop(hwndtmp);
	}
}


#if 0
void LoadProfileNames() {
	WIN32_FIND_DATA ffdata;
	char *sdfg, blah[8];
	if (profiles) {
		for (int i = 0; i != numprofiles; i++)
			free(profiles[i]);
		free(profiles);
		profiles = NULL;
	} 

	numprofiles = 0;
	do {
		sdfg = revstrchr(ffdata.cFileName, '.');
		*sdfg = 0;
		profiles = (LPPROFILES *)realloc(profiles, (numprofiles + 1) * sizeof(LPPROFILES));
		profiles[numprofiles] = (LPPROFILES)malloc(sizeof(PROFILES));
		strncpy(profiles[numprofiles]->name, ffdata.cFileName, 64);
		profiles[numprofiles]->hash = hash((unsigned char *)profiles[numprofiles]->name, -1);
		profiles[numprofiles]->loaded = false;
		profiles[numprofiles]->defload = (*GetStuff(profiles[numprofiles]->name,
			"Main", "LoadOnStartup", sizeof(blah), blah) == 0x31);
		numprofiles++; 
	} while (FindNextFile(hFile, &ffdata));
	FindClose(hFile);
}
#endif


void ProfilePopupMenu(HWND lvwhwnd, int index) {				
	POINT point;
	MENUITEMINFO mii;
	GetCursorPos(&point);
	mii.cbSize = sizeof(MENUITEMINFO);
	if (index == -1)
		return;
	if (!bot[index]->ploaded) {	
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = "Load";
		SetMenuItemInfo(hMenu_lvwBots, 10090, false, &mii);
		mii.fState = MFS_DISABLED; 
	} else {
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = "Unload";
		SetMenuItemInfo(hMenu_lvwBots, 10090, false, &mii);	
		mii.fState = MFS_ENABLED; 
	}
	mii.fMask = MIIM_STATE;	
	SetMenuItemInfo(hMenu_lvwBots, 10091, false, &mii);
	SetMenuItemInfo(hMenu_lvwBots, 10093, false, &mii);
	if (bot[index]->ploaded) {
		mii.fMask |= MIIM_STRING;
		mii.dwTypeData = bot[index]->connected ? "Disconnect" : "Connect";	
	}
	SetMenuItemInfo(hMenu_lvwBots, 10092, false, &mii);
	int i = TrackPopupMenuEx(hMenu_lvwBots,
		TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
		point.x, point.y, hWnd_main, NULL);

	switch (i) {
		case 10090: //Load/Unload profile 
			if (lvwhwnd) {
				if (bot[index]->ploaded)
					UnloadProfile(index);
				else
					LoadProfile(index);
					//LoadProfile(GetItemTextLt(lvwhwnd, index, buf, sizeof(buf)));
			} else {
				UnloadProfile(index);
			}
			break;
		case 10091:	//Reload Settings
			//if (lvwhwnd)
			//	AssignStructFromConfig(effprof, bot[effprof]->profilename);
			//else 
				AssignStructFromConfig(index, bot[index]->pname);
			//break;
		case 10092: //Connect/Disconnect
			//effprof = lvwhwnd ? GetBotFromProfile(index) : index;
			if (bot[index]->connected)
				DisconnectProfile(index, true);
			else
				ConnectProfile(index);
			break;
		case 10093: //Reconnect
			//effprof = lvwhwnd ? GetBotFromProfile(index) : index;
			DisconnectProfile(index, false);
			ConnectProfile(index);
			break;
		case 10094: //Configure
			if (!hWnd_Config) {
				//effprof = lvwhwnd ? index : GetProfileFromBot(index);
				hWnd_Config = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONFIG),
					hWnd_main, (DLGPROC)ConfigProc, (LPARAM)index);
				ShowWindow(hWnd_Config, SW_SHOW);
			}
	}
}


#if 0
void AddToLoad(char *profilename) {
	char asdf[256];
	GetStuffConfig("Main", "ProfilesToLoad", asdf);
	strcat(asdf, profilename);
	*(short *)(asdf + strlen(asdf)) = ',';
	WriteStuffConfig("Main", "ProfilesToLoad", asdf);
}


void RemoveToLoad(char *profilename) {
	char asdf[256], sdfg[256] = {0}, *str = asdf, *tmp;
	GetStuffConfig("Main", "ProfilesToLoad", asdf);
	while (str) {
		tmp = strchr(str, ',');
		if (tmp) {
			*tmp++ = 0;
			if (strcmp(str, profilename)) {
				strcat(sdfg, str);
				*(short *)(sdfg + strlen(sdfg)) = ',';
			}
			str = tmp;
		}																	  
	}
	WriteStuffConfig("Main", "ProfilesToLoad", sdfg);
}
#endif

