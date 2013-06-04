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
#include "hashing.h"
#include "packetbuffer.h"
#include "resource.h"
#include "hashtable.h"
#include "loaddll.h"
#include "packets.h"	
#include "warden.h"
#include "winamp.h"
#include "gui.h"
#include "fxns.h"

char *eastsvr[] = {
	"USEast",
	"Azeroth"
};

char *westsvr[] = {
	"USWest",					
	"Lordaeron"
};

char *eurosvr[] = {
	"Europe",
	"Northrend"
};

char *asiasvr[] = {
	"Asia",
	"Kalimdor"
};

char *friendstati[] = {
	"Mutual",
	"DND",
	"Away"
};

char *filestocheck[] = {
	"Errors",
	"Logs",
	"Modules",
	"Profiles",

	"config.ini",
	"CDKeys.txt",
	"channels.atm",
	"favorites.dat",
	
	"notepad.txt",
	"Proxies.txt",
	"Servers.txt"
};


///////////////////////////////////////////////////////////////////////////////


void strcpychr(char *dest, char *src, int c) {
	while ((*dest++ = *src++) != c);	
}


char *ucase(char *str) {
	char *tmp = str;
	while (*tmp = toupper(*tmp))
		tmp++;
	return str;
}


char *ucasecpy(char *dest, char *src) {
	char *tmp = dest;
	while (*tmp++ = toupper(*src++));
	return dest;
}


char *lcase(char *str) {
	char *tmp = str;
	while (*tmp = tolower(*tmp))
		tmp++;
	return str;
}


char *lcasecpy(char *dest, char *src) {
	char *tmp = dest;
	while (*tmp++ = tolower(*src++));
	return dest;
}


char *revstrchr(char *str, int tofind) {
	int i = strlen(str);
	do {
		i--;
		if (str[i] == tofind)
			return str + i;
	} while (i);
	return 0;
	/*__asm {
		cld
		mov edi, str_
		xor eax, eax
		xor ecx, ecx
		not ecx
		rep scasb
		neg ecx
		mov eax, tofind
		mov edi, str_
		add edi, ecx
		xor ecx, ecx
		not ecx
		std
		rep scasb
		not ecx
		lea eax, [ecx - 1]
		add eax, str_
	}*/				  
}


int stricmp_(const char *s1, const char *s2) {
	while (*s1) {
		if (tolower(*s1) != tolower(*s2))
			return 1;
		s1++;
		s2++;
	}
	return 0;
}


