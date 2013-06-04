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
#include "resource.h"
#include "main.h"
#include "fxns.h"
#include "packetbuffer.h"
#include "clan.h"

char *clanranks[] = {
	"Peon (Initiate)",
	"Peon",
	"Grunt",
	"Shaman",
	"Chieftain"
};

char *inviteresstrs[] = {
	"Invitation accepted!",
	"Invalid user.",   		
	NULL,                 
	NULL,				  
	"Invitation declined.",	 
	"Failed to invite user.", 
	NULL,                 
	"Unauthorized to invite.",
	"Cannot invite user.",	
	"Clan is full!"			
};

char *clanloc[] = {
	"offline",
	"online (not in chat)",
	"in a channel",
	"in a public game",
	"<unknown>",
	"in a private game"
};

char *clanstatus[] = {
	"Success.",
	"In use.",
	"Too soon.",
	"Not enough members.",
	"Invitation was declined.",
	"Declined.",
	"Accepted.",
	"Not authorized.",
	"User not found.",
	"Clan is full.",
	"Bad tag.",
	"Bad name.",
	"Not in clan."
};

char *p0x70resstrs[] = {
	"Success!",
	"Clan tag already taken.",
	"Clanned CDKey.",  
	NULL,			
	NULL,				 
	NULL,				  
	NULL,				   
	NULL,					
	"Already in a clan.",
	NULL,
	"Invalid clan tag."
};

int clancreatorindex = -1;


///////////////////////////////////////////////////////////////////////////////


void Send0x70(char *clantag, int index) {
	unsigned __int32 tag = *(unsigned __int32 *)clantag;
	fastswap32(&tag);

	InsertDWORD(0x3713);
	InsertDWORD(tag);
	SendPacket(0x70, index);
}


void Send0x71(char *clantag, char *clanname, char *invited, int numinvited, int index) {
	unsigned __int32 tag = *(unsigned __int32 *)clantag;
	fastswap32(&tag);

	InsertDWORD(0x3713);
	InsertNTString(clanname);
	InsertDWORD(tag);
	InsertByte(numinvited);
	for (int i = 0; i != numinvited; i++)
		InsertNTString(invited + (i << 5));
	SendPacket(0x71, index);
}


void Send0x72(char response, int index) {
	if (bot[index]->invite) {
		InsertDWORD(bot[index]->invite->cookie);
		InsertDWORD(bot[index]->invite->tag);
		InsertNTString(bot[index]->invite->inviter);
		InsertByte(response == 'y' ? 0x06 : 0x04);
		SendPacket(0x72, index);
		
		free(bot[index]->invite);
		bot[index]->invite = NULL;
	} else {
		AddChat(vbRed, "There is no active clan invitation!", bot[index]->hWnd_rtfChat);
	}
}


void Send0x74(char *user, int index) { //change chieftain
	InsertDWORD(0x3713);
	InsertNTString(user);
	SendPacket(0x74, index);
}


void Send0x77(char *user, int index) { //clan join invite
	InsertDWORD(0x3713);
	InsertNTString(user);
	SendPacket(0x77, index);
}


void Send0x78(char *user, int index) { //remove member
	InsertDWORD(0x3713);
	InsertNTString(user);
	SendPacket(0x78, index);
}


void Send0x79(char response, int index) {
	if (bot[index]->invite) {
		InsertDWORD(bot[index]->invite->cookie);
		InsertDWORD(bot[index]->invite->tag);
		InsertNTString(bot[index]->invite->inviter);
		InsertByte(response == 'y' ? 0x06 : 0x04);
		SendPacket(0x79, index);

		free(bot[index]->invite);
		bot[index]->invite = NULL;
	} else {
		AddChat(vbRed, "There is no active clan invitation!", bot[index]->hWnd_rtfChat);
	}
}


void Send0x7A(char *user, unsigned char newrank, int index) {
	InsertDWORD(0x3713);
	InsertNTString(user);
	InsertByte(newrank);									 
	SendPacket(0x7A, index);
}


void Send0x7D(int index) {
	InsertDWORD(0x3713);
	SendPacket(0x7D, index);
}


void Send0x82(char *user, int index) {
	InsertDWORD(0x3713);
	InsertDWORD(0); //clan tag
	InsertNTString(user);
	SendPacket(0x82, index);
}


