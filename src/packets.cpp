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
#include "hashing.h"
#include "checkrevision.h"
#include "packetbuffer.h"
#include "connection.h"
#include "chat.h"
#include "clan.h"
#include "gui.h"
#include "bnftp.h"
#include "warden.h"
#include "profile.h"
#include "loaddll.h"
#include "packets.h"

const char *keyresstr[] = {
	"invalid",
	"in use",
	"banned",
	"wrong product"
};


char *cdkey2_resstrs[] = {
	"Invalid CDKey.",
	"Wrong product CDKey.",
	"Banned CDKey.",
	"CDKey In use."
};

const char *verresstr[] = {
	"Old product version.",
	"Invalid product version.",
	"Version must be downgraded."
};

char *p0x07errs[] = {
	"Failed version check.",
	"Old game version.",
	NULL,
	"Reinstall required."
};

char *p0x30errs[] = {
	"Invalid CDKey",
	"Bad product CDKey",
	"Banned CDKey",
	"CDKey in use"
};

char *locations[] = {
	"offline",
	"not in chat",
	"in chat",
	"in a public game",
	"in a private game (non-mutual)",
	"in a private game (mutual)"
};

/*
const char *resstr0x51[] = {
	"Old product version.",
	"Invalid product version.",
	"Version must be downgraded.",
	"Invalid CDKey.",
	"CDKey in use.",
	"Banned CDKey.",
	"Wrong product CDKey.",
	"Invalid expansion CDKey.",
	"Expansion CDKey in use.",
	"Banned expansion CDKey.",
	"Wrong product expansion CDKey."
};
*/


///////////////////////////////////////////////////////////////////////////////


void ParsePacket(unsigned char packetid, char *data, int index) {
	char asdf[128];

	if (loglevel) {
		int len = *(unsigned __int16 *)(data + 2);

		sprintf(asdf, "[%02d] [BNET] Received 0x%02x, len %d",
			index, (unsigned char)packetid, len);
		AddChat(0xFFC080, asdf, hWnd_status_re);
		if (loglevel > 1)
			StrToHexOut(data, len, hWnd_status_re);
	}
	if (!bot[index]->connected) {
		sprintf(asdf, "[BNET] Received 0x%02x!", (unsigned char)packetid);
		AddChat(vbGreen, asdf, bot[index]->hWnd_rtfChat);
	}
	switch (packetid) {
		case 0x00: //14 minutes
			//send(bot[index]->sck, data, 4, 0);
			break;
		case 0x05:
			Parse0x05(data, index);
			break;
		case 0x06:
			Parse0x06(data, index);
			break;
		case 0x07:
			Parse0x07(data, index);
			break;
		case 0x0A:
			Parse0x0A(data, index);
			break;
		case 0x0B:
			break;
		case 0x0F:
			Parse0x0F(data, index);
			break;
		case 0x13:
			AddChat(vbRed, "[BNET] You have flooded out!", bot[index]->hWnd_rtfChat);
			break;
		case 0x15:
			Parse0x15(data, index);
			break;
		case 0x19:
			AddChat(vbRed, data + 8, bot[index]->hWnd_rtfChat);
			break;
		case 0x1C:
			Parse0x1C(data, index);
			break;
		case 0x1D:
			Parse0x1D(data, index);
			break;
		//case 0x22:
		//	Parse0x22(data, index);
		//	break;
		case 0x25: //20 seconds
			if (!bot[index]->connected) {
				if (bot[index]->pingtype == PINGTYPE_NEG1 || bot[index]->pingtype == PINGTYPE_ZERO)
					return;
			}
			bot[index]->sendcount++;
			send(bot[index]->sck, data, 8, 0);
			break;
		case 0x26:
			Parse0x26(data, index);
			break;
		case 0x28:
			Parse0x28(data, index);
			break;
		case 0x29:
			Parse0x29(data, index);
			break;
		case 0x30:
			Parse0x30(data, index);
			break;
		case 0x31:
			Parse0x31(data, index);
			break;
		case 0x36:
			Parse0x36(data, index);
			break;
		case 0x3A:
			Parse0x3A(data, index);
			break;
		case 0x3D:
			Parse0x3D(data, index);
			break;
		case 0x4A:
			AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow, "Optional Work: %s", data + 4);
			break;
		case 0x4C:
			AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow, "Required Work: %s", data + 4);
			break;
		case 0x4E:
			Parse0x4E(data, index);
			break;
		case 0x50:
			Parse0x50(data, index);
			break;
		case 0x51:
			Parse0x51(data, index);
			break;
		case 0x52:
			Parse0x52(data, index);
			break;
		case 0x53:
			Parse0x53(data, index);
			break;
		case 0x54:
			Parse0x54(data, index);
			break;
		case 0x55:
			Parse0x55(data, index);
			break;
		case 0x56:
			Parse0x56(data, index);
			break;
		case 0x59:
			if (bot[index]->fstate & BFS_AUTOREGEMAIL)
				Send0x59(index);
			break;
		case 0x5E:
			Parse0x5E(data, index);
			break;
	    case 0x65:
			Parse0x65(data, index);
			break;
		case 0x66: //friends update
			break;
		case 0x67: //friends add
			break;
		case 0x68:
			Parse0x68(data, index);
			break;
		case 0x69: //friends position change
			Parse0x69(data, index);
			break;
		case 0x70:
			Parse0x70(data, index);
			break;
		case 0x71:
			Parse0x71(data, index);
			break;
		case 0x72:
			Parse0x72(data, index);
			break;	 
		case 0x73:
			AddChat(vbGreen, clanstatus[data[8]], bot[index]->hWnd_rtfChat);
			break;	 
		case 0x74:
			Parse0x74(data, index);
			break;			 
		case 0x75:
			Parse0x75(data, index);
			break;					
		case 0x76:
			Parse0x76(data, index);													  
			break;
		case 0x77:
			Parse0x77(data, index);
			break;
		case 0x78:
			AddChat(vbGreen, clanstatus[data[8]], bot[index]->hWnd_rtfChat);
			break;
		case 0x79:
			Parse0x79(data, index);
			break;
		case 0x7A:
			Parse0x7A(data, index);
			break;
		case 0x7C:
			AddChat(vbGreen, data + 12, bot[index]->hWnd_rtfChat);
			break;
		case 0x7D:
			Parse0x7D(data, index);
			break;
		case 0x7E:
			AddChatf(asdf, bot[index]->hWnd_rtfChat, vbYellow,
				"%s has been removed from the clan.", data + 4);
			break; 
		case 0x7F:
			Parse0x7F(data, index);
			break;
		case 0x81:
			Parse0x81(data, index);
			break;
		case 0x82:
			Parse0x82(data, index);
			break;
		default:
			sprintf(asdf, "[BNET] Unhandled packet 0x%02x received!", (unsigned char)packetid);
			AddChat(vbYellow, asdf, bot[index]->hWnd_rtfChat);
			StrToHexOut(data, *(unsigned short *)(data + 2), bot[index]->hWnd_rtfChat);
	}
}


