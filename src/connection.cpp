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
#include "packets.h"
#include "clan.h"
#include "gui.h"
#include "hashtable.h"
#include "tray.h"
#include "connection.h"


///////////////////////////////////////////////////////////////////////////////


bool ConnectSocket(SOCKET *sck, char *server, unsigned short port) {
	struct sockaddr_in sName;

	*sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*sck == INVALID_SOCKET)
		return false;

	sName.sin_family = AF_INET;
	sName.sin_port = htons(port);

	char *p = server;
	while (*p && (isdigit(*p) || (*p == '.')))
		p++;

	if (*p) {
		struct hostent *hstEnt = gethostbyname(server);
		if (!hstEnt)
			return false;
		memcpy(&sName.sin_addr, hstEnt->h_addr, hstEnt->h_length);
	} else {
		sName.sin_addr.s_addr = inet_addr(server);
	}

	if (connect(*sck, (struct sockaddr *)&sName, sizeof(sName)))
		return false;

	return true;
}


bool ConnectAsyncSocket(int index, char *server, unsigned short port) {
	struct sockaddr_in sName;
	SOCKET sck;

	sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sck == INVALID_SOCKET)
		return false;

	if (WSAAsyncSelect(sck, bot[index]->hWnd_main, WM_WSDATAARRIVAL,
		FD_READ | FD_CLOSE | FD_CONNECT) == SOCKET_ERROR)
		return false;

	memset(&sName, 0, sizeof(sName));
	sName.sin_family = AF_INET;
	sName.sin_port   = htons(port);

	char *p = server;
	while (*p && (isdigit(*p) || (*p == '.')))
		p++;

	if (*p) {
		struct hostent *hstEnt = gethostbyname(server);
		if (!hstEnt) {
			closesocket(sck);
			return false;
		}
		memcpy(&sName.sin_addr, hstEnt->h_addr, hstEnt->h_length);
	} else {
		sName.sin_addr.s_addr = inet_addr(server);
	}

	if (connect(sck, (struct sockaddr *)&sName, sizeof(sName))) {
		int tmp = WSAGetLastError();
		if (tmp != WSAEWOULDBLOCK) {
			shutdown(sck, SD_BOTH);
			closesocket(sck);
			return false;
		}
	}

	bot[index]->sck = sck;

	return true;
}


void ConnectProfile(int index) {
	char asdf[256];
	unsigned int proxyport;
	TCITEM tc;
	MENUITEMINFO mii;

	if (index == -1)
		return;
	if (bot[index]->connected)
		DisconnectProfile(index, true);

	KillTimer(bot[index]->hWnd_main, index | 0x4000);

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_CHECKED;			
	SetMenuItemInfo(hMenu_main_popup, index + 11000, false, &mii);
	
	tc.iImage = STATEICO_CONNERR;
	tc.mask = TCIF_IMAGE;
	SendMessage(hWnd_Tab, TCM_SETITEM, (WPARAM)GetTabFromIndex(index), (LPARAM)&tc);

	switch (bot[index]->client) {
		case 'STAR':
		case 'SEXP':
			bot[index]->fstate |= BFS_STARCRAFT;
			break;
		case 'D2DV':
		case 'D2XP':
			bot[index]->fstate |= BFS_DIABLO2;
			break;
		case 'WAR3':
		case 'W3XP':
			bot[index]->fstate |= BFS_WARCRAFT3;
			break;
		case 'SSHR':
		case 'JSTR':
		case 'DSHR':
		case 'DRTL':
			bot[index]->fstate |= BFS_USELEGACY;
	}
	
	if (bot[index]->fstate & BFS_USEPROXY) {
		char proxystr[32], *portstr;
		
		strcpy(proxystr, bot[index]->proxy);
		portstr = strchr(proxystr, ':');
		if (!portstr) {
			AddChat(vbRed, "[PROXY] Bad proxy format, cannot get port.",
				bot[index]->hWnd_rtfChat);
			return;
		}
		
		*portstr++ = 0;
		proxyport = atoi(portstr);

		AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow,
			"[PROXY] Connecting to %s:%d...", proxystr, proxyport);
		if (!ConnectAsyncSocket(index, proxystr, proxyport))
			AddChatf(asdf, bot[index]->hWnd_rtfChat, vbRed,
				"ConnectAsyncSocket failed, WSAGetLastError: %d", WSAGetLastError());
	} else {
		AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow,
			"[BNET] Connecting to %s:%6112...", bot[index]->server);
		if (!ConnectAsyncSocket(index, bot[index]->server, 6112))
			AddChatf(asdf, bot[index]->hWnd_rtfChat, vbRed,
				"ConnectAsyncSocket failed, WSAGetLastError: %d", WSAGetLastError());
	}
}


