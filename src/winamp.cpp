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
#include "winamp.h"

HWND hWnd_Winamp;
char WinampPath[MAX_PATH];


///////////////////////////////////////////////////////////////////////////////


void GetWinampPath() {
	HKEY hKey;
	unsigned long value = 1, size = MAX_PATH;
	RegOpenKey(HKEY_CURRENT_USER, "SOFTWARE\\Winamp", &hKey);
	RegQueryValueEx(hKey, NULL, 0, &value, (unsigned char *)WinampPath, &size);
	RegCloseKey(hKey);
}


bool InitWinamp() {
	if (!WinampPath[0])
		GetWinampPath();
	hWnd_Winamp = FindWindow("Winamp v1.x", NULL);
	return hWnd_Winamp != 0;
}


void LoadWinamp() {
	GetWinampPath();
	int pathlen = strlen(WinampPath);
	strcpy(WinampPath + pathlen, "\\winamp.exe");
	ShellExecute(hWnd_main, NULL, WinampPath, NULL, "C:\\", 1);
	WinampPath[pathlen] = 0;
	InitWinamp();
}


void QuitWinamp() {
	if (InitWinamp())
		PostMessage(hWnd_Winamp, WM_CLOSE, 0, 0);
}


void PushWinampButton(WPARAM button) {
	if (InitWinamp())
		SendMessage(hWnd_Winamp, WM_COMMAND, button, 0);
}


void SetWinampVolume(int volume) {
	if (InitWinamp())
		SendMessage(hWnd_Winamp, WM_USER, (volume << 1) + (volume >> 1) + (volume & 0x0F) + 1, 0x7A);
}


void GetWinampSong(char *output) {
	char asdf[128];
	if (InitWinamp()) {
		GetWindowText(hWnd_Winamp, asdf, sizeof(asdf));
		div_t current = div((int)SendMessage(hWnd_Winamp, WM_USER, 0, 105) / 1000, (int)60);
		div_t res = div((int)SendMessage(hWnd_Winamp, WM_USER, 1, 0x69), (int)60);
		sprintf(output, "%s  [%02d:%02d/%02d:%02d]", asdf, current.quot, current.rem, res.quot, res.rem);
	}
}


void PlayWinampSong(char *songname) {
	char buf[256];
	if (InitWinamp()) {
		if (int trackindex = atoi(songname)) {
			SendMessage(hWnd_Winamp, WM_USER, trackindex - 1, 0x79);
		} else {
			SendMessage(hWnd_Winamp, WM_COMMAND, 0, 120);
			int pathlen = strlen(WinampPath);
			strcpy(WinampPath + pathlen, "\\Winamp.m3u");
			FILE *file = fopen(WinampPath, "r");
			if (!file) {
				sprintf(buf, "Failed to open %s!", WinampPath);
				AddChat(vbRed, buf, hWnd_status_re);
				return;
			}
			WinampPath[pathlen] = 0;
			while (!feof(file)) {
				fgets(buf, sizeof(buf), file);
				if (*(int *)buf != 'TXE#') {
					char *tmp = revstrchr(buf, '\\');
					if (tmp) {
						*tmp++ = 0;
						if (strstr(ucase(tmp), ucase(songname))) {
							SendMessage(hWnd_Winamp, WM_USER, trackindex, 0x79);
							break;
						}
						trackindex++;
					}
				}
			}
			fclose(file);
		}
	}
	SendMessage(hWnd_Winamp, WM_COMMAND, WINAMP_PLAY, 0);
}