char *strrev(char *str) {
	char *p1, *p2;
	if (!str || !*str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}


char *strrevex(char *string, char *result) {
	int length = strlen(string), j = 0;
	result[length] = 0;
	for (int i = length - 1; i >= 0; i--) {
		result[j] = string[i];
		j++;
	}
	return result;
}


int charcount(char *str, int tofind) {
	int count = 0;
	while (*str) {
		if (*str == tofind)
			count++;
		str++;
	}
	return count;	
}


void StrToHexOut(char *data, int len, HWND hwnd) {
	char *sdfg = (char *)malloc(len * 3 + 1);
	for (int i = 0; i != len; i++) {
		sprintf(sdfg + i * 3, "%02x ", (unsigned char)data[i]);
	}
	AddChat(vbYellow, sdfg, hwnd);
	free(sdfg);
}


char *GetStuff(char *profilename, char *section, char *key, int size, char *output) {
	char tmppath[256];
	sprintf(tmppath, "%s\\Profiles\\%s.ini", CurrDir, profilename);
	GetPrivateProfileString(section, key, NULL, output, size, tmppath);
	return output;
}


void WriteStuff(char *profilename, char *section, char *key, char* value) {
	char tmppath[256];
	sprintf(tmppath, "%s\\Profiles\\%s.ini", CurrDir, profilename);
	WritePrivateProfileString(section, key, value, tmppath);
}


char *GetStuffConfig(char *section, char *key, char *output) {
	char tmppath[256];
	sprintf(tmppath, "%s\\config.ini", CurrDir);
	GetPrivateProfileString(section, key, NULL, output, 256, tmppath);
	return output;
}


void WriteStuffConfig(char *section, char *key, char* value) {
	char tmppath[256];
	sprintf(tmppath, "%s\\config.ini", CurrDir);
	WritePrivateProfileString(section, key, value, tmppath);
}


char *TimeStamp(char *buf) {
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	if (st.wHour == 0) 
		st.wHour = 12;
	sprintf(buf, "[%02i:%02i:%02i %cM] ", (st.wHour > 12) ? st.wHour % 12 :
		st.wHour, st.wMinute, st.wSecond, (st.wHour > 12) ? 'P' : 'A');
	return buf;
}


bool DecodeCDKey(char *key, unsigned long *ProdVal, unsigned long *PubVal, unsigned long *PrivVal) {
	char CDKey[32];
	strcpy(CDKey, key);
	switch (strlen(CDKey)) {
		case 13:
			if (!DecodeStarcraftKey(CDKey))
				return false;
			sscanf(CDKey, "%2d%7d%3d", ProdVal, PubVal, PrivVal);
			return true;
		case 16:
			if (!DecodeWarcraft2Key(CDKey))
				return false;
			sscanf(CDKey, "%2X%6X%8X", ProdVal, PubVal, PrivVal);
			return true;
		case 26:
			if (!DecodeWC3Key(CDKey, (unsigned int *)ProdVal, (unsigned int *)PubVal, (char *)PrivVal))
				return false;
			return true;
	}
	return false;
}


int GetBNLSClient(unsigned long client) {
	switch (client) {
		case 'STAR':
			return 1;
		case 'SEXP':
			return 2;
		case 'W2BN':
			return 3;
		case 'D2DV':
			return 4;
		case 'D2XP':
			return 5;
		case 'JSTR':
			return 6;
		case 'WAR3':
			return 7;
		case 'W3XP':
			return 8;
		case 'DRTL':
			return 9;
		case 'DSHR':
			return 10;
		case 'SSHR':
			return 11;
	}
	return 0;
}						


///////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned int __fastcall GetIcon(unsigned long client, unsigned long flags) {
	if (flags) {
		if (flags & 64)
			return 17;
		if (flags & 32)
			return 16;
		if (flags & 8)
			return 15;
		if (flags & 4)
			return 14;
		if (flags & 2)
			return 13;
		if (flags & 1)
			return 12;
	}
	switch (client) {
		case 'CHAT':
			return 11;
		case 'W3XP':
			return 10;
		case 'WAR3':
			return 9;
		case 'W2BN':
			return 8;
		case 'D2XP':
			return 7;
		case 'D2DV':
			return 6;
		case 'DSHR':
			return 5;
		case 'DRTL':
			return 4;
		case 'JSTR':
			return 3;
		case 'SSHR':
			return 2;
		case 'SEXP':
			return 1;
		case 'STAR':
			return 0;
		default:
			return -1;
	}	   
}


int GetPingIcon(unsigned long ping, unsigned long flags) {
	if (flags & 0x10)
		return 31;
	else if (ping == 0xFFFFFFFF)
		return 30;
	else if (ping == 0)
		return -1;
	else if (ping <= 200)
		return 24;
	else if (ping <= 300)
		return 25;
	else if (ping <= 400)
		return 26;
	else if (ping <= 500)
		return 27;
	else if (ping <= 600)
		return 28;
	else
		return 29;
}


//////////////////////////////////////////////////////////////////


void InsertSubItem(int index, int column, char *text, int icon, int botindex) {
	LVITEMA lvsubitem;
	lvsubitem.mask = (icon != -1) ? LVIF_IMAGE : LVIF_TEXT;
	lvsubitem.pszText = text;
	lvsubitem.iImage  = icon;
	lvsubitem.iItem = index == -1 ? (int)SendMessage(bot[botindex]->hWnd_lvwChannel, LVM_GETITEMCOUNT, 0, 0) - 1: index;
	lvsubitem.iSubItem = column;
	SendMessage(bot[botindex]->hWnd_lvwChannel, LVM_SETITEM, 0, (LPARAM)&lvsubitem);
}


