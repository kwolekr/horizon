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
#include "winamp.h"
#include "packetbuffer.h"
#include "profile.h"
#include "packets.h"
#include "resource.h"
#include "clan.h"
#include "botprofiles.h"
#include "connection.h"
#include "queue.h"
#include "tray.h"
#include "chain.h"
#include "chat.h"

/*const char *commands[] = {
	"ver",
	"version",
	"cq",
	"profile",
	"forcejoin",
	"fjoin",
	"leavechat",
	"rj",
	"rejoin",
	"uptime",
	"home",
	"connect",
	"rc",
	"reconnect",
	"disconnect",

	"play",
	"stop",
	"mp3",
	"pause",
	"vol", 
	"setvolume",
	"next",
	"prev",
	"winamp"
};		  */


///////////////////////////////////////////////////////////////////////////////


void Parse0x0F(char *data, int index) {
	char *user, *text, sdfg[512];
	unsigned long event = *(unsigned long *)(data + 4);
	unsigned long flags = *(unsigned long *)(data + 8);
	unsigned long ping  = *(unsigned long *)(data + 12);

	#ifdef SHOW_DEFUNCT_CHAT_INFO
		AddChatf(sdfg, bot[index]->hWnd_rtfChat, vbCyan, "\n   IP: %08x\n   Acc #: %08x\n   Reg Auth: %08x",
			*(int *)(data + 16), *(int *)(data + 20), *(int *)(data + 24));
	#endif
	user = data + 28;
	text = data + 29 + strlen(user);
	switch (event) {
		case 0x01:
		case 0x02:
			EID_JOIN(user, text, flags, ping, event == 2, index);
			break;
		case 0x03:
			EID_LEAVE(user, flags, ping, index);
			break;
		case 0x04:
			EID_WHISPERFROM(user, text, flags, index);
			break;
		case 0x05:
			EID_TALK(user, text, flags, index);
			break;
		case 0x06:
			sprintf(sdfg, "Broadcast: %s", text);
			AddChat(vbRDBlue, sdfg, bot[index]->hWnd_rtfChat);
			break;
		case 0x07:
			EID_CHANNEL(text, flags, ping, index);
			break;
		case 0x09:
			EID_FLAGSUPDATE(user, text, flags, ping, index);
			break;
		case 0x0A:
			EID_WHISPERTO(user, text, flags, index);
			break;
		case 0x0F:
			strcat(text, " has Clan Private enabled.");
			AddChat(vbRed, text, bot[index]->hWnd_rtfChat);
			break;
		case 0x12:
			AddChat(vbRDBlue, text, bot[index]->hWnd_rtfChat);
			/*if (strstr(text, "as ban"))
				bot[index]->bancount++;
			else if (strstr(text, "as unb"))
				bot[index]->bancount--;*/
			EID_INFO(data, text, index);
			break;
		case 0x13:
			AddChat(vbRed, text, bot[index]->hWnd_rtfChat);
			break;
		case 0x17:
			EID_EMOTE(user, text, flags, index);
			break;
		default:
			AddChatf(sdfg, hWnd_status_re, vbYellow,
				"[%d] WARNING: Unhandled EID 0x%02x encountered", index, event);
	}
}