void Send0x50(int index) {
	InsertDWORD(0);
	InsertDWORD(bot[index]->platform);
	InsertDWORD(bot[index]->client);
	InsertDWORD(bot[index]->verbyte);
	InsertDWORD('enUS');
	InsertDWORD(0);
	InsertDWORD(0);
	InsertDWORD(0);
	InsertDWORD(0);
	InsertNTString(countryabbriv);
	InsertNTString(countryname);
	SendPacket(0x50, index);
	if (bot[index]->pingtype == PINGTYPE_ZERO) {
		InsertDWORD(0);
		SendPacket(0x25, index);
	}
}


void Parse0x50(char *data, int index) {
	char wc3privbuf[10], KeyHash[20], exeInfo[128] = {0};
	char *mpqName = data + 24, *ChecksumFormula = data + 25 + strlen(mpqName);
	unsigned long ProductValue, PublicValue, PrivateValue, Checksum, exeVersion;
	FILETIME ft;

	if (bot[index]->pingtype == PINGTYPE_CUST)
		WaitForPingSpoof(index);

	bot[index]->ClientToken = GetTickCount();	
	bot[index]->ServerToken = *(unsigned long *)(data + 8);
	bot[index]->fstate     |= *(int *)(data + 4) ? BFS_USESRP : 0;
	//udptoken				= *(int *)(data + 12);
	ft.dwLowDateTime        = *(int *)(data + 16);
	ft.dwHighDateTime       = *(int *)(data + 20);

	//AddChatf(exeInfo, hWnd_status_re, vbRDBlue,
	//	"\n  Using %s logon.\n  CheckRevision library: %s.",
	//	bot[index]->fstate & BFS_USESRP ? "SRP" : "X-SHA", mpqName);

	if (!DecodeCDKey(bot[index]->cdkey, &ProductValue, &PublicValue,
		(bot[index]->cdkeylen == 26) ? (unsigned long *)wc3privbuf : &PrivateValue)) {
		AddChat(vbRed, "[BNET] Invalid hash CDKey!", bot[index]->hWnd_rtfChat); 	
		return;
	}

	if (bot[index]->cdkeylen == 26)
		HashWAR3Key(bot[index]->ClientToken, bot[index]->ServerToken,
		ProductValue, PublicValue, wc3privbuf, KeyHash);
	else
		HashCDKey(KeyHash, bot[index]->ClientToken, bot[index]->ServerToken,
		ProductValue, PublicValue, PrivateValue);

	if (!DoCheckRevision(ChecksumFormula, &ft, mpqName, &Checksum, &exeVersion, exeInfo, index)) {
		DisconnectProfile(index, false);
		return;
	}

	if (bot[index]->fstate & (BFS_STARCRAFT | BFS_WARCRAFT3))
		WardenKeyInit(KeyHash, index);

	Send0x51(ProductValue, PublicValue, KeyHash, exeVersion, Checksum, exeInfo, index);
}


void Send0x51(unsigned long ProductValue, unsigned long PublicValue, char *KeyHash,
			  unsigned long exeVersion, unsigned long Checksum, char *exeInfo,
			  int index) {
	InsertDWORD(bot[index]->ClientToken);
	InsertDWORD(exeVersion);
	InsertDWORD(Checksum);
	InsertDWORD(1);
	InsertDWORD(0);

	InsertDWORD(strlen(bot[index]->cdkey));
	InsertDWORD(ProductValue);
	InsertDWORD(PublicValue);
	InsertDWORD(0);
	InsertVoid(KeyHash, 20);

	InsertNTString(exeInfo);
	InsertNTString(bot[index]->cdkeyowner);
	SendPacket(0x51, index);
}