///////////////////////////////////////////////////////////////////////////////


void Parse0x70(char *data, int index) {
	char asdf[64];
	char *curdata = data + 10;
	HWND tmphwnd;
	LVITEM lvi;
	if (bot[index]->fstate & BFS_REQCLANCREATE)	{
		bot[index]->fstate &= ~BFS_REQCLANCREATE;
		tmphwnd = GetDlgItem(hWnd_ClanCreate, CLAN_LBLSTATUS);
		if (data[8]) {
			SetWindowText(tmphwnd, p0x70resstrs[data[8]]);
			return;
		}
		sprintf(asdf, "Found %d candidates.", data[9]);
		SetWindowText(tmphwnd, asdf);
		tmphwnd = GetDlgItem(hWnd_ClanCreate, CLAN_LVWCANDIDATES);
		
		ListView_DeleteAllItems(tmphwnd);
		ZeroMemory(&lvi, sizeof(LVITEM));
		lvi.mask = LVIF_TEXT;
		for (int i = 0; i != data[9]; i++) {
			
			lvi.pszText = curdata;
			ListView_InsertItem(tmphwnd, &lvi);
			curdata += strlen(curdata) + 1;
		}
	} else {
		AddChat(vbRDBlue, p0x70resstrs[data[8]], bot[index]->hWnd_rtfChat);
		if (data[8])
			return;

		if (data[9])
			AddChatf(asdf, bot[index]->hWnd_rtfChat, vbGreen, "Found %d candidates:", data[9]);
		else
			AddChat(vbRed, "Found no candidates.", bot[index]->hWnd_rtfChat);

		for (int i = 0; i != data[9]; i++) {
			AddChat(vbRDBlue, curdata, bot[index]->hWnd_rtfChat);
			curdata += strlen(curdata) + 1;
		}
	}
}


void Parse0x71(char *data, int index) {
	char asdf[64];
	if (data[8]) {
		switch (data[8]) {
			case 4:
				AddChat(vbRed, "Declined", bot[index]->hWnd_rtfChat);
				break;
			case 5:
				AddChat(vbRed, "Not available", bot[index]->hWnd_rtfChat);
				break;
			default:
				AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow,
					"Unrecognized clan creation failure code: 0x%02x.",
					(unsigned char)data[8]);
		}
		unsigned short length = *(unsigned short *)(data + 2);
		int pos = 9;
		while (pos <= length) {
			AddChat(vbRed, data + pos, bot[index]->hWnd_rtfChat);
			pos += strlen(data + pos) + 1;
		}
	} else {
		AddChat(vbGreen, "Successfully created clan!", bot[index]->hWnd_rtfChat);
	}
}


void Parse0x72(char *data, int index) {
	char asdf[128], clanstr[8];

	LPCLANJOINREQ cjr = (LPCLANJOINREQ)malloc(sizeof(CLANJOINREQ));
	cjr->cookie   = *(unsigned __int32 *)(data + 4);
	cjr->tag      = *(unsigned __int32 *)(data + 8);

	*(__int32 *)clanstr = *(__int32 *)(data + 8);
	fastswap32((unsigned __int32 *)clanstr);
	clanstr[4] = 0;

	char *clanname = data + 12;

	char *invitinguser = clanname + strlen(clanname) + 1;
	strncpy(cjr->inviter, invitinguser, sizeof(cjr->inviter));

	AddChatf(asdf, bot[index]->hWnd_rtfChat, vbGreen,
		"%s has invited you to create %s (Tag: %s). Type /accept [y/n] to respond.",
		invitinguser, clanname, clanstr);

	if (bot[index]->invite)
		free(bot[index]->invite);
	bot[index]->invite = cjr;

	char *curpos = invitinguser + strlen(invitinguser) + 1;
	int numinvited = *curpos;
	curpos++;
	for (int i = 0; i != numinvited; i++) {
		AddChat(vbYellow, curpos, bot[index]->hWnd_rtfChat);
		curpos += strlen(curpos) + 1;
	}
	AddChatf(asdf, bot[index]->hWnd_rtfChat, vbGreen,
		"(%d others) have also been invited.", numinvited);
	bot[index]->fstate |= BFS_CREATEINVITE;
}