void EID_JOIN(char *user, char *text, unsigned long flags, unsigned long ping, bool joined, int index) {
	char sdfg[128], strclient[8], strclan[8], *tmp, *character = NULL;

	if (bot[index]->fstate & BFS_DIABLO2) {
		character = user;
		tmp = strchr(user, '*');
		if (tmp) {
			*tmp = 0;
			user = tmp + 1;
		}
	}

	if (GetValue(user, bot[index]->users, TABLESIZE_USERS)) {
		if (*(int *)text == 'WAR3' || *(int *)text == 'W3XP') {
			bot[index]->lastpersonjoined->clan = *(int *)(text + strlen(text) - 4);
				fastswap32((unsigned __int32 *)&bot[index]->lastpersonjoined->clan);
		} else {
			AddChatf(sdfg, hWnd_status_re, vbRed, "[%d] ERROR: %s joined twice!", index, user);
		}
		return;
	}

	bool self = !strcmp(user, bot[index]->realname);
	int GetColorTmp = GetLVColor(flags), wcg;
	int lvpos = UserOp(flags) ? 0 : -1;
	int curtick = GetTickCount();
	unsigned __int32 client = *(unsigned __int32 *)text; 
	LPLVCOLOR lplvColor = (LPLVCOLOR)malloc(sizeof(LVCOLOR));
	lplvColor->cItem1 = self ? 0x00FFB8B8 : GetColorTmp;
	lplvColor->cItem2 = GetPingTextColor(ping, flags);
	lplvColor->cItem3 = GetColorTmp;
	lplvColor->bBold = self;
	int tmpicon = GetIcon(client, flags);

	*(__int32 *)strclient = client;
	fastswap32((unsigned __int32 *)strclient);
	strclient[4] = 0;
	*strclan = 0;
	
	switch (client) {
		case 'STAR':
		case 'SEXP':
			wcg = *(int *)(text + strlen(text) - 4);
			switch (wcg) {
				case 'WCPL':
					tmpicon = 18;
					break;
				case 'WCPG':
					tmpicon = 19;
			}
			break;
		case 'WAR3':
		case 'W3XP':
			if (charcount(text, ' ') == 3) {
				tmp = revstrchr(text, ' ');
				*tmp++ = 0;
				strrevex(tmp, strclan);
			}
			break;
		case 'D2DV':
		case 'D2XP':
			/*if (bot[index]->fstate & BFS_DIABLO2) {
				if (character)
					AddChat(vbGreen, character, bot[index]->hWnd_rtfChat);
			}*/
			break;
	}
	if (!bot[index]->curlvtab)
		InsertItem(user, tmpicon, UserOp(flags) ? 0 : -1, lplvColor, index);
	bot[index]->numusers++;
	UpdatelblChannel(index);

	LPUSER pUser = (LPUSER)malloc(sizeof(USER));
	strncpy(pUser->username, user, sizeof(pUser->username));
	pUser->ping      = ping;
	pUser->flags     = flags;
	pUser->client    = client;
	pUser->jointick  = curtick;
	pUser->lastspoke = curtick;
	pUser->speak     = 0;
	pUser->clan	     = *strclan ? *(int *)strclan : 0;
	pUser->chanpos   = bot[index]->numusers;	  
	//pUser->haschar   = (character && *character);
	pUser->clrstruct = lplvColor;
	InsertValue(user, pUser, bot[index]->users, TABLESIZE_USERS);
	ChainInsertItem(pUser, UserOp(flags), index);

	if ((character && *character))										  
		strncpy(pUser->charname, character, sizeof(pUser->charname));
	else
		pUser->charname[0] = 0;

	bot[index]->lastpersonjoined = pUser;

	if (joined) {
		if ((curtick - bot[index]->lastjointime) < 500) {
			if (!bot[index]->flooded) {
				bot[index]->prefloodc++;
				if (bot[index]->prefloodc > 4) {
					bot[index]->prefloodc = 0;
					bot[index]->flooded   = true;
					bot[index]->floodtime = curtick;
					AddChat(vbYellow, "Flood detected!", bot[index]->hWnd_rtfChat);
				}
			}
		}
		bot[index]->lastjointime = curtick;
	}

	if (self) {				  
		bot[index]->self = pUser;
		bot[index]->bnetflags = flags;
		bot[index]->ping = ping;
		sprintf(sdfg, "Flags: 0x%02x", flags);
		SendMessage(bot[index]->hWnd_Statusbar, SB_SETTEXT, SBPART_FLAGS, (LPARAM)sdfg);
	}

	if (!bot[index]->flooded) {
		if (joined) {
			if (bot[index]->fstate & BFS_USEJOINGREET)
				SendGreet(user, flags, ping, false, index);
			if (!*strclan) {
				if (*pUser->charname)
					sprintf(sdfg, " -- %s (character %s) [%ums] has joined the channel using %s.", user, character, ping, strclient);
				else
					sprintf(sdfg, " -- %s [%ums] has joined the channel using %s.", user, ping, strclient);
			} else {
				sprintf(sdfg, " -- %s [%ums] has joined the channel using %s, in clan %s.", user, ping, strclient, strclan);
			}
			AddChat(vbGreen, sdfg, bot[index]->hWnd_rtfChat);
		}
	} else {
		bot[index]->floodcount++;
	}

	//AddChat(vbBlue, text, bot[index]->hWnd_rtfChat);
	if (!bot[index]->curlvtab) {
		if (gfstate & GFS_UITEXTPING) {
			sprintf(sdfg, "%u", ping);
			InsertSubItem(lvpos, 1, sdfg, -1, index);
		} else {
			InsertSubItem(lvpos, 1, NULL, GetPingIcon(ping, flags), index);
		}
		sprintf(sdfg, "0x%02x", flags);
		InsertSubItem(lvpos, 2, sdfg, -1, index);
		InsertSubItem(lvpos, 3, strclan, -1, index);
	}
	bot[index]->joincount++;
}