void Parse0x51(char *data, int index) {
	char strerr[64], *exterr;
	int result = *(int *)(data + 4);
	if (!result) {
		if (gfstate & GFS_GETICONS)
			SendPacket(0x2d, index);
		if (!(bot[index]->fstate & BFS_USEPLUG)) {
			InsertDWORD('bnet');
			SendPacket(0x14, index);
		}
		if (bot[index]->fstate & BFS_CHANGEEMAIL) {
			if (*bot[index]->email2)
				Send0x5B(index);
			else
				Send0x5A(index);
			DisconnectProfile(index, true);
			return;
		}
		if (bot[index]->fstate & BFS_USESRP) {
			if (bot[index]->fstate & BFS_FORCECREATE) {
				Send0x52(index);
			} else {
				if (bot[index]->fstate & BFS_CHANGEPASS)
					Send0x55(index);
				else 
					Send0x53(index);
			}
		} else {
			if (bot[index]->fstate & BFS_FORCECREATE) {
				Send0x3D(index);
			} else { 
				if (bot[index]->fstate & BFS_CHANGEPASS)
					Send0x31(index);
				else
					Send0x3A(index);
			}
		}
	} else {
		strcpy(strerr, "[BNET] ");
		if (result & 0x0200) {
			if (result & 0x10)
				strcat(strerr, "Expansion ");
			strcat(strerr, "CDKey ");
			strcat(strerr, keyresstr[result & 3]);
			*(short *)(strerr + strlen(strerr)) = '.';
			if (result & 1) {
				AddChat(vbRed, strerr, bot[index]->hWnd_rtfChat);
				exterr = data + 8;
				if (*exterr) 
					AddChat(vbRed, exterr, bot[index]->hWnd_rtfChat);
				DisconnectProfile(index, false);
				AutoReconnectStart(index, 60);
				return;
			}
		} else if (result & 0x0100) {
			strcat(strerr, verresstr[result & 0x03]);
		}
		AddChat(vbRed, strerr, bot[index]->hWnd_rtfChat);
		exterr = data + 8;
		if (*exterr) 
			AddChat(vbRed, exterr, bot[index]->hWnd_rtfChat);
		DisconnectProfile(index, false);
	}	
}


void Send0x3A(int index) {
	char outbuf[20];
	doubleHashPassword(bot[index]->password, bot[index]->ClientToken, bot[index]->ServerToken, outbuf);
	InsertDWORD(bot[index]->ClientToken);
	InsertDWORD(bot[index]->ServerToken);
	InsertVoid(outbuf, 20);
	InsertNTString(bot[index]->username);
	SendPacket(0x3A, index);
}


void Parse0x3A(char *data, int index) {
	char errstr[64];
	strcpy(errstr, "[BNET] ");
	switch (*(unsigned long *)(data + 4)) {
		case 0:
			Send0x0A(index);
			return;
		case 1:
			AddChat(vbYellow, "[BNET] Account doesn't exist, creating.", bot[index]->hWnd_rtfChat);
			Send0x3D(index);
			return;
		case 2:
			AddChat(vbRed, "[BNET] Logon failed.", bot[index]->hWnd_rtfChat);
			break;
		default:
			AddChat(vbRed, data + 8, bot[index]->hWnd_rtfChat);
	}
	DisconnectProfile(index, false);
}


void Send0x0A(int index) {
	InsertNTString(bot[index]->username);
	InsertByte(0x00);
	SendPacket(0x0A, index);
	if (bot[index]->fstate & BFS_WARCRAFT3) {
		InsertDWORD('WAR3');
		SendPacket(0x0B, index);
	}
	InsertDWORD((gfstate & GFS_JOINPUBCHAN) ? 0x01 : 0x02);
	InsertNTString(bot[index]->homechannel);
	SendPacket(0x0C, index);
}


void Parse0x0A(char *data, int index) {
	TCITEM tc;
	char asdf[64];

	bot[index]->connected = true;
	bot[index]->connectedtick = GetTickCount();
	strncpy(bot[index]->realname, data + 4, 64);
	tc.iImage = STATEICO_CONN;
	tc.mask = TCIF_IMAGE;
	SendMessage(hWnd_Tab, TCM_SETITEM, (WPARAM)GetTabFromIndex(index), (LPARAM)&tc);
	sprintf(asdf, "Username: %s", bot[index]->realname);
	//SendMessage(bot[index]->hWnd_Statusbar, SB_SETTEXT, 0, (LPARAM)"Connected: Yes");
	SendMessage(bot[index]->hWnd_Statusbar, SB_SETTEXT, 0, (LPARAM)asdf);
	
	*(int *)asdf = 'eilC';
	*(int *)(asdf + 4) = ' :tn';
	asdf[8] = 0;
	strcat(asdf, bot[index]->clientstr);
	SendMessage(bot[index]->hWnd_Statusbar, SB_SETTEXT, SBPART_CLIENT, (LPARAM)asdf);
	SendMessage(bot[index]->hWnd_Statusbar, SB_SETICON, SBPART_CLIENT,
		(LPARAM)bnicons[GetIcon(bot[index]->client, 0)]);

	if (gfstate & GFS_SHOWACCINFO) {
		bot[index]->fstate |= BFS_REQACCINFO;
		Send0x26AccInfo(index);	
	}								
	SetTimer(hWnd_main, (index + 1), 15000, (TIMERPROC)ChatOtherTimerProc);
	KillTimer(bot[index]->hWnd_main, index | 0x4000);
	//SetTimer(hWnd_main, (index + 1) | 0x1000, 15000, (TIMERPROC)ChatOtherTimerProc);
}


///////////////////////////////other///////////////////////////////////////


void Send0x3D(int index) {
	char buf[20];
	BSHA1(bot[index]->password, strlen(bot[index]->password), (unsigned long *)buf);
	InsertVoid(buf, 20);
	InsertNTString(bot[index]->username);
	SendPacket(0x3D, index);
}