int LVFindItem(char *text, int index) {
	LVFINDINFOA lvfind;
	lvfind.flags = LVFI_STRING;
	lvfind.psz = text;
	return SendMessage(bot[index]->hWnd_lvwChannel, LVM_FINDITEM, -1, (LPARAM)&lvfind);
}


unsigned long GetItemlParam(long iIndex, HWND hwnd) {
	LVITEM lvItem;
	lvItem.iItem = iIndex;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	return SendMessage(hwnd, LVM_GETITEM, 0, (LPARAM)&lvItem) ? lvItem.lParam : 0;
}


char *GetItemText(int iIndex, char *outbuf, unsigned int buflen, int index) {
	LVITEMA lvitem;
	if (iIndex != -1)	{
		lvitem.mask = LVIF_TEXT;
		lvitem.iSubItem = 0;
		lvitem.pszText = outbuf;
		lvitem.cchTextMax = buflen;
		lvitem.iItem = iIndex;
		SendMessage(bot[index]->hWnd_lvwChannel, LVM_GETITEMTEXT, iIndex, (LPARAM)&lvitem); 
	}
	return outbuf;
}


#if 0
void AddLVItemSimple(char *text, unsigned long color, int index) {
	LPLVCOLOR lplvColor;
	lplvColor = (LPLVCOLOR)malloc(sizeof(LVCOLOR));
	lplvColor->bBold = false;
	lplvColor->cItem1 = color;
	lplvColor->cItem2 = color;
	lplvColor->cItem3 = color;
	InsertItem(text, -1, -1, lplvColor, index);	
}
#endif


void InsertItem(char *szItem, int iIcon, int iIndex, LPLVCOLOR lpColor, int index) {
	unsigned int tmpIndex;
	tmpIndex = (iIndex == -1) ? (int)SendMessage(bot[index]->hWnd_lvwChannel, LVM_GETITEMCOUNT, 0, 0) : iIndex;
	LVITEMA lviItem;
	lviItem.mask =  LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
	lviItem.iItem = tmpIndex;
	lviItem.iSubItem = 0;
	lviItem.pszText = szItem;
	lviItem.iImage = iIcon;
	lviItem.lParam = (long)lpColor;
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_INSERTITEM, 0, (LPARAM)&lviItem);
}


void ResetChLVContents(int index) {
	int numItems = SendMessage(bot[index]->hWnd_lvwChannel, LVM_GETITEMCOUNT, 0, 0);
	while (numItems--) 
		free((void *)GetItemlParam(numItems, bot[index]->hWnd_lvwChannel));
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_DELETEALLITEMS, 0, 0);
}


void ClearLVTmpToolTipText(int index) {
	int numItems = SendMessage(bot[index]->hWnd_lvwChannel, LVM_GETITEMCOUNT, 0, 0);
	while (numItems--) {
		LPLVCOLOR lpColor = (LPLVCOLOR)GetItemlParam(numItems, bot[index]->hWnd_lvwChannel);
		if (!IsBadReadPtr((void *)lpColor->cItem3, 64))
			free((void *)lpColor->cItem3);
	}
}


void FreeAllLVStuff(int index) {
	for (int i = 0; i != ListView_GetItemCount(bot[index]->hWnd_lvwChannel); i++) {
		LPLVCOLOR lplvColor = (LPLVCOLOR)GetItemlParam(i, bot[index]->hWnd_lvwChannel);
		if (lplvColor) {
			if (bot[index]->curlvtab) {
				if (!IsBadReadPtr((void *)lplvColor->cItem3, 128))
					free((void *)lplvColor->cItem3);
			}
			free(lplvColor);
		}
	}
}