void EID_LEAVE(char *user, int flags, int ping, int index) {
	char sdfg[512];

	if (bot[index]->fstate & BFS_DIABLO2) {
		char *tmp = strchr(user, '*');
		if (tmp)
			user = tmp + 1;
	}

	if (!bot[index]->flooded) {
		if (bot[index]->fstate & BFS_USELEAVEGREET)
			SendGreet(user, flags, ping, true, index);
		sprintf(sdfg, " -- %s has left the channel.", user);
		AddChat(vbGreen, sdfg, bot[index]->hWnd_rtfChat);
	}

	LPUSER lpUser = (LPUSER)GetValue(user, bot[index]->users, TABLESIZE_USERS);
	if (!lpUser)
		return;
	
	free(lpUser->clrstruct); 
	ChainRemoveItem(lpUser, index);

	if (!bot[index]->curlvtab) {
		int userindex = LVFindItem(user, index);
		SendMessage(bot[index]->hWnd_lvwChannel, LVM_DELETEITEM, userindex, 0);
	}

	bot[index]->numusers--;
	RemoveValue(user, bot[index]->users, TABLESIZE_USERS);
	UpdatelblChannel(index);
}


void EID_WHISPERFROM(char *user, char *text, unsigned long flags, int index) {
	char sdfg[64];

	if (bot[index]->fstate & BFS_DIABLO2) {
		char *tmp = strchr(user, '*');
		if (tmp)
			user = tmp + 1;
	}

	AppendText(0xD0D0D0, TimeStamp(sdfg), bot[index]->hWnd_rtfChat);
	sprintf(sdfg, "<From: %s> ", user);
	AppendText(GetColor(flags), sdfg, bot[index]->hWnd_rtfChat);
	AppendText(vbGray, text, bot[index]->hWnd_rtfChat);
	AppendText(vbWhite, "\n", bot[index]->hWnd_rtfChat);
}


void EID_TALK(char *user, char *text, unsigned long flags, int index) {
	char sdfg[256];

	bot[index]->talkcount++;

	if (bot[index]->fstate & BFS_DIABLO2) {
		char *tmp = strchr(user, '*');
		if (tmp)
			user = tmp + 1;
	}

	LPUSER lpUser = (LPUSER)GetValue(user, bot[index]->users, TABLESIZE_USERS);
	if (lpUser && (int)lpUser != -1) {
		unsigned long curtick = GetTickCount();
		if (curtick - lpUser->jointick < 600)
			return;
			if (gfstate & GFS_USENUMFILTER) {
				char *asdf = strchr(user, '#');
				if (asdf) {
					asdf++;
					if (atoi(asdf) > numfilter)
						return;
				}
			}
			lpUser->speak++;
			lpUser->lastspoke = curtick;
	}

	AppendText(0xD0D0D0, TimeStamp(sdfg), bot[index]->hWnd_rtfChat);
	sprintf(sdfg, "<%s> ", user);
	AppendText(GetColor(flags), sdfg, bot[index]->hWnd_rtfChat);
	AppendText((flags & 0x01) ? vbRBlue : vbWhite, text, bot[index]->hWnd_rtfChat);
	AppendText(vbWhite, "\n", bot[index]->hWnd_rtfChat);

	if (gfstate & GFS_DECODE) {
		switch (*text) {
			case 'E':
				AppendText(0xD0D0D0, TimeStamp(sdfg), bot[index]->hWnd_rtfChat);
				sprintf(sdfg, "(Hex) <%s> ", user);
				AppendText(0x0000B5, sdfg, bot[index]->hWnd_rtfChat);
				HexToStr(text + 1, sdfg);
				*(__int16 *)(sdfg + strlen(sdfg)) = '\n';
				AppendText(0x707070, sdfg, bot[index]->hWnd_rtfChat);

				break;
			case 'U':
				AppendText(0xD0D0D0, TimeStamp(sdfg), bot[index]->hWnd_rtfChat);
				sprintf(sdfg, "(XR) <%s> ", user);
				AppendText(0x0000B5, sdfg, bot[index]->hWnd_rtfChat);
				XRDecrypt((unsigned char *)text + 1, (unsigned char *)sdfg, xrchannel, xrfactor);
				*(__int16 *)(sdfg + strlen(sdfg)) = '\n';
				AppendText(0x707070, sdfg, bot[index]->hWnd_rtfChat);
				
		}
	}
}