void Parse0x3D(char *data, int index) {
	unsigned long response = *(unsigned long *)(data + 4);
	char *str;

	switch (response) {
		case 0:
			AddChat(vbGreen, "Account created successfully!", bot[index]->hWnd_rtfChat);
			if (bot[index]->fstate & BFS_USELEGACY)
				Send0x29(index);
			else
				Send0x3A(index);
			return;
		case 2:
			str = "Name contains invalid characters";
			break;
		case 3:
			str = "Name contains a banned word";
			break;
		case 4:
			str = "Account already exists";
			break;
		case 6:
			str = "Name did not contain enough alphanumeric characters";
			break;
		default:
			str = "unknown error";
	}
	AddChat(vbRed, str, bot[index]->hWnd_rtfChat);
	DisconnectProfile(index, false);
}


void Send0x31(int index) {
	char buf[20];
	InsertDWORD(bot[index]->ClientToken);
	InsertDWORD(bot[index]->ServerToken);
	doubleHashPassword(bot[index]->password, bot[index]->ClientToken, bot[index]->ServerToken, buf);
	InsertVoid(buf, sizeof(buf));
	BSHA1(bot[index]->newpass, strlen(bot[index]->newpass), (unsigned long *)buf);
	InsertVoid(buf, sizeof(buf));
	InsertNTString(bot[index]->username);
	SendPacket(0x31, index);
}


void Parse0x31(char *data, int index) {
	if (*(bool *)(data + 4)) {
		strcpy(bot[index]->password, bot[index]->newpass);
		AddChat(vbGreen, "[BNET] Account password change successful!", bot[index]->hWnd_rtfChat);
		if (bot[index]->fstate & BFS_USELEGACY)
			Send0x29(index);
		else 
			Send0x3A(index);
	} else {
		AddChat(vbRed, "[BNET] Account password change failed.", bot[index]->hWnd_rtfChat);
		DisconnectProfile(index, false);
	}
}


void Send0x59(int index) {
	InsertNTString(bot[index]->email1);
	SendPacket(0x59, index);
}


void Send0x5A(int index) {
	InsertNTString(bot[index]->username);
	InsertNTString(bot[index]->email1);
	SendPacket(0x5A, index);
}


void Send0x5B(int index) {
	InsertNTString(bot[index]->username);
	InsertNTString(bot[index]->email1);
	InsertNTString(bot[index]->email2);
	SendPacket(0x5B, index);
}


/////////////////////////////// SRP ///////////////////////////////////////


void Send0x52(int index) {
	if (!bot[index]->nls)
		bot[index]->nls = bncsutil.nls_init(bot[index]->username, bot[index]->password);
	int buflen = strlen(bot[index]->username) + 65; //32 + 32 + 1
	char *buf = (char *)malloc(buflen);
	if (bncsutil.nls_account_create(bot[index]->nls, buf, buflen)) {
		InsertVoid(buf, buflen);
		SendPacket(0x52, index);
	}
	free(buf);
}


void Parse0x52(char *data, int index) {
	char *str, buf[64];
	switch (*(int *)(data + 4)) {
		case 0x00:
			AddChat(vbGreen, "[BNET] Account creation successful.", bot[index]->hWnd_rtfChat);
			Send0x53(index);
			return;
		case 0x04:
			str = "already exists.";
			break;
		case 0x07:
			str = "is too short.";
			break;
		case 0x08:
			str = "contains illegal character.";
			break;
		case 0x09:
			str = "contains illegal word.";
			break;
		case 0x0A:
			str = "contains too few alphanumeric characters.";
			break;
		case 0x0B:
			str = "contains adjacent punctuation characters.";
			break;
		case 0x0C:
			str = "contains too many punctuation characters.";
			break;
		default:
			str = "Unknown account creation failure.";
	}
	strcpy(buf, "[BNET] Name ");
	strcpy(buf + 12, str);
	AddChat(vbRed, buf, bot[index]->hWnd_rtfChat);
	DisconnectProfile(index, false);
}


void Send0x53(int index) {
	char A[32];
	if (!bot[index]->nls)
		bot[index]->nls = bncsutil.nls_init(bot[index]->username, bot[index]->password);
	bncsutil.nls_get_A(bot[index]->nls, A);

	InsertVoid(A, sizeof(A));
	InsertNTString(bot[index]->username);
	SendPacket(0x53, index);
}


void Parse0x53(char *data, int index) {
	char M1[20];
	switch (*(int *)(data + 4)) {
		case 0:										 //b var  |  salt
			bncsutil.nls_get_M1(bot[index]->nls, M1, data + 40, data + 8);
			InsertVoid(M1, sizeof(M1));
			SendPacket(0x54, index);
			break;
		case 1:
			AddChat(vbGreen, "Account doesn't exist, creating.", bot[index]->hWnd_rtfChat);
			Send0x52(index);
			break;
		default:
			DisconnectProfile(index, false);
	}
}


void Parse0x54(char *data, int index) {
	switch (*(int *)(data + 4)) {
		case 0x0E:
			Send0x59(index);
			AddChat(vbYellow, "[BNET] Sending email registration...", bot[index]->hWnd_rtfChat);
		case 0x00:
			FreeNLS(index);
			Send0x0A(index);
			return;
		case 0x02:
			AddChat(vbRed, "[BNET] Invalid password or account closure!", bot[index]->hWnd_rtfChat);
			break;
		case 0x0F:
			AddChat(vbRed, data + 28, bot[index]->hWnd_rtfChat);
			break;
		default:
			AddChat(vbRed, "[BNET] Unhandled logon failure!", bot[index]->hWnd_rtfChat);
	}
	DisconnectProfile(index, false);
}