int SendProxyRequest(int index, const char *identdname) {
	char txbuf[64];
	int len;

	len = strlen(identdname);
	if (len >= 32)
		return 0;

	txbuf[0] = 0x04;
	txbuf[1] = 0x01;
	
	*(unsigned __int16 *)(txbuf + 2) = htons(6112);
	*(unsigned __int32 *)(txbuf + 4) = inet_addr("206.17.175.120");
	strcpy(txbuf + 8, identdname);
	txbuf[8 + len] = 0;

	if (send(bot[index]->sck, txbuf, 8 + len + 1, 0) == -1)
		return 0;

	return 1;
}


#define USE_NEW_LEGACY_LOGON
void ContinueConnectProcess(int index) {
	AddChat(vbGreen, (bot[index]->fstate & BFS_USEPROXY) ? 
		"[PROXY] Connected!" : "[BNET] Connected!", bot[index]->hWnd_rtfChat);

	if ((bot[index]->fstate & BFS_USEPROXY) && !(bot[index]->fstate & BFS_FIRSTDATARX)) {
		SendProxyRequest(index, "anonymous");
	} else {
		send(bot[index]->sck, "\x01", 1, 0);
		if (bot[index]->fstate & BFS_USELEGACY) {
			#ifdef USE_NEW_LEGACY_LOGON
				Send0x1E(index);
				Send0x12(index);
				Send0x06(index);
			#else
				Send0x05(index);
				Send0x06(index);
			#endif
		} else {
			Send0x50(index);
		}
	}
}


void DisconnectProfile(int index, bool graceful) {
	TCITEM tc;
	MENUITEMINFO mii;

	if (index == -1)
		return;

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hMenu_main_popup, index + 11000, false, &mii); 

	for (int i = 0; i != NUM_SBPARTS; i++)
		SendMessage(bot[index]->hWnd_Statusbar, SB_SETTEXT, i, NULL);
	SendMessage(bot[index]->hWnd_Statusbar, SB_SETICON, SBPART_CLIENT, NULL);

	shutdown(bot[index]->sck, 2);
	closesocket(bot[index]->sck);

	bot[index]->connected = false;
	bot[index]->sendcount = 0;
	bot[index]->recvcount = 0;
	bot[index]->fstate &= BFS_STATICATTRIBUTEMASK;

	FreeNLS(index);
	FreeAllLVStuff(index);

	bot[index]->numusers     = 0;
	bot[index]->bancount     = 0;
	bot[index]->clan         = 0;
	bot[index]->firstlvuser  = NULL;
	bot[index]->lastadded    = NULL;
	bot[index]->flooded      = false;
	bot[index]->floodamt     = 0;
	bot[index]->floodtime    = 0;
	bot[index]->idleints     = 0;
	bot[index]->lastjointime = 0;
	bot[index]->self         = NULL;
	
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_DELETEALLITEMS, 0, 0);
	ResetTableContents(bot[index]->users, TABLESIZE_USERS); 
	SetWindowText(bot[index]->hWnd_lblChannel, NULL);
	AddChat(vbRed, "[BNET] Disconnected.", bot[index]->hWnd_rtfChat);

	tc.iImage = graceful ? STATEICO_DISCONN : STATEICO_CONNERR;
	tc.mask = TCIF_IMAGE;
	SendMessage(hWnd_Tab, TCM_SETITEM, (WPARAM)GetTabFromIndex(index), (LPARAM)&tc);

	KillTimer(hWnd_main, index + 1);
	KillTimer(bot[index]->hWnd_main, index | 0x4000);

	if (hWnd_ClanCreate && clancreatorindex == index)
		DestroyWindow(hWnd_ClanCreate);
}