void ReloadChannelList(int index) {
	char sdfg[64], strclan[8];

	LPUSER lpUser = bot[index]->firstlvuser;
	if (!lpUser) {
		AddChat(vbRed, "Error, firstlvuser == NULL!", bot[index]->hWnd_rtfChat);
		return;
	}

	for (int i = 0; i != bot[index]->numusers; i++) {
		if (!lpUser)
			break;
		InsertItem(lpUser->username, GetIcon(lpUser->client, lpUser->flags), i, lpUser->clrstruct, index);
		if (gfstate & GFS_UITEXTPING) {
			sprintf(sdfg, "%u", lpUser->ping);
			InsertSubItem(i, 1, sdfg, -1, index);
		} else {
			InsertSubItem(i, 1, NULL, GetPingIcon(lpUser->ping, lpUser->flags), index);
		}
		sprintf(sdfg, "0x%02x", lpUser->flags);
		InsertSubItem(i, 2, sdfg, -1, index);

		*(__int32 *)strclan = lpUser->clan;
		strclan[4] = 0;
		//fastswap32((unsigned long *)strclan);
		InsertSubItem(i, 3, strclan, -1, index);

		lpUser = (LPUSER)lpUser->nextlvuser;
	}
	UpdatelblChannel(index);
}


char *GetItemTextLt(HWND hwnd, int index, char *buf, int size) {
	LVITEM lvitem;
	lvitem.mask = LVIF_TEXT;
	lvitem.iSubItem = 0;
	lvitem.pszText = buf;
	lvitem.cchTextMax = size;
	lvitem.iItem = index;
	SendMessage(hwnd, LVM_GETITEMTEXT, index, (LPARAM)&lvitem);
	return buf;
}


int GetUptimeString(unsigned long time, char *outbuf) {
	unsigned long days, hours, minutes, seconds, milliseconds;
	div_t tmp;
	tmp          = udiv(time, 86400000);
	days         = tmp.quot;
	time         = tmp.rem;
	tmp          = udiv(time, 3600000);
	hours        = tmp.quot;
	time         = tmp.rem;
	tmp          = udiv(time, 60000);
	minutes      = tmp.quot;
	time         = tmp.rem;
	tmp          = udiv(time, 1000);
	seconds      = tmp.quot;
	milliseconds = tmp.rem;
	return sprintf(outbuf, "%u d | %u h | %u m | %u s | %u ms",
		days, hours, minutes, seconds, milliseconds);	
}


int GetLngUptimeString(unsigned long time, char *outbuf) {
	unsigned long years, weeks, days, hours, minutes, seconds;
	div_t tmp;
	tmp     = udiv(time, 31449600);
	years   = tmp.quot;
	time    = tmp.rem;
	tmp     = udiv(time, 604800);
	weeks   = tmp.quot;
	time    = tmp.rem;
	tmp     = udiv(time, 86400);
	days    = tmp.quot;
	time    = tmp.rem;
	tmp     = udiv(time, 3600);
	hours   = tmp.quot;
	time    = tmp.rem;
	tmp     = udiv(time, 60);
	minutes = tmp.quot;
	seconds = tmp.rem;
	return sprintf(outbuf, "%u years, %u weeks, %u days, %u hours, %u minutes, %u seconds",
		years, weeks, days, hours, minutes, seconds);	
}


int GetSmallUptimeString(unsigned long time, char *outbuf) {
	unsigned long hours, minutes, seconds;
	div_t tmp;
	tmp          = udiv(time, 3600000);
	hours        = tmp.quot;
	time         = tmp.rem;
	tmp          = udiv(time, 60000);
	minutes      = tmp.quot;
	time         = tmp.rem;
	seconds = time / 1000;
	return sprintf(outbuf, "%u h, %u m, %u s", hours, minutes, seconds);	
}