void Send0x55(int index) {
	char A[32];
	if (!bot[index]->nls)
		bot[index]->nls = bncsutil.nls_init(bot[index]->username, bot[index]->password);
	bncsutil.nls_get_A(bot[index]->nls, A);
	InsertVoid(A, sizeof(A));
	InsertNTString(bot[index]->username);
	SendPacket(0x55, index);
}


void Parse0x55(char *data, int index) {
	char *str, buf[84];	//32 + 32 + 20
	int newnls;
	switch (*(int *)(data + 4)) {
		case 0:
			newnls = bncsutil.nls_account_change_proof(bot[index]->nls, buf,
							bot[index]->newpass, data + 40, data + 8);
			bot[index]->oldnls = bot[index]->nls;
			bot[index]->nls = newnls;
			InsertVoid(buf, sizeof(buf));
			SendPacket(0x56, index);
			return;
		case 1:
			str = "Account doesn't exist.";
			break;
		case 5:
			str = "[BNET] Account upgrade required!";
			break;
		default:
			str = "[BNET] Unknown response.";

	}
	AddChat(vbRed, str, bot[index]->hWnd_rtfChat);
	DisconnectProfile(index, false);
}


void Parse0x56(char *data, int index) {
	switch (*(int *)(data + 4)) {
		case 0:
			if (!bncsutil.nls_check_M2(bot[index]->oldnls, data + 8, NULL, NULL)) {
				AddChat(vbRed, "[BNET] Password proof failed!", bot[index]->hWnd_rtfChat);
				DisconnectProfile(index, false);
				return;
			}
			AddChat(vbGreen, "[BNET] Account password change successful.", bot[index]->hWnd_rtfChat);
			strcpy(bot[index]->password, bot[index]->newpass);
			//disable password changing and write stuff to config!
			Send0x53(index);
			break;
		case 2:
			AddChat(vbRed, "[BNET] Invalid password.", bot[index]->hWnd_rtfChat);
			DisconnectProfile(index, false);
			break;

	}
}


///////////////////////////////legacy////////////////////////////////////


void Send0x05(int index) {
	InsertDWORD(1); //reg version
	InsertDWORD(0); //reg auth
	InsertDWORD(0); //account number
	InsertDWORD(0); //reg token
	InsertNTString("Horizon");
	InsertNTString("BreW");
	SendPacket(0x05, index);
}


void Send0x06(int index) {
	InsertDWORD(bot[index]->platform);
	InsertDWORD(bot[index]->client);
	InsertDWORD(bot[index]->verbyte);
	InsertDWORD(0x00);
	SendPacket(0x06, index);
}


void Send0x12(int index) {
	TIME_ZONE_INFORMATION tzi;
	SYSTEMTIME st;
	FILETIME lft, sft;

	GetTimeZoneInformation(&tzi);

	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &lft);
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &sft);

	InsertVoid(&sft, sizeof(lft));
	InsertVoid(&sft, sizeof(sft));
	InsertDWORD(tzi.Bias);
	InsertDWORD(0); //sys def lcid
	InsertDWORD(0); //user def lcid
	InsertDWORD(0); //user def lang id
	InsertNTString("en");
	InsertNTString(countryname);
	InsertNTString(countryabbriv);
	InsertNTString(countryname);
	SendPacket(0x12, index);
}


void Send0x1E(int index) {
	InsertDWORD(LEGACYSVRVERSION);
	#if LEGACYSVRVERSION == 1
		InsertDWORD(0);			 //Reg version
		InsertDWORD(0xbaadf00d); //Reg auth
	#else
		InsertDWORD(0xbaadf00d); //Reg auth
		InsertDWORD(0);			 //Reg version
	#endif
	InsertDWORD(0xbaadf00d);	 //account number
	InsertDWORD(0);				 //reg token
	InsertNTString("Horizon");
	InsertNTString("BreW");
	SendPacket(0x1E, index);
}


void Parse0x1D(char *data, int index) {
	bot[index]->ServerToken = *(unsigned long *)(data + 8);
}


void Parse0x28(char *data, int index) {
	bot[index]->ServerToken = *(unsigned long *)(data + 4);
}


void Parse0x05(char *data, int index) {
#ifdef SHOW_ACCREG_INFO
	char asdf[256];
	AddChatf(asdf, bot[index]->hWnd_rtfChat, vbCyan,
		"\n   Registration Version: 0x%08x\n"
		"   Registration Authority: 0x%08x\n"
		"   Account Number: 0x%08x\n"
		"   Registration Token: 0x%08x",
		*(int *)(data + 4), *(int *)(data + 8),
		*(int *)(data + 12), *(int *)(data + 16));
#endif
}


void Parse0x06(char *data, int index) {
	unsigned long exeVersion, checksum;
	char exeInfo[128];
	FILETIME ft;
	if (bot[index]->crtype == CRTYPE_NONE) {	   
		Send0x07(0, "", 0, index);
	} else {
		ft.dwLowDateTime  = *(int *)(data + 4);
		ft.dwHighDateTime = *(int *)(data + 8);
		char *mpqName = data + 12;
		char *formula = data + 13 + strlen(mpqName);

		ZeroMemory(exeInfo, sizeof(exeInfo)); 
		if (!DoCheckRevision(formula, &ft, mpqName, &checksum, &exeVersion, exeInfo, index)) {
			DisconnectProfile(index, false);
			return;
		}
		Send0x07(exeVersion, exeInfo, checksum, index);
	}
}


