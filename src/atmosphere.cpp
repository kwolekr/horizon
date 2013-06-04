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
#include "gui.h"
#include "fxns.h"
#include "hashtable.h"
#include "atmosphere.h"

bool atmosactive;


///////////////////////////////////////////////////////////////////////////////


void LoadAtmosphere() {
	if (!(gfstate & GFS_UIATMOSPHERE))
		return;
	atmosactive = true;
	char buf[512];
	int i = 0, tmp;
	LPCHANNEL lpChannel;
	FILE *file = fopen("channels.atm", "r");
	if (file) {
		while (!feof(file)) {
			fgets(buf, sizeof(buf), file);
			tmp = strlen(buf) - 1;
			if (buf[tmp] == 0x0A) 
				buf[tmp] = 0; 
			if (buf[0] && buf[0] != '#') {
				if (!i) {
					lpChannel = (LPCHANNEL)malloc(sizeof(CHANNEL));
					strncpy(lpChannel->channelname, buf, 64);
					lcase(lpChannel->channelname);
				} else {
					strncpy((char *)lpChannel + 64 + ((i - 1) * 128), buf, 128);
				}
				i++;
				if (i == 4) {
					i = 0;
					InsertValue(lpChannel->channelname, lpChannel, channels, TABLESIZE_CHANNELS);
				}
			}
		}
		fclose(file);
	}
}


void EnableAtmosphere() {			 
	LVBKIMAGE lvimg;
	char tmp[64];
	if (atmosactive)
		return;
	atmosactive = true;
	lvimg.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
	for (int i = 0; i != numprofiles; i++) {
		if (bot[i]->ploaded && bot[i]->connected) {
			lcasecpy(tmp, bot[i]->currentchannel);
			LPCHANNEL lpChannel = (LPCHANNEL)GetValue(tmp, channels, TABLESIZE_CHANNELS);
			SetWindowLong(bot[i]->hWnd_rtfChat, GWL_EXSTYLE, 
				GetWindowLong(bot[i]->hWnd_rtfChat, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
			if (bot[i]->rtbPic)	
				DeleteObject(bot[i]->rtbPic);
			bot[i]->rtbPic = (HBITMAP)LoadImage(g_hInst, lpChannel->rtbPic, IMAGE_BITMAP, 480, 400, LR_LOADFROMFILE);
			if (bot[i]->lblPic)
				DeleteObject(bot[i]->lblPic);
			bot[i]->lblPic = (HBITMAP)LoadImage(g_hInst, lpChannel->lblPic, IMAGE_BITMAP, 480, 400, LR_LOADFROMFILE);
			lvimg.pszImage = lpChannel->lvwPic;
			SendMessage(bot[i]->hWnd_lvwChannel, LVM_SETBKIMAGE, 0, (LPARAM)&lvimg);
			RedrawWindow(bot[i]->hWnd_lblChannel, NULL, NULL, RDW_INVALIDATE);
		}
	}
}


void DisableAtmosphere() {
	LVBKIMAGE lvimg;
	if (!atmosactive)
		return;
	atmosactive = false;
	lvimg.ulFlags = LVBKIF_SOURCE_NONE;
	for (int i = 0; i != numprofiles; i++) {
		if (bot[i]->ploaded) {
			SetWindowLong(bot[i]->hWnd_rtfChat, GWL_EXSTYLE, 
				GetWindowLong(bot[i]->hWnd_rtfChat, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
			if (bot[i]->rtbPic)	{
				DeleteObject(bot[i]->rtbPic);
				bot[i]->rtbPic = NULL;
			}
			if (bot[i]->lblPic)	{
				DeleteObject(bot[i]->lblPic);
				bot[i]->lblPic = NULL;
			}
			SendMessage(bot[i]->hWnd_lvwChannel, LVM_SETBKIMAGE, 0, (LPARAM)&lvimg);
			RedrawWindow(bot[i]->hWnd_lblChannel, NULL, NULL, RDW_INVALIDATE);
		}
	}
}