void FormatText(char *dest, char *src, int index, LPGREETARGS ga) {
	while (*src) {
		if (*src == '%') {
			src++;
			switch (*src) {
				case 'u':
					src++;
					switch (*src) {
						case 's':
							dest += GetUptimeString(GetTickCount(), dest);
							break;
						case 'c':
							dest += GetUptimeString(bot[index]->connectedtick, dest);
							break;
						case 'l':
							dest += GetUptimeString(bot[index]->loadedtick, dest);
					}
					break;
				case 'g':
					src++;
					if (ga) {
						switch (*src) {
							case 'u':
								dest += sprintf(dest, "%s", ga->user) - 1;
								break;
							case 'p':
								dest += sprintf(dest, "%d", ga->ping) - 1;
								break;
							case 'f':
								dest += sprintf(dest, "0x%02x", ga->flags) - 1;
						}
					}
					break;
				case 'b':
					dest += sprintf(dest, "%d", bot[index]->bancount) - 1;
					break;
				case 'n':
					dest += sprintf(dest, "%d", bot[index]->numusers) - 1;
					break;
				case 'p':
					dest += sprintf(dest, "%d", bot[index]->ping) - 1;
					break;
				case 'f':
					dest += sprintf(dest, "0x%02x", bot[index]->bnetflags) - 1;
					break;
				case 'c':
					dest += sprintf(dest, "%s", bot[index]->currentchannel) - 1;
					break;
				case 's':
					dest += sprintf(dest, "%d", bot[index]->sendcount) - 1;
					break;
				case 'r':
					dest += sprintf(dest, "%d", bot[index]->recvcount) - 1;
					break;
				case 'j':
					dest += sprintf(dest, "%d", (GetTickCount() - bot[index]->lastjointime) / 1000) - 1;
					break;
				case 'o':
					dest += sprintf(dest, "%d", bot[index]->joincount) - 1;
					break;
				case 'a':
					dest += sprintf(dest, "%d", bot[index]->talkcount) - 1;
					break;
				case 'w':
					GetWinampSong(dest);
					dest += strlen(dest);
					break;
				case '%':
					*dest = '%';
			}
		} else {
			*dest = *src;
		}
		src++;
		dest++;
	}
	*dest = 0;
}


unsigned long GetLVColor(unsigned long flags) {
	if (flags) {
		if (flags & 1)
			return vbRBlue;
		else if (flags & 2)
			return vbRCream;
		else if (flags & 4)
			return 0x80FFFF;
		else if (flags & 8)
			return vbRGreen;
		else if (flags & 32)
			return vbRed;
	}
	return vbWhite;
}


unsigned long GetPingTextColor(unsigned long ping, unsigned long flags) {
	if (flags & 0x10)
		return vbGray;
	if (ping == 0xFFFFFFFF)
		return 0x00FF00AA;
	else if (ping == 0)
		return 0x00DE9D2E;
	else if (ping <= 300)
		return vbGreen;
	else if (ping <= 500)
		return vbYellow;
	else
		return vbRed;
}


unsigned long GetColor(unsigned long flags) {
	if (flags) {
		if (flags & 0x01)
			return vbRBlue;
		else if (flags & 0x02)
			return vbWhite;
		else if (flags & 0x08)
			return vbRGreen;
	}
	return vbYellow;
}


void UpdatelblChannel(int index) {
	char asdf[128];
	if (!bot[index]->curlvtab) {
		sprintf(asdf, "%s (%u)", bot[index]->currentchannel, bot[index]->numusers);
		SendMessage(bot[index]->hWnd_Statusbar, SB_SETTEXT, SBPART_CHANNEL, (LPARAM)asdf);
		SetWindowText(bot[index]->hWnd_lblChannel, asdf);
	}
} 	   


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char *TextTransformOutgoing(char *text, int txtattrib) {
	int len, totallen = 0;
	char *blah, *output, *tmp;
	int convsfinished = 0;
	len = strlen(text);
	output = text;
	if (!(txtattrib & (TM_XR | TM_HEX))) {
		if (txtattrib & TM_CONVERTTABS) {
			
			blah = text;  	
			while (*blah) {
				totallen += (*blah == 0x09) ? 3 : 1;
				blah++;
			}
			//if (totallen != (int)len) {
				len = totallen;
				output = (char *)malloc(len + 2);
													  
				blah = text;
				tmp = output;								  		  
				while (*blah) {
					if (*blah == 0x09) {
						*(int *)tmp = 0x202020;
						tmp += 3;
					} else { 
						*tmp = *blah;
						tmp++;
					}
					blah++;
				}
				output[len] = 0;
				convsfinished |= TM_CONVERTTABS;
			//}
		}

	}
									 
	if (txtattrib & TM_ALTCAPS)
		TextTransformToAltCaps(output);
	else if (txtattrib & TM_LEETSPEAK)
		TextTransformToLeetSpeak(output);

	if (txtattrib & TM_XR) { //parameters: channel and factor

		output = (char *)malloc(len + 8);
		output[len + 7] = 0;
		*output = 'U';
		XREncrypt((unsigned char *)text, (unsigned char *)output + 1, 40, 41);

	} else if (txtattrib & TM_HEX) {
		output = (char *)malloc((len << 1) + 2);
		output[(len << 1) + 1] = 0;
		*output = 'E';
		for (int i = 0; i != len; i++)
			sprintf(output + i * 2 + 1, "%02X", (unsigned char)text[i]);
	}
	return output;
}
																		