void Send0x07(unsigned long exeVersion, char *exeInfo, unsigned long checksum, int index) {
	InsertDWORD(bot[index]->platform);
	InsertDWORD(bot[index]->client);
	InsertDWORD(bot[index]->verbyte);
	InsertDWORD(exeVersion);
	InsertDWORD(checksum); 
	InsertNTString(exeInfo); 
	SendPacket(0x07, index);
}


void Parse0x07(char *data, int index) {
	int res = *(int *)(data + 4);
	if (res == 2) {
		if (bot[index]->client == 'JSTR') {
			Send0x30(index);
		} else {
			if (bot[index]->fstate & BFS_FORCECREATE) {
				Send0x3D(index);
			} else {
				if (bot[index]->fstate & BFS_CHANGEPASS)
					Send0x31(index);
				else
					Send0x29(index);
			}
		}
	} else {
		AddChat(vbRed, p0x07errs[res], bot[index]->hWnd_rtfChat);
		DisconnectProfile(index, false);
	}
}


void Send0x29(int index) {
	char buf[20];
	bot[index]->ClientToken = GetTickCount() + 7000;
	doubleHashPassword(bot[index]->password, bot[index]->ClientToken, bot[index]->ServerToken, buf);
	InsertDWORD(bot[index]->ClientToken);
	InsertDWORD(bot[index]->ServerToken);
	InsertVoid(buf, 20);
	InsertNTString(bot[index]->username);
	SendPacket(0x29, index);
}


void Parse0x29(char *data, int index) {
	if (*(int *)(data + 4)) {
		Send0x0A(index);
	} else {
		AddChat(vbRed, "Logon failed.", bot[index]->hWnd_rtfChat);
		DisconnectProfile(index, false);
	}
}


void Send0x30(int index) {
	InsertDWORD(0);
	InsertNTString(bot[index]->cdkey);
	InsertNTString(bot[index]->cdkeyowner);
	SendPacket(0x30, index);
}
			
	
void Parse0x30(char *data, int index) {
	int res = *(int *)(data + 4);
	if (res == 1) {
		if (bot[index]->fstate & BFS_FORCECREATE)
			Send0x3D(index);
		else
			Send0x29(index);
	} else {
		AddChat(vbRed, p0x30errs[res - 2], bot[index]->hWnd_rtfChat);
		char *cdkeyowner = data + 8;
		if (*cdkeyowner)
			AddChat(vbRed, cdkeyowner, bot[index]->hWnd_rtfChat);
		DisconnectProfile(index, false);
	}
}


void Send0x36(int index) {
	unsigned long prodval, pubval, privval;
	char keyhash[20];

	bot[index]->ClientToken = GetTickCount();	

	if (!DecodeCDKey(bot[index]->cdkey, &prodval, &pubval, &privval)) {
		AddChat(vbRed, "[BNET] Invalid hash CDKey!", bot[index]->hWnd_rtfChat); 	
		return;
	}
	HashCDKey(keyhash, bot[index]->ClientToken, bot[index]->ServerToken, prodval, pubval, privval);

	InsertDWORD(0);
	InsertDWORD(bot[index]->cdkeylen);
	InsertDWORD(prodval);
	InsertDWORD(pubval);
	InsertDWORD(bot[index]->ServerToken);
	InsertDWORD(bot[index]->ClientToken);
	InsertVoid(keyhash, sizeof(keyhash));
	InsertNTString(bot[index]->cdkeyowner);
	SendPacket(0x36, index);
}


void Parse0x36(char *data, int index) {
	unsigned int response = *(int *)(data + 4);
	if (response == 1) {
		Send0x29(index);
	} else {
		if (response <= 5) {
			AddChat(vbRed, cdkey2_resstrs[response - 2], bot[index]->hWnd_rtfChat);
			if (data[8])
				AddChat(vbRed, data + 8, bot[index]->hWnd_rtfChat);
		}
	}
}


////////////////////////////// other ///////////////////////


void Parse0x15(char *data, int index) {
	char asdf[256];
	sprintf(asdf, "Active advertisement: %s, URL: %s", data + 20, data + 21 + strlen(data + 20));
	AddChat(vbRDBlue, asdf, bot[index]->hWnd_rtfChat);
	sprintf(asdf, "Your active ping to %s is %dms.",
		bot[index]->server, GetTickCount() - bot[index]->tmptick);
	AddChat(vbRDBlue, asdf, bot[index]->hWnd_rtfChat);
}


void Parse0x2D(char *data, int index) {
	FILETIME ft;
	char tmpfile[64], buf[128];
	
	ft.dwLowDateTime  = *(int *)(data + 4);
	ft.dwHighDateTime = *(int *)(data + 8);
	char *filename    = data + 12;
	
	*(int *)(tmpfile)     = 'noci';
	*(int *)(tmpfile + 4) = '\\s';
	strncpy(tmpfile + 6, filename, 64 - 6);

	if (!IsFileValid(tmpfile, &ft)) {
		AddChatf(buf, hWnd_status_re, vbYellow,
			"Icon file %s is missing or out of date, downloading...", tmpfile);
		if (!InitiateDLAndWait(filename, tmpfile, false, index)) {
			AddChat(vbRed, "[BNFTP] Download failed!", bot[index]->hWnd_rtfChat);
			return;
		}
	}

	//load the icon file as the listview imagelist icon set here
}