void Parse0x74(char *data, int index) {	//New chief result
	if (data[8])
		AddChat(vbGreen, "Chieftain successfully changed.", bot[index]->hWnd_rtfChat);
	else
		AddChat(vbRed, "Failed to change chieftain.", bot[index]->hWnd_rtfChat);
}


void Parse0x75(char *data, int index) { //Clan info
	char asdf[128];
	bot[index]->clan = *(__int32 *)(data + 5);
	*(__int32 *)bot[index]->clanstr = *(__int32 *)(data + 5);
	fastswap32((unsigned __int32 *)bot[index]->clanstr);
	bot[index]->clanstr[4] = 0;
	sprintf(asdf, "You are a %s in Clan %s.", clanranks[data[9]], bot[index]->clanstr);
	AddChat(vbGreen, asdf, bot[index]->hWnd_rtfChat);
}


void Parse0x76(char *data, int index) { //Clan removal
	bot[index]->clan = 0;
	*bot[index]->clanstr = 0;
	AddChat(vbRed, "You have been removed from the clan!", bot[index]->hWnd_rtfChat);
}


void Parse0x77(char *data, int index) { //Invite result
	AddChat(data[8] ? vbRed : vbGreen, inviteresstrs[data[8]], bot[index]->hWnd_rtfChat);
}


void Parse0x78(char *data, int index) { //Clan member remove result
	AddChat(data[8] ? vbRed : vbGreen, clanstatus[data[8]], bot[index]->hWnd_rtfChat);
}


void Parse0x79(char *data, int index) {
	char clanstr[8], tmp[256];

	LPCLANJOINREQ cjr = (LPCLANJOINREQ)malloc(sizeof(CLANJOINREQ));
	cjr->cookie   = *(unsigned __int32 *)(data + 4);
	cjr->tag      = *(unsigned __int32 *)(data + 8);
	
	*(__int32 *)clanstr = *(__int32 *)(data + 8);
	fastswap32((unsigned __int32 *)clanstr);
	clanstr[4] = 0;

	char *clanname = data + 12;
	char *invitinguser = clanname + strlen(clanname) + 1;
	strncpy(cjr->inviter, invitinguser, sizeof(cjr->inviter));

	if (bot[index]->invite)
		free(bot[index]->invite);
	bot[index]->invite = cjr;

	sprintf(tmp, "%s has invited you to join %s (Tag: %s). Type /accept [y/n] to respond.",
		invitinguser, clanname, clanstr);
	AddChat(vbGreen, tmp, bot[index]->hWnd_rtfChat);
}


void Parse0x7A(char *data, int index) {
	AddChat(vbRDBlue, clanstatus[data[8]], bot[index]->hWnd_rtfChat);
}


void Parse0x7D(char *data, int index) {	
	char asdf[16], sdfg[64];
	if (bot[index]->fstate & BFS_REQCLANINFO) {
		bot[index]->fstate &= ~BFS_REQCLANINFO;
		register char *tmp = data + 9;
		sprintf(asdf, "Members: %d", data[8]);
		SetWindowText(bot[index]->hWnd_lblChannel, asdf);
		for (int i = 0; i != data[8]; i++) {
			char *user = tmp;
			char *sdfg = (char *)malloc(64);
			LPLVCOLOR lpColor = (LPLVCOLOR)malloc(sizeof(LVCOLOR));
			lpColor->cItem1 = vbWhite;
			lpColor->cItem2 = 0x444444;
			lpColor->cItem3 = (int)sdfg;
			lpColor->bBold  = false;
			tmp += strlen(tmp) + 1;
			char rank = *tmp;				      
			int tmprank = -(rank - 3) + 1; 
			InsertItem(user, 19 + rank + !rank, tmprank | (-1 & -(tmprank > 1)), lpColor, index);
			
			sprintf(sdfg, "User %s [%s] is o%sline.",
				user, clanranks[*tmp], tmp[1] ? "n" : "ff");
			//strncpy((char *)lpColor->cItem3 + 2, tmp, 62); //// is there ever a location? 
			tmp += 2;
			tmp += strlen(tmp) + 1;
		}      
		sprintf(sdfg, "Clan %s (%d members)", bot[index]->clanstr, data[8]);
		SetWindowText(bot[index]->hWnd_lblChannel, sdfg);
	}
}