void TextTransformToAltCaps(char *text) {
	int i;
	int len = strlen(text);
	int len2 = len >> 1;
	len -= len2;
	for (i = 0; i != len; i++)
		text[i << 1] = toupper(text[i << 1]);
	for (i = 0; i != len2; i++)
		text[(i << 1) + 1] = tolower(text[(i << 1) + 1]);
}

void TextTransformToLeetSpeak(char *text) {
	while (*text) {
		int blah = tolower(*text);
		if (blah >= 'a' && blah <= 'z')
			*text = "43cd3f6h1jk1mn0pq257uvwxyz"[blah - 'a'];
		text++;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if 0
int __fastcall GetProfileFromBot(int botindex) {
	for (int i = 0; i != numprofiles; i++) {
		if (profiles[i]->hash == bot[botindex]->pnamehash)
			return i;
	}
	return -1;
}

int __fastcall GetBotFromProfile(int profileindex) {
	for (int i = 0; i != numbots; i++) {
		if (bot[i]) {
			if (bot[i]->pnamehash == profiles[profileindex]->hash)
				return i;
		}
	}
	return -1;
}
#endif


int __fastcall GetTabFromIndex(int index) {
	//for (int i = 0; i != TabCtrl_GetItemCount(hWnd_Tab); i++) {
	//	if (index == GetTabItemParam(i))
	//		return i;
	//}
	//return -1;
	return (index >= 0) ? bot[index]->tabindex : -1;
}


int __fastcall GetTabItemParam(int item) {
	TCITEM ti;
	ti.mask = TCIF_PARAM;
	TabCtrl_GetItem(hWnd_Tab, item, &ti);
	return ti.lParam;
}


void FreeNLS(int index) {
	if (bot[index]->nls) {
		bncsutil.nls_free(bot[index]->nls);
		bot[index]->nls = NULL;
	}
	if (bot[index]->oldnls)	{
		bncsutil.nls_free(bot[index]->oldnls);
		bot[index]->oldnls = NULL;
	}
}


void CheckCommCtrl32Ver() {
	char asdf[64];
	DLLVERSIONINFO dvi;
	dvi.cbSize = sizeof(DLLVERSIONINFO);
	HMODULE hComCtl = GetModuleHandle("comctl32");
	DLLGETVERSIONPROC DllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hComCtl, "DllGetVersion");
	DllGetVersion(&dvi);
	if (dvi.dwMajorVersion < 6) {
		sprintf(asdf, "WARNING: Using ComCtl32 version %d.%d.%d - Some UI elements may not work properly!",
			dvi.dwMajorVersion, dvi.dwMinorVersion, dvi.dwBuildNumber);
		if (hWnd_status_re)
			AddChat(vbRed, asdf, hWnd_status_re);
		else
			MessageBox(NULL, asdf, NULL, 0);	 
	}
}


void LoadBinaryImages() {
	//unsigned long read;	
	char blah[MAX_PATH];
	GetStuffConfig("Hashes", "STARHash1", blah);
	HMODULE hMod = LoadLibraryEx(blah, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hMod == INVALID_HANDLE_VALUE) {
		AddChat(vbRed, "Failed to open starcraft.exe!", hWnd_status_re);
		return;
	}

	lpModuleImage[0] = (char *)((int)hMod & 0xFFFF0000);

	GetStuffConfig("Hashes", "WAR3Hash3", blah);
	hMod = LoadLibraryEx(blah, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hMod == INVALID_HANDLE_VALUE) {
		AddChat(vbRed, "Failed to open game.dll!", hWnd_status_re);
		return;
	}

	lpModuleImage[1] = (char *)((int)hMod & 0xFFFF0000);
	/*
	HANDLE hfStarCraft = CreateFile(blah, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hfStarCraft == INVALID_HANDLE_VALUE) {
		AddChat(vbRed, "Failed to open starcraft.exe!", hWnd_status_re);
		return;
	}
	int filesize = GetFileSize(hfStarCraft, NULL);
	lpSCImage = (char *)malloc(filesize);
	ReadFile(hfStarCraft, lpSCImage, filesize, &read, NULL);
	CloseHandle(hfStarCraft);								
	if (*(__int16 *)lpSCImage != 'ZM')
		AddChat(vbRed, "Invalid PE signature in starcraft.exe", hWnd_status_re);
	*/
}


char *GetServer(int index, bool wc3) {
	switch (*(int *)(bot[index]->server + 4)) {
		case '2.04':
			return eastsvr[wc3];
		case '8.14':
			return westsvr[wc3];
		case '.842':
			return eurosvr[wc3];
		case '.332':
			return asiasvr[wc3];
	}
	return NULL;
}


void CheckMissingFiles() {
	char asdf[512], sdfg[128];
	bool missing = false;
	strcpy(asdf, "Missing the following file(s):\n");
	for (int i = 0; i != NUM_FILESTOCHECK; i++) {
		if (GetFileAttributes(filestocheck[i]) == INVALID_FILE_ATTRIBUTES) {
			missing = true;
			sprintf(sdfg, "   %s %s\n", i < 4 ? "directory" : "file", filestocheck[i]);
			strcat(asdf, sdfg);
			if (i < 4) {
				CreateDirectory(filestocheck[i], NULL);
			} else {
				HANDLE hFile = CreateFile(filestocheck[i], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
				CloseHandle(hFile);
			}
		}
	} 
	if (missing)
		AddChat(vbRed, asdf, hWnd_status_re);
}


void __stdcall PingSpoofProc(int index) {
	char asdf[64];
	sprintf(asdf, "Sleep()ing for %d milliseconds...", bot[index]->spoofedping);
	AddChat(vbYellow, asdf, bot[index]->hWnd_rtfChat);
	Sleep(bot[index]->spoofedping);
	AddChat(vbGreen, "Wakey wakey!", bot[index]->hWnd_rtfChat);
	InsertDWORD(0);
	SendPacket(0x25, index);
	PostMessage(hWnd_main, WM_WAKEUP, 0, index);
}


void WaitForPingSpoof(int index) {
	MSG msg;
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PingSpoofProc, (void *)index, 0, NULL);
	while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
		if (msg.message == WM_WAKEUP && msg.lParam == index && msg.hwnd == hWnd_main)
			break;
		if (!TranslateMDISysAccel(hWnd_Client, &msg)) {
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}
	CloseHandle(hThread);
}


void XRDecrypt(unsigned char *in, unsigned char *out, int channel, int factor) {
	unsigned char ci[4];
	if (channel < 0)
		channel = -channel;
	if (channel > 9999)
		channel %= 10000;
	ci[3] = channel % 10;
	ci[2] = (channel % 100) / 10;
	ci[1] = (channel % 1000) / 100;
	ci[0] = channel / 1000;
	if (((in[0] - ci[0] - 32) % 25) &&
		((in[1] - ci[1] - 32) % 25) &&
		((in[2] - ci[2] - 32) % 25) &&
		((in[3] - ci[3] - 32) % 25)) { 
		*out = 0;
		return;
	}
	int z = in[4] - 32;
	in += 5;
	while (*in) {
		z += factor;
		z %= 94;
		int k = *in - z - 32;
		if (k < 0)
			k += 94;
		//k += 32;
		*out = k + 32;
		in++;
		out++;
	}
	*out = 0;
}


void XREncrypt(unsigned char *in, unsigned char *out, int channel, int factor) {
	if (channel < 0)
		channel = -channel;
	if (channel > 9999)
		channel %= 10000;
	srand(GetTickCount());
	int z = rand() % 94;
	out[0] = ((rand() % 4) * 25) + 32 + (channel / 1000);
	out[1] = ((rand() % 4) * 25) + 32 + ((channel % 1000) / 100);
	out[2] = ((rand() % 4) * 25) + 32 + ((channel % 100) / 10);
	out[3] = ((rand() % 4) * 25) + 32 + (channel % 10);
	out[4] = z + 32;
	out += 5;
	while (*in) {
		z += factor;
		z %= 94;
		int k = *in + z;//((z + factor) % 94);
		*out = (k > 126) ? (((k + 62) % 94) + 32) : k;
		in++;
		out++;
	}
	*out = 0;
}


void HexToStr(char *in, char *out) {
	while (*in) {
		if ((unsigned char)(*in - 'A') <= 'F' - 'A')
			*out = (*in - 'A' + 10) << 4;
		else if ((unsigned char)(*in - '0') <= '9' - '0')
			*out = (*in - '0') << 4;
		else
			goto done;
		in++;
		if ((unsigned char)(*in - 'A') <= 'F' - 'A')
			*out |= (*in - 'A' + 10);
		else if ((unsigned char)(*in - '0') <= '9' - '0')
			*out |= (*in - '0');
		else
			goto done;
		in++;
		out++;
	}
done:
	*out = 0;
}


void QueryFormatVars(char *output, char *value, int index) {
	int cmdid = hash((unsigned char *)lcase(value), -1);
	switch (cmdid) {
		case QUERYVAL_XR:
			sprintf(output, "XR Channel: %d  |  XR Factor: %d",	xrchannel, xrfactor);
			break;
		case QUERYVAL_SCK:
			sprintf(output, "Socket descriptor: %d", bot[index]->sck);
			break;
		case QUERYVAL_HWND:
			sprintf(output, "main: 0x%08x  |  rtfChat: 0x%08x  |  lvwChannel: 0x%08x"
				"  |  txtChat: 0x%08x  |  Statusbar: 0x%08x  |  tab: 0x%08x  "
				"|  lblChannel: 0x%08x", bot[index]->hWnd_main, bot[index]->hWnd_rtfChat,
				bot[index]->hWnd_lvwChannel, bot[index]->hWnd_txtChat,
				bot[index]->hWnd_Statusbar, bot[index]->hWnd_tab, bot[index]->hWnd_lblChannel);
			break;
		case QUERYVAL_STATE:
			sprintf(output, "Profile specific state: 0x%08x  |  Global state: 0x%08x",
				bot[index]->fstate, gfstate);
			break;
		case QUERYVAL_BANCOUNT:
			sprintf(output, "Bancount: %u",	bot[index]->bancount);
			break;
		case QUERYVAL_SENT:
			sprintf(output, "Total packets sent since logon: %d.", bot[index]->sendcount);
			break;
		case QUERYVAL_RECEIVED:
			sprintf(output, "Total packets recevied since logon: %d.", bot[index]->recvcount);
			break;
		default:
			strcpy(output, "Unknown variable.");
	}
}


bool IsFileValid(char *filename, LPFILETIME lpft) {
	//FILETIME actualft;
	HANDLE hFile = CreateFile(filename, GENERIC_READ, 0, NULL,
					 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

#if 0	
	if (!GetFileTime(hFile, &actualft, NULL, NULL))
		return false;
	CloseHandle(hFile);	
	if (CompareFileTime(lpft, &actualft))
		return false; //always incorrect filetime... what??
#endif

	CloseHandle(hFile);
	return true;
}