void Parse0x65(char *data, int index) {
	char asdf[16], sdfg[32];
	if (bot[index]->fstate & BFS_REQFLIST) {
		bot[index]->fstate &= ~BFS_REQFLIST;
		if (bot[index]->curlvtab != LVTAB_FRIENDS)
			return;
		char *tmp = data + 5;
		for (int i = 0; i != data[4]; i++) {
			char *user = tmp;
			char *tttext = (char *)malloc(128);
			LPLVCOLOR lpColor = (LPLVCOLOR)malloc(sizeof(LVCOLOR));
			lpColor->cItem1 = vbWhite;
			lpColor->cItem2 = 0x444444;
			lpColor->cItem3 = (int)tttext;
			lpColor->bBold  = false;
			tmp += strlen(tmp) + 1;
			*(__int32 *)asdf  = *(__int32 *)(tmp + 2);
			InsertItem(user, GetIcon(*(__int32 *)asdf, 0), -1, lpColor, index);
			if (*asdf) {
				sprintf(tttext, "User %s (", user);
				int tmplen = strlen(tttext);
				for (int j = 0; j < 4; j++) {
					if (*tmp & (1 << j)) {
						strcat(tttext, friendstati[j]);
						tmplen = strlen(tttext);
						*(__int32 *)(tttext + tmplen) = ' ,';
					}
				}
				tttext[tmplen] = 0;
				fastswap32((unsigned __int32 *)asdf);
				asdf[4] = 0;	
				sprintf(tttext + tmplen, ") is using %s in %s, %s", asdf, locations[tmp[1]], tmp + 6);
			} else {
				sprintf(tttext, "User %s is offline.", user);
			}
			tmp += 6;
			tmp += strlen(tmp) + 1;
		}
		sprintf(sdfg, "Friends (%d)", data[4]);
		SetWindowText(bot[index]->hWnd_lblChannel, sdfg);
	}
}


void Parse0x4E(char *data, int index) { 
	//char asdf[256];
	StrToHexOut(data, *(unsigned short *)(data + 2), bot[index]->hWnd_rtfChat);
}


void Parse0x67(char *data, int index) {
	/*
		(STRING) 	 Account
		(BYTE)		 Friend Type
		(BYTE)		 Friend Status	
		(DWORD)		 ProductID
		(STRING) 	 Location
	*/
	//TODO: implement this
}


void Parse0x68(char *data, int index) {
	if (bot[index]->curlvtab != LVTAB_FRIENDS)
		return;
	LPLVCOLOR lplvColor = (LPLVCOLOR)GetItemlParam(data[4], bot[index]->hWnd_lvwChannel);  
	if (lplvColor) {
		if (!IsBadReadPtr((void *)lplvColor->cItem3, 128))
			free((void *)lplvColor->cItem3);
		free(lplvColor);
	
	}
	SendMessage(bot[index]->hWnd_lvwChannel, LVM_DELETEITEM, data[4], 0);
}


void Parse0x69(char *data, int index) {
	LVITEM lvi1, lvi2;
	char asdf[64], sdfg[64];
	int tmp;

	if (bot[index]->curlvtab != LVTAB_FRIENDS)
		return;
	ZeroMemory(&lvi1, sizeof(LVITEM));
	lvi1.mask       = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
	lvi1.cchTextMax = sizeof(asdf);
	lvi1.pszText    = asdf;
	lvi1.iItem      = data[4];

	ZeroMemory(&lvi2, sizeof(LVITEM));
	lvi2.mask       = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
	lvi2.cchTextMax = sizeof(sdfg);
	lvi2.pszText    = sdfg;
	lvi2.iItem      = data[5];

	ListView_GetItem(bot[index]->hWnd_lvwChannel, &lvi1);
	ListView_GetItem(bot[index]->hWnd_lvwChannel, &lvi2);

	tmp = lvi1.iItem;
	lvi1.iItem = lvi2.iItem;
	lvi2.iItem = tmp;

	ListView_SetItem(bot[index]->hWnd_lvwChannel, &lvi1);
	ListView_SetItem(bot[index]->hWnd_lvwChannel, &lvi2);
}


void Send0x1C(int index, bool startgame) {
	static int tick = 0;
	char blah[128];
	sprintf(blah, ",44,12,6,5,a,,1,e2431c54,3,,%s\rHahahaha\r", bot[index]->username);
	InsertDWORD(startgame ? 0x0C : 0x00);
	InsertDWORD(tick ? (GetTickCount() - tick) / 1000 : 0);
	InsertWORD(0x02);
	InsertWORD(0x01);
	InsertDWORD(0x01F);
	InsertDWORD(0);
	InsertNTString("testing");
	InsertNTString("");
	InsertNTString(blah);
	SendPacket(0x1C, index);
	if (!tick)
		tick = GetTickCount();
}


void Parse0x1C(char *data, int index) {
	Send0x22(index);
	AddChat(vbGreen, "Joined game~", bot[index]->hWnd_rtfChat);
	StrToHexOut(data, *(short *)(data + 2), bot[index]->hWnd_rtfChat);
}


void Send0x22(int index) {
	InsertDWORD(bot[index]->client);
	InsertDWORD(bot[index]->verbyte);
	InsertNTString("testing");
	InsertNTString("");
	SendPacket(0x22, index);
}