void Parse0x7F(char *data, int index) {
	char asdf[128];
	char *afteruser	= data + strlen(data + 4) + 5;
	AddChatf(asdf, bot[index]->hWnd_rtfChat,
		vbYellow, "%s (%s) is now %s (%s).",
		data + 4, clanranks[*afteruser], clanloc[afteruser[1]], afteruser + 2);
}


void Parse0x81(char *data, int index) {
	char asdf[128];
	char *prefix;
	int color;
	if (data[5] > data[4]) {
		color = vbGreen;
		prefix = "pro";
	} else {
		color = vbRed;
		prefix = "de";
	}
	AddChatf(asdf, bot[index]->hWnd_rtfChat, color, 
		"%s has %smoted you from %s to %s.",
		data + 6, prefix, clanranks[data[4]], clanranks[data[5]]);
}


void Parse0x82(char *data, int index) {
	/*
		(DWORD)		 Cookie
		(BYTE)		 Status code
		(STRING) 	 Clan name
		(BYTE)		 User's rank
		(FILETIME)	 Date joined
	*/
	
	char asdf[128];
	if (data[8]) {
		char *posafter = data + 10 + strlen(data + 9);
		AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow, "%s | %s | %u %u",
			data + 9, clanranks[data[8]], *(int *)(posafter + 1), *(int *)(posafter + 5)); 
	} else {
		AddChat(vbRed, "Clan member info request failed.", bot[index]->hWnd_rtfChat);
	}
}


///////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK ClanCreateProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {  
	char asdf[64];
	HWND hwndtmp;
	switch (message) {
		case WM_INITDIALOG:
			hWnd_ClanCreate = hDialog;
			clancreatorindex = lParam;

			hwndtmp = GetDlgItem(hDialog, CLAN_LVWCANDIDATES);

			LVCOLUMN lvc;
			ZeroMemory(&lvc, sizeof(LVCOLUMN));
			lvc.mask = LVCF_TEXT;
			lvc.pszText = "Potential Candidates";
			lvc.cx = 200;			
			SendMessage(hwndtmp, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

			SendMessage(hwndtmp, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case CLAN_CMDCHECK:
					bot[clancreatorindex]->fstate |= BFS_REQCLANCREATE;
					GetWindowText(GetDlgItem(hDialog, CLAN_TXTTAG), asdf, sizeof(asdf));
					Send0x70(asdf, clancreatorindex);
					break;
				case CLAN_CMDCREATE:
					SendCreationInvites(hDialog);
					break;
				case CLAN_CMDCANCEL:
					DestroyWindow(hDialog);
			}
			break; 
		case WM_DESTROY:
			hWnd_ClanCreate = NULL;
			clancreatorindex = -1;
			break;
		case WM_CLOSE:
			DestroyWindow(hDialog);
			break;
		default:
			DefWindowProc(hDialog, message, wParam, lParam);
    }
	return false;
}


void SendCreationInvites(HWND hDialog) {
	LVITEM lvi;
	char asdf[32];
	char sdfg[64];
	HWND hwndtmp = GetDlgItem(hDialog, CLAN_LVWCANDIDATES);
	int numitems = ListView_GetItemCount(hwndtmp);	
	int i, checked = 0;
	for (i = 0; i != numitems; i++) {
		if (ListView_GetCheckState(hwndtmp, i))
			checked++;		
	}
	if (checked < 9) {
		SetWindowText(GetDlgItem(hDialog, CLAN_LBLSTATUS),
			"You require 9 or more candidates selected!");
		return;
	}
		
	ZeroMemory(&lvi, sizeof(LVITEM));
	lvi.pszText = asdf;
	lvi.cchTextMax = sizeof(asdf);
	lvi.mask = LVIF_TEXT;
	char *invited = (char *)malloc(checked << 5);
	for (i = 0; i != checked; i++) {
		ListView_GetItem(hwndtmp, &lvi);
		strncpy(invited + (i << 5), asdf, 32);
		lvi.iItem++;
	}
	GetWindowText(GetDlgItem(hDialog, CLAN_TXTTAG), asdf, sizeof(asdf));
	GetWindowText(GetDlgItem(hDialog, CLAN_TXTNAME), sdfg, sizeof(sdfg));
	Send0x71(asdf, sdfg, invited, checked, clancreatorindex);
}

