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
//#include "game.h"
#include "packetbuffer.h"

char sendbuffer[2048];
unsigned int pbufferlen = 4;


///////////////////////////////////////////////////////////////////////////////


void __fastcall InsertByte(unsigned char data) {
	*(sendbuffer + pbufferlen) = data;
	pbufferlen++;
}


void __fastcall InsertWORD(unsigned short data) {
	*(unsigned short *)(sendbuffer + pbufferlen) = data;
	pbufferlen += 2;
}


void __fastcall InsertDWORD(unsigned long data) {
	*(unsigned long *)(sendbuffer + pbufferlen) = data;
	pbufferlen += 4;
}


void __fastcall InsertNTString(char *data) {
	int datalen = strlen(data);
	strcpy(sendbuffer + pbufferlen, data);
	pbufferlen += datalen;
	sendbuffer[pbufferlen] = 0;
	pbufferlen++;
}


void __fastcall InsertNonNTString(char *data) {
	strcpy(sendbuffer + pbufferlen, data);
	pbufferlen += strlen(data);
}


void __fastcall InsertVoid(void *data, unsigned int len) {
	memcpy(sendbuffer + pbufferlen, data, (size_t)len);
	pbufferlen += len;
}


void __fastcall SendPacket(unsigned char PacketID, int index) {
	char sdfg[64];

	sendbuffer[0] = (unsigned char)0xFF;
	sendbuffer[1] = PacketID;
	*(unsigned short *)(sendbuffer + 2) = pbufferlen;
	send(bot[index]->sck, sendbuffer, pbufferlen, 0);
	bot[index]->sendcount++;

	if (!bot[index]->connected) { 
		sprintf(sdfg, "[BNET] Sending 0x%02x...", (unsigned char)PacketID);
		AddChat(vbYellow, sdfg, bot[index]->hWnd_rtfChat);
	}

	if (loglevel) {
		sprintf(sdfg, "[%02d] [BNET] Sent 0x%02x, len %d",
			index, (unsigned char)PacketID, pbufferlen);
		AddChat(0xFFC080, sdfg, hWnd_status_re);
		if (loglevel > 1)
			StrToHexOut(sendbuffer, pbufferlen, hWnd_status_re);
	}

	pbufferlen = 4; 
}


void __fastcall SendBNLSPacket(unsigned char PacketID, SOCKET sck) {
	//char sdfg[32];
	*(unsigned short *)(sendbuffer + 1) = --pbufferlen;
	sendbuffer[3] = PacketID;
	send(sck, sendbuffer + 1, pbufferlen, 0);
	//sprintf(sdfg, "[BNLS] Sending 0x%02x...", (unsigned char)PacketID);
	//AddChat(vbYellow, sdfg);
	pbufferlen = 4; 
} 	


void __fastcall SendPacketBNFTP(SOCKET sck) {
	pbufferlen -= 2;
	*(unsigned short *)(sendbuffer + 2) = pbufferlen;
	send(sck, sendbuffer + 2, pbufferlen, 0);
	pbufferlen = 4;
}


#if 0
void SendPacketUDP(unsigned char PacketID, unsigned char cls, PPLAYER player, bool repeat) {
	char asdf[32];
	if (!repeat)
		player->sent[cls]++;
	memmove(sendbuffer + 16, sendbuffer + 4, pbufferlen - 4);
	*(unsigned long *)sendbuffer = 0;
	*(unsigned short *)(sendbuffer + 4) = 0;
	*(unsigned short *)(sendbuffer + 6) = pbufferlen + 8;
	*(unsigned short *)(sendbuffer + 8) = player->sent[cls];
	*(unsigned short *)(sendbuffer + 10) = player->recvd[cls];
	*(sendbuffer + 12) = cls; //0; //command class 0, 1, 2 etc
	*(sendbuffer + 13) = PacketID;
	*(sendbuffer + 14) = playerid;
	*(sendbuffer + 15) = 0;
	*(unsigned short *)(sendbuffer + 4) = udpchecksum(sendbuffer + 4);
	if (sendto(udpsck, sendbuffer, pbufferlen + 12, 0,
		(const struct sockaddr *)&player->sin, 0x10) == SOCKET_ERROR) {
		sprintf(asdf, "sendto() failed! WSAGetLastError: %d", WSAGetLastError());
		AddChat(vbRed, asdf);
	}
	sprintf(asdf, "[UDP] Sending 0x%02x...", PacketID);
	AddChat(vbYellow, asdf);
	char dsfarg[1024];
	for (unsigned int i = 0; i != pbufferlen + 12; i++)
		sprintf(dsfarg + (i * 3), "%02x ", (unsigned char)sendbuffer[i]);
	AddChat(vbBlue, dsfarg);
	pbufferlen = 4;
}
#endif