/*
  <score overall="650" units="200" structures="400" resources="50"/>
  <units score="200" produced="4" killed="0" lost="0"/>
  <structures score="400" constructed="1" razed="0" lost="0"/>
  <resources score="50" gas="0" minerals="50" spent="0"/>
*/
typedef union {
	struct {
		int made;
		int killed;
		int lost;
	};
	struct {
		int gas;
		int minerals;
		int spent;
	};
} _iscore;

typedef struct _score {
	_iscore units;
	_iscore structures;
	_iscore resources;
} SCORE, *LPSCORE;


void Send0x2C(int index) {
	/*
	FF 2C AC 01 header
	00 00 00 00 game type
	08 00 00 00 number of results
	03 00 00 00 (1 results
	03 00 00 00 (2
	03 00 00 00 (3
	03 00 00 00 (4
	03 00 00 00 (5
	03 00 00 00 (6
	03 00 00 00 (7
	03 00 00 00 (8
	6D 65 6E 6F 74 2E 6E 6F 6F 62 00 (1 game players
	4E 75 6C 6C 00 (2
	00 (3
	00 (4
	00 (5
	00 (6
	00 (7
	00 (8
	3C 6D 61 70 3E 05 50 79 74 68 6F 6E 20 31  ..<map>.Python 1
	2E 33 3C 2F 6D 61 70 3E 0A 00 map name

	<leagueid>0</leagueid>
	<gameid>0xe2431c54</gameid>
	<race>Terran</race>
	<time>25</time>

	<score overall="650" units="200" structures="400" resources="50"/>
	<units score="200" produced="4" killed="0" lost="0"/>
	<structures score="400" constructed="1" razed="0" lost="0"/>
	<resources score="50" gas="0" minerals="50" spent="0"/>
	*/
	SCORE score;


	char buf[512];
	char *gameresult =
		"<leagueid>0</leagueid>\n<gameid>0xe2431c54</gameid>\n<race>Terran</race>\n"
		"<time>%d</time>\n\n  <score overall=\"%d\" units=\"%d\" structures=\"%d\" "
		"resources=\"%d\"/>\n\n  <units score=\"%d\" produced=\"%d\" killed=\"%d\" "
		"lost=\"%d\"/>\n\n  <structures score=\"%d\" constructed=\"%d\" razed=\"%d\" "
		"lost=\"%d\"/>\n\n  <resources score=\"%d\" gas=\"%d\" minerals=\"%d\" spent=\"%d\"/>\n";

	score.resources.minerals = 4500;
	score.resources.gas = 2500;		
	score.resources.spent = 2000;
	if (index == 1) {
		score.units.made = 100;
		score.units.killed = 30;
		score.units.lost = 20;
		score.structures.made = 20;
		score.structures.killed = 10;
		score.structures.lost = 2;
	} else {
		score.units.made = 70;
		score.units.killed = 20;
		score.units.lost = 30;
		score.structures.made = 10;
		score.structures.lost = 10;
		score.structures.killed = 2;
	}
	
	int totalunits = score.units.made + score.units.killed - score.units.lost; 
	int totalstructures = score.structures.made + score.structures.killed - score.structures.lost;
	int totalresources = score.resources.gas + score.resources.minerals - score.resources.spent;

	sprintf(buf, gameresult, 125,
		totalunits + totalstructures + totalresources, totalunits, totalstructures, totalresources,		
		totalunits, score.units.made, score.units.killed, score.units.lost,
		totalstructures, score.structures.made, score.structures.killed, score.structures.lost,
		totalresources, score.resources.gas, score.resources.minerals, score.resources.spent);

	InsertDWORD(0);
	InsertDWORD(8);
	InsertDWORD(1);
	InsertDWORD(2);
	InsertDWORD(0);
	InsertDWORD(0);
	InsertDWORD(0);
	InsertDWORD(0);
	InsertDWORD(0);
	InsertDWORD(0);
	InsertNTString("l)K-Toe");
	InsertNTString("Watty234");
	InsertNTString("");
	InsertNTString("");
	InsertNTString("");
	InsertNTString("");
	InsertNTString("");
	InsertNTString("");
	InsertNTString("<map>Hahahaha</map>\n");
	InsertNTString(gameresult);
	SendPacket(0x2C, index);
	AddChat(vbGreen, "Sending game result", bot[index]->hWnd_rtfChat);
	SendPacket(0x02, index);
}


//////////////////////////////// bnls ////////////////////////////////////////////


void SendBNLS0x09(char *mpqName, char *ChecksumFormula, SOCKET sbnls, int index) {
	InsertDWORD(GetBNLSClient(bot[index]->client));
	InsertDWORD(ExtractCRMPQNumber(mpqName));
	InsertNTString(ChecksumFormula);
	SendBNLSPacket(0x09, sbnls);
}


void SendBNLS0x18(char *mpqName, char *ChecksumFormula, SOCKET sck, int index) {
	InsertDWORD(GetBNLSClient(bot[index]->client));
	InsertDWORD(ExtractCRMPQNumber(mpqName));
	InsertDWORD(0);
	InsertDWORD(0x3713);
	InsertNTString(ChecksumFormula);
	SendBNLSPacket(0x18, sck);
}


void SendBNLS0x1A(char *mpqName, char *ChecksumFormula, PFILETIME pmpqFiletime, SOCKET sck, int index) {
	InsertDWORD(GetBNLSClient(bot[index]->client));
	InsertDWORD(0);
	InsertDWORD(0x3713);
	InsertVoid(pmpqFiletime, sizeof(FILETIME));
	InsertNTString(mpqName);
	InsertNTString(ChecksumFormula);
	SendBNLSPacket(0x1A, sck);
}