void EID_EMOTE(char *user, char *text, unsigned long flags, int index) {
	char sdfg[386];

	bot[index]->talkcount++;

	if (bot[index]->fstate & BFS_DIABLO2) {
		char *tmp = strchr(user, '*');
		if (tmp)
			user = tmp + 1;
	}

	LPUSER lpUser = (LPUSER)GetValue(user, bot[index]->users, TABLESIZE_USERS);
	if (lpUser) {
		unsigned long curtick = GetTickCount();
		if (curtick - lpUser->jointick > 600) {
			if (gfstate & GFS_USENUMFILTER) {
				char *asdf = strchr(user, '#');
				if (asdf++) {
					if (atoi(asdf) >= numfilter)
						return;
				}
			}
			lpUser->speak++;
			lpUser->lastspoke = curtick;
		}
	}

	sprintf(sdfg, "<%s %s>", user, text);
	AddChat(GetColor(flags), sdfg, bot[index]->hWnd_rtfChat);
}


void EID_CHANNEL(char *text, unsigned long flags, unsigned long ping, int index) {
	char sdfg[128];
	LVBKIMAGE lvimg;

	bot[index]->joincount	= 0;
	bot[index]->talkcount	= 0;
	bot[index]->bancount	= 0;
	bot[index]->lastadded   = NULL;
	bot[index]->firstlvuser = NULL;
	bot[index]->self        = NULL;
	bot[index]->lastpersonjoined = NULL;

	ResetChLVContents(index);
	strcpy(bot[index]->currentchannel, text);
	bot[index]->numusers = 0;
	UpdatelblChannel(index);

	sprintf(sdfg, " -- Joined Channel: %s -- Flags: 0x%02x --", text, flags);
	AddChat(vbGreen, sdfg, bot[index]->hWnd_rtfChat);

	sprintf(sdfg, "Ping: %ums", ping);
	SendMessage(bot[index]->hWnd_Statusbar, SB_SETTEXT, SBPART_PING, (LPARAM)sdfg);
	if (flags & 0x08) {
		if (gfstate & GFS_SHOWVOIDUL) {
			if (*(int *)text == ' ehT')	{
				if (*(int *)(text + 4) == 'dioV') {
					*(int *)sdfg	   = 'snu/';
					*(int *)(sdfg + 4) = 'leuq';
					*(int *)(sdfg + 8) = ' hc';
					strcpy(sdfg + 11, bot[index]->realname);
					Send0x0E(sdfg, index);
				}
			}
		}
	}

	ResetTableContents(bot[index]->users, TABLESIZE_USERS);
	if (gfstate & GFS_UIATMOSPHERE) {
		int style = GetWindowLong(bot[index]->hWnd_rtfChat, GWL_EXSTYLE);

		LPCHANNEL lpChannel = (LPCHANNEL)GetValue(lcase(text), channels, TABLESIZE_CHANNELS);
		if (lpChannel) {
			style |= WS_EX_TRANSPARENT;
			bot[index]->rtbPic = (HBITMAP)LoadImage(g_hInst, lpChannel->rtbPic,
				IMAGE_BITMAP, 480, 400, LR_LOADFROMFILE);
			bot[index]->lblPic = (HBITMAP)LoadImage(g_hInst, lpChannel->lblPic,
				IMAGE_BITMAP, 480, 400, LR_LOADFROMFILE);
			lvimg.ulFlags = LVBKIF_SOURCE_URL | LVBKIF_STYLE_TILE;
			lvimg.pszImage = lpChannel->lvwPic;
		} else {
			if (!(gfstate & GFS_USEGLASS))
				style &= ~WS_EX_TRANSPARENT;
			DeleteObject(bot[index]->rtbPic);
			DeleteObject(bot[index]->lblPic);
			bot[index]->rtbPic = NULL;
			bot[index]->lblPic = NULL;
			lvimg.ulFlags = LVBKIF_SOURCE_NONE;	
		}

		SendMessage(bot[index]->hWnd_lvwChannel, LVM_SETBKIMAGE, 0, (LPARAM)&lvimg);
		SetWindowLong(bot[index]->hWnd_rtfChat, GWL_EXSTYLE, style);
	}

	RedrawWindow(bot[index]->hWnd_main, NULL, NULL,
		RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
}


void EID_FLAGSUPDATE(char *user, char *text, unsigned long flags, unsigned long ping, int index) {
	char asdf[128];
	if (bot[index]->fstate & BFS_DIABLO2) {
		char *tmp = strchr(user, '*');
		if (tmp)
			user = tmp + 1;
	}

	unsigned long client = *(unsigned long *)text;
	
	LPUSER lpUser = (LPUSER)GetValue(user, bot[index]->users, TABLESIZE_USERS);
	if (!lpUser) {
		AddChatf(asdf, hWnd_status_re, vbRed,
			"[%d] Warning! User %s flag update without join!", index, user);
		EID_JOIN(user, text, flags, ping, false, index);
		return;
	}

	int i = LVFindItem(user, index);
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_DELETEITEM, i, 0);
	if (UserOp(flags)) {
		if (!UserOp(lpUser->flags)) {
			sprintf(asdf, "%s has gained operator status.", user);
			AddChat(vbRDBlue, asdf, bot[index]->hWnd_rtfChat);
			i = 0;
			ChainMoveToTop(lpUser, index);
		}
	}

	int GetColorTmp = GetLVColor(flags);
	lpUser->flags = flags;
	lpUser->clrstruct->cItem1 = GetColorTmp;
	lpUser->clrstruct->cItem2 = GetPingTextColor(ping, flags);
	lpUser->clrstruct->cItem3 = GetColorTmp;
	lpUser->clrstruct->bBold = !strcmp(user, bot[index]->realname);

	if (!bot[index]->curlvtab) {
		InsertItem(user, GetIcon(client, flags), i, lpUser->clrstruct, index);
		if (gfstate & GFS_UITEXTPING) {
			sprintf(asdf, "%u", ping);
			InsertSubItem(i, 1, asdf, -1, index);
		} else {
			InsertSubItem(i, 1, NULL, GetPingIcon(ping, flags), index);
		}
		sprintf(asdf, "0x%02x", flags);
		InsertSubItem(i, 2, asdf, -1, index);
	}
}