void HandleProxyResponse(int i, SOCKET sck) {
	char rxbuf[16], *errstr;
	int recvlen;

	recvlen = recv(sck, rxbuf, sizeof(rxbuf), 0);
	if (!recvlen || recvlen == SOCKET_ERROR) {
		errstr = "[PROXY] Connection error!";
		goto err;
	}

	if (recvlen != 8) {
		errstr = "[PROXY] Invalid SOCKS4 CONNECT response length.";
		goto err;
	}

	if (rxbuf[0] != 0) {
		errstr = "[PROXY] Unsupported SOCKS4 version.";
		goto err;
	}

	switch (rxbuf[1]) {
		case 90:
			AddChat(vbGreen, "[PROXY] Request granted.", bot[i]->hWnd_rtfChat);
			break;
		case 91:
			errstr = "[PROXY] Request rejected.";
			goto err;
		case 92:
			errstr = "[PROXY] Request rejected becasue SOCKS server "
					"cannot connect to identd on client.";
			goto err;
		case 93:
			errstr = "[PROXY] Request rejected because the client program "
					"and identd report different userids.";
			goto err;
		default:
			errstr = "[PROXY] Unknown error.";
			goto err;
	}
	
	return;
err:
	AddChat(vbRed, errstr, bot[i]->hWnd_rtfChat);
	DisconnectProfile(i, false);
}


void HandleDataRecv(int i, SOCKET sck, LPARAM lParam) {
	int bnetlen;
	char asdf[256];

	if (LOWORD(lParam) == FD_READ) {
		if (loglevel > 2)
			AddChatf(asdf, hWnd_status_re, 0xFFC080, "FD_READ event on %d", i);
		if ((bot[i]->fstate & BFS_USEPROXY) && !(bot[i]->fstate & BFS_FIRSTDATARX)) {
			HandleProxyResponse(i, sck);
			bot[i]->fstate |= BFS_FIRSTDATARX;
			ContinueConnectProcess(i);
			return;
		}
		int tmp = recv(sck, bot[i]->wsdata + bot[i]->recvlen,
			sizeof(bot[i]->wsbuffer) - (bot[i]->wsdata - bot[i]->wsbuffer) - bot[i]->recvlen, 0);
		if (tmp == SOCKET_ERROR) {
			sprintf(asdf, "recv() error %d!", WSAGetLastError());
			AddChat(vbRed, asdf, bot[i]->hWnd_rtfChat);
			return;
		} else if (!tmp) {
			AddChat(vbRed, "recv() returned 0!", bot[i]->hWnd_rtfChat);
			DisconnectProfile(i, false);
			return;
		}
		bot[i]->recvlen += tmp;

		while (bot[i]->recvlen >= 4) {
			if ((unsigned char)*bot[i]->wsdata != (unsigned char)0xFF) {
				sprintf(asdf, "[BNET] Parse error! expected 0xFF, 0x%02x.", (unsigned char)*bot[i]->wsdata);
				AddChat(vbRed, asdf, bot[i]->hWnd_rtfChat);
				bot[i]->wsdata  = bot[i]->wsbuffer;
				bot[i]->recvlen = 0;
				return;
			}

			bnetlen = *(unsigned short *)(bot[i]->wsdata + 2);
			if (bnetlen > bot[i]->recvlen) {
				sprintf(asdf, "[BNET] Expecting %d more bytes.", bnetlen - bot[i]->recvlen);
				AddChat(vbYellow, asdf, bot[i]->hWnd_rtfChat);
				if (bot[i]->wsdata + bot[i]->recvlen >= bot[i]->wsbuffer + sizeof(bot[i]->wsbuffer)) {
					memcpy(bot[i]->wsbuffer, bot[i]->wsdata, bot[i]->recvlen);
					bot[i]->wsdata = bot[i]->wsbuffer;
				}
				return;
			}
#if 0
			int bleep, i2, i3;							 
			bleep = 0;
			i3 = 0;
			for (i2 = 0; i2 != bnetlen; i2++) {
				i3++;
				bleep += bot[i]->wsdata[i2] << 1;
				if (i3 == 5) {
					Beep((bleep << 3) + 300, 25);
					i3 = 0;
					bleep = 0;
				}
			}
#endif
			ParsePacket(bot[i]->wsdata[1], bot[i]->wsdata, i);
			bot[i]->recvcount++;
			bot[i]->recvlen -= bnetlen;
			bot[i]->wsdata  += bnetlen;
		}

		bot[i]->wsdata  = bot[i]->wsbuffer;
		bot[i]->recvlen = 0;
	} else {
		if (LOWORD(lParam) == FD_CLOSE) {
			if (loglevel > 2)
				AddChatf(asdf, hWnd_status_re, 0xFFC080, "FD_CLOSE event on %d", i);

			AddChat(vbRed, "[BNET] Winsock Close.", bot[i]->hWnd_rtfChat);
			bnetlen = HIWORD(lParam);
			if (bnetlen) {
				sprintf(asdf, "[BNET] Winsock Error %d !", bnetlen);
				AddChat(vbRed, asdf, bot[i]->hWnd_rtfChat);
				sprintf(asdf, "Winsock error %d encountered on profile %d!", bnetlen, i);
				PopupTrayMsg("Connection dropped!", asdf, NIIF_ERROR);
			}
			DisconnectProfile(i, false);  
			AutoReconnectStart(i, 0);
			return;	
		} else if (LOWORD(lParam) == FD_CONNECT) {
			if (loglevel > 2)
				AddChatf(asdf, hWnd_status_re, 0xFFC080, "FD_CONNECT event on %d", i);
		
			int err = WSAGETSELECTERROR(lParam);	
			if (err) {
				AddChatf(asdf, bot[i]->hWnd_rtfChat, vbRed,
					"Connection failed, error %d!", err);
				DisconnectProfile(i, false);
				AutoReconnectStart(i, 0);
				return;
			}
			ContinueConnectProcess(i);
		}
	}
	return;						  
}