void EID_INFO(char *data, char *text, int index) {
	int userlen, datalen, textlen;

	userlen = strlen(data + 28);
	datalen = *(unsigned __int16 *)(data + 2);
	textlen = datalen - userlen - 30;

	if (textlen > 16) {
		//AddChat(vbBlue | 0x303030, data + 28, bot[index]->hWnd_rtfChat); ////
		switch (*(unsigned __int32 *)(data + (datalen - 14 - userlen))) {
			case 'ab s':
				bot[index]->bancount++;
				break;
			case 'abnu':
				bot[index]->bancount--;
		}
	}
}


void EID_WHISPERTO(char *user, char *text, unsigned long flags, int index) {
	char sdfg[64];

	if (bot[index]->fstate & BFS_DIABLO2) {
		char *tmp = strchr(user, '*');
		if (tmp)
			user = tmp + 1;
	}

	AppendText(0xD0D0D0, TimeStamp(sdfg), bot[index]->hWnd_rtfChat);
	sprintf(sdfg, "<To: %s> ", user);
	AppendText(vbCyan, sdfg, bot[index]->hWnd_rtfChat);
	//strcat(text, "\n");
	AppendText(vbGray, text, bot[index]->hWnd_rtfChat);
	AppendText(vbWhite, "\n", bot[index]->hWnd_rtfChat);
	strncpy(bot[index]->lastwhispered, user, sizeof(bot[index]->lastwhispered));
}


bool ParseCommand(char *command, int index) {
	bool ret = true;
	char *tmp, buf[256], *args = strchr(++command, ' ');
	if (args)
		*args++ = 0;
	if (!*command)
		return false;

	switch (hash((unsigned char *)lcase(command), -1)) {
		case CMD_VER:
		case CMD_VERSION:
			AddQueue("/me ~:(}{) Horizon " HORIZON_VERSION " (}{):~", index);
			break;
		case CMD_CQ:
			ClearQueue(index);
			AddChat(vbRDBlue, "Queue cleared.", bot[index]->hWnd_rtfChat);
			break;
		case CMD_PROFILE:
			if (args) {
				PROFILEINIT pi;
				if (hWnd_Profile)
					DestroyWindow(hWnd_Profile);
				pi.index = index;
				pi.username = args;
				hWnd_Profile = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_PROFILE),
					hWnd_main, (DLGPROC)ProfileProc, (LONG)&pi); 
				ShowWindow(hWnd_Profile, SW_SHOW);
			}
			break;
		case CMD_FORCEJOIN:
		case CMD_FJOIN:
			if (args) {
				InsertDWORD(0x02);
				InsertNTString(args);
				SendPacket(0x0C, index);
			}
			break;
		case CMD_LEAVECHAT:
			SendPacket(0x10, index);
			break;
		case CMD_RJ:
		case CMD_REJOIN:
			SendPacket(0x10, index);
			InsertDWORD(0x02);
			InsertNTString(bot[index]->currentchannel);
			SendPacket(0x0C, index);
			break;
		case CMD_UPTIME:
			if (args) {
				UptimeCmdHandle(buf, *args, index);
				AddChat(vbRDBlue, buf, bot[index]->hWnd_rtfChat);
			}
			break;
		case CMD_HOME:
			sprintf(buf, "/j %s", bot[index]->homechannel);
			AddQueue(buf, index);
			break;
		case CMD_SPOKE:
			char sdfg[64];
			if (args) {
				LPUSER lpUser;
				lpUser = (LPUSER)GetValue(args, bot[index]->users, TABLESIZE_USERS);
				if (lpUser) {
					GetUptimeString(GetTickCount() - lpUser->lastspoke, sdfg);
					sprintf(buf, "User %s Spoke %d lines, %s ago.",
						args, lpUser->speak, sdfg);
					AddChat(vbRDBlue, buf, bot[index]->hWnd_rtfChat);
				} else {
					AddChat(vbRed, "Cannot find user in channel list!",
						bot[index]->hWnd_rtfChat);
				}
			} else {
				sprintf(buf, "%d lines have been spoken since you joined the channel.",
					bot[index]->talkcount);
				goto addc;
			}
			break;
		case CMD_ACCINFO:
			bot[index]->fstate |= BFS_REQACCINFO;
			Send0x26AccInfo(index);	
			break;
		case CMD_ACCEPT:
			if (args) {
				AddChat(vbRDBlue, *args == 'y' ? "Accepted invitation." :
					"Declined invitation.", bot[index]->hWnd_rtfChat);
				if (bot[index]->fstate & BFS_CREATEINVITE)
					Send0x72(*args, index);
				else 
					Send0x79(*args, index);
			}
			break;
		case CMD_CRASH:
			*(int *)NULL = 1337;
			break;
		case CMD_QUERY:
			QueryFormatVars(buf, args, index);
			//sprintf(buf, "hInstance: 0x%08x | hWnd_main: 0x%08x | "
			//	"current mdi: 0x%08x | current sck: %d | fstate: 0x%08x",
			//	g_hInst,  hWnd_main, bot[index]->hWnd_main, bot[index]->sck,
			//	bot[index]->fstate);
			goto addc;
		case CMD_SETCWD:
			if (args) {
				if (SetCurrentDirectory(args))
					AddChat(vbRDBlue, "Successfully set CWD!", bot[index]->hWnd_rtfChat);
				else
					AddChat(vbRed, "Failed to set CWD!", bot[index]->hWnd_rtfChat);
			}
			break;
		case CMD_ACTIVEPING:
			bot[index]->tmptick = GetTickCount();
			InsertDWORD(bot[index]->platform);
			InsertDWORD(bot[index]->client);
			InsertDWORD(0);
			InsertDWORD(0);
			SendPacket(0x15, index);
			break;
		case CMD_SETNAME:
			tmp = "Username";
			goto thewrite;
		case CMD_SETPASS:
			tmp = "Password";
			goto thewrite;
		case CMD_SETHOME:
			tmp = "Home";