void AutoReconnectStart(int index, int crcrate) {
	char asdf[64];
	if (rcrate) {
		AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow,
			"Attempting reconnect in %d seconds...", rcrate);
		SetTimer(bot[index]->hWnd_main, index | 0x4000,
			(crcrate ? crcrate : rcrate) * 1000, AutoReconnectProc);
	}
}


VOID CALLBACK AutoReconnectProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	int index = idEvent & ~0x4000;
	KillTimer(bot[index]->hWnd_main, idEvent);
	ConnectProfile(index);
}


#if 0
char *str_WinsockClose  = "[BNET] Winsock Close.";
char *str_WinsockErr    = "[BNET] Winsock Error %d !";
char *str_WinsockErrTray = "Winsock error %d encountered on profile %d!";
char *str_ErrTrayMsg = "Connection dropped!";

void HanddleDataRecv(int index, SOCKET sck, LPARAM lParam) {
	int bnetlen;
	char asdf[256];
	__asm {
		mov ecx, index
		lea edi, [bot + ecx * 4]
		mov edi, dword ptr [edi] 

		//;edx contains the current profile bot struct address; to be preserved
		//;ebx contains the error if there is any; to be preserved

		mov ebx, lParam
		cmp bx, FD_CLOSE
		jnz notclosed

		mov edx, dword ptr [edi]BOT.hWnd_rtfChat
		push offset str_WinsockClose
		push vbRed
		call AddChat
		add esp, 0Ch

		shr ebx, 16
		test ebx, ebx
		jz noerror

		push ebx
		push offset str_WinsockErr
		lea esi, [asdf]
		push esi
		call sprintf
		add esp, 0Ch

		mov edx, dword ptr [edi]BOT.hWnd_rtfChat
		push esi
		push vbRed
		call AddChat
		add esp, 0Ch

		push index
		push ebx
		push offset str_WinsockErrTray
		push esi
		call sprintf
		add esp, 0Ch

		push NIIF_ERROR
		push esi
		push str_ErrTrayMsg
		call PopupTrayMsg
		add esp, 0Ch

noerror:
		push 0
		push index
		call DisconnectProfile
		add esp, 8
		jmp tehret

notclosed:
		//ecx still = index here
		//find how much of the buffer is left
		mov eax, [edi]BOT.wsdata
		lea edx, [edi]BOT.wsbuffer
		sub eax, edx
		mov edx, 1024
		sub edx, eax

		mov eax, [edi]BOT.wsdata
		add eax, [edi]BOT.recvlen

		push 0
		push edx
		push eax
		push sck
		call dword ptr [recv]

		cmp eax, SOCKET_ERROR
		jnz norecverr

		call dword ptr [WSAGetLastError]
		lea esi, [asdf]
		push eax
		push offset str_ErrRecv
		push esi
		call sprintf
		add esp, 0Ch

		push [edi]BOT.hWnd_rtfChat
		push esi
		push vbRed
		call AddChat
		add esp, 0Ch

		jmp tehret		

norecverr:
		add [edi]BOT.recvlen, eax

parseloop:
		cmp [edi]BOT.recvlen, 4
		jl donewithloop

		cmp byte ptr [edi]BOT.wsdata, 0xFF
		jz sanitycheckpassed

		//error notification here

sanitycheckpassed:
		cmp [edi]BOT.recvlen, 4
		jge parseloop

donewithloop:

		//mov ebx, eax
	}
tehret:
	return;
}
#endif