thewrite:
			if (args) {
				WriteStuff(bot[index]->pname, "Main", tmp, args);
				AssignStructFromConfig(index, bot[index]->pname);
				sprintf(buf, "Set %s.", tmp);
addc:
				AddChat(vbRDBlue, buf, bot[index]->hWnd_rtfChat);
			}
			break;
		case CMD_RC:
		case CMD_RECONNECT:
			DisconnectProfile(index, false);
		case CMD_CONNECT:
			ConnectProfile(index);
			break;
		case CMD_DISCONNECT:
			DisconnectProfile(index, true);
			break;
		case CMD_PLAY:
			if (args)
				PlayWinampSong(args);
			break;
		case CMD_STOP: 
			PushWinampButton(WINAMP_STOP);
			break;
		case CMD_MP3:	
			GetWinampSong(buf + 4);
			*(int *)buf = ' em/';
			AddQueue(buf, index);
			break;
		case CMD_PAUSE:
			PushWinampButton(WINAMP_PAUSE);
			break;
		case CMD_VOL:		 
		case CMD_SETVOLUME:
			if (args)
				SetWinampVolume(atoi(args));
			break;
		case CMD_NEXT:
			PushWinampButton(WINAMP_NEXT);
			break;
		case CMD_PREV: 
			PushWinampButton(WINAMP_PREV);
			break;
		case CMD_WINAMP:
			LoadWinamp();
			break;
		case CMD_DISPLIST:
			int i;
			LPUSER lpNext;

			lpNext = bot[index]->firstlvuser;
			for (i = 0; i != bot[index]->numusers; i++) {
				AddChat(vbWhite, lpNext->username, bot[index]->hWnd_rtfChat);
				if (IsBadReadPtr(lpNext->nextlvuser, 32)) {
					sprintf(buf, "Bad read ptr on %s, lpNext->nextlvuser == 0x%08x!",
						lpNext->username, lpNext->nextlvuser);
					AddChat(vbRed, buf, bot[index]->hWnd_rtfChat);
					break;
				} else {
					lpNext = (LPUSER)lpNext->nextlvuser;
				}
			}
			break;
		case CMD_JOINGAME:
			SendPacket(0x10, index);
			Send0x1C(index, false);
			break;
		case CMD_LEAVEGAME:
			Send0x2C(index);
			InsertDWORD(0);
			InsertNTString(bot[index]->currentchannel);
			SendPacket(0x0C, index);
			break;
		case CMD_STARTGAME:	
			Send0x1C(index, true);
			break;
		case CMD_XRENCRYPT:
			XREncrypt((unsigned char *)args, (unsigned char *)buf, xrchannel, xrfactor);
			AddQueue(buf, index);
			break;
		case CMD_XRDECRYPT:
			XRDecrypt((unsigned char *)args, (unsigned char *)buf, xrchannel, xrfactor);
			AddChat(vbGreen, buf, bot[index]->hWnd_rtfChat);
			break;											
		case CMD_CLANRANK:										 
		case CMD_CRANK:
			if ((bot[index]->fstate & BFS_WARCRAFT3) && args) {
				tmp = strchr(args, ' ');
				if (tmp) {
					*tmp++ = 0;
					Send0x7A(args, atoi(tmp), index);
				}
			}
			break;
		case CMD_CHIEFTAIN:
			if (bot[index]->fstate & BFS_WARCRAFT3) {
				if (MessageBox(0, "Are you sure?", "Change chieftain",
					MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					Send0x74(args, index);
			}
			break;
		case CMD_GETMOTD:
			if (bot[index]->fstate & BFS_WARCRAFT3) {
				AddChat(vbRDBlue, "Requesting MOTD...", bot[index]->hWnd_rtfChat);
				InsertDWORD(0x3713);
				SendPacket(0x7C, index);
			}
			break;
		case CMD_CREMOVE:
			if (bot[index]->fstate & BFS_WARCRAFT3)
				Send0x78(args, index);
			break;
		case CMD_CMI:
			AddChat(vbYellow, "Requesting member information...", bot[index]->hWnd_rtfChat);
			Send0x82(args, index);
			break;
		case CMD_OP:
			sprintf(buf, "/designate %c%s",
				(bot[index]->fstate & BFS_DIABLO2) ? '*' : ' ', args);
			Send0x0E(buf, index);
			SendPacket(0x10, index);
			InsertDWORD(0x02);
			InsertNTString(bot[index]->currentchannel);
			SendPacket(0x0C, index);
			break;
		case CMD_DISBAND:
			if (bot[index]->fstate & BFS_WARCRAFT3) {
				if (!strcmp(command, "disband")) {
					if (MessageBox(0, "Disband current clan?", "Disband?",
						MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
						if (MessageBox(0, "Are you actually sure?", "Sure?",
							MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK) {
							AddChat(vbRed, "Disbanding clan.", bot[index]->hWnd_rtfChat);
							InsertDWORD(0x3713);
							SendPacket(0x73, index);
						}
					}

				}
			}
			break;
		case CMD_CHECKCLAN:
			if (bot[index]->fstate & BFS_WARCRAFT3 && args)
				Send0x70(args, index);
			break;
		case CMD_CLEARCHAT:
			SetWindowText(bot[index]->hWnd_rtfChat, NULL);
			AddChat(vbYellow, "Cleared chat.", bot[index]->hWnd_rtfChat);
			break;
		case CMD_SLAP:
			//AddChatf(buf, bot[index]->hWnd_rtfChat, vbGreen, "0x%08x", *(int *)0x7FFE0300);
			if (args) {
				sprintf(buf, "/me slaps %s around a bit with a large trout", args);
				AddQueue(buf, index);
			}
			break;
		case CMD_PACKETLOG:
			if (args)
				loglevel = atoi(args);
			else
				loglevel = 0;

			sprintf(buf, "Set loglevel to %d.", loglevel);
			AddChat(vbRDBlue, buf, bot[index]->hWnd_rtfChat);
			break;
		default:						  
			ret = false;
	}
	if (args)
		*--args = ' ';
	return ret;
}


void CALLBACK ChatIdleTimerProc(int idEvent) {	//////cleanup needed
	char sdfg[256];
	
	bot[idEvent]->idleints++;
	if (bot[idEvent]->idletype) {
		if (bot[idEvent]->idleints == (bot[idEvent]->idleinterval << 2)) {
			switch (bot[idEvent]->idletype) {
				case IDLETYPE_MP3:
					GetWinampSong(sdfg + 4);
					*(int *)sdfg = ' em/';
					break;
				case IDLETYPE_UPTIME:
					UptimeCmdHandle(sdfg + 4, 'a', idEvent);
					*(int *)sdfg = ' em/';
					break;
				case IDLETYPE_MESSAGE:
					FormatText(sdfg, bot[idEvent]->idlemsg, idEvent, NULL);
			}
			AddQueue(sdfg, idEvent);
			bot[idEvent]->idleints = 0;
		}						   
	}
	if (!(bot[idEvent]->idleints & 0x07))
		SendPacket(0x00, idEvent);
}				   


void CALLBACK ChatOtherTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	char sdfg[256];

	idEvent--;
	ChatIdleTimerProc(idEvent);
	if (bot[idEvent]->flooded) {
		if (GetTickCount() - bot[idEvent]->lastjointime > 4000) {
			sprintf(sdfg, "Flood over. Clocked @ %d FPM, %d floods.",
				int((float)bot[idEvent]->floodcount / (float)((bot[idEvent]->lastjointime
				- bot[idEvent]->floodtime) / 1000.f) * 60.f), bot[idEvent]->floodcount);
			AddChat(vbGreen, sdfg, bot[idEvent]->hWnd_rtfChat);
			bot[idEvent]->floodcount = 0;
			bot[idEvent]->flooded = false;
			bot[idEvent]->floodamt = 0;
		} else {
			bot[idEvent]->floodamt++;
		}

		if (bot[idEvent]->floodamt == 7) {
			sprintf(sdfg, "Channel %s @ %s is being massloaded/flooded!",
				bot[idEvent]->currentchannel, GetServer(idEvent, false));
			PopupTrayMsg("Warning", sdfg, NIIF_WARNING);
		}
	}
}


void UptimeCmdHandle(char *buf, char uptimetype, int index) {
	char *tmp = buf;
	switch (uptimetype) {
		case 'a': //all
		case 'A':
			*(int *)buf	= 'tsyS';
			*(int *)(buf + 4) = '  me';
			tmp += 8;
			tmp += GetUptimeString(GetTickCount(), tmp);
			*(int *)tmp = 'C | '; 
			*(int *)(tmp + 4) = 'enno';
			*(int *)(tmp + 8) = 'detc';
			*(int *)(tmp + 12) = '  ';
			tmp += 14;
			tmp += GetUptimeString(GetTickCount() - bot[index]->connectedtick, tmp);
			*(int *)tmp = 'l | ';  
			*(int *)(tmp + 4) = 'edao';
			*(int *)(tmp + 8) = '  d';
			tmp += 11;
		case 'l':
		case 'L':
			GetUptimeString(GetTickCount() - bot[index]->loadedtick, tmp);
			break;
		case 's':
		case 'S':
			GetUptimeString(GetTickCount(), buf);
			break;
		case 'c':
		case 'C':
			GetUptimeString(GetTickCount() - bot[index]->connectedtick, buf);		
	}
}


void SendGreet(char *user, int flags, int ping, bool leave, int index) {
	char asdf[256];
	GREETARGS ga;

	ga.user = user;
	ga.flags = flags;
	ga.ping = ping;
	FormatText(asdf, leave ? bot[index]->leavemsg : bot[index]->greetmsg, index, &ga);
	AddQueue(asdf, index);
}

