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
#include "queue.h"


///////////////////////////////////////////////////////////////////////////////


void AddQueue(char *text, int index) {
	if (!bot[index]->queuecount) {
		int time = GetQueueTime(strlen(text), index);
		if (time) {
			SetTimer(hWnd_main, (index + 1) | 0x8000, time, QueueTimerProc);
		} else {
			Send0x0E(text, index);
			return;
		}
	}
	bot[index]->queuecount++;
	bot[index]->queue = (char **)realloc(bot[index]->queue,
							sizeof(char *) * bot[index]->queuecount);
	bot[index]->queue[bot[index]->queuecount - 1] = (char *)malloc(strlen(text) + 3);
	strcpy(bot[index]->queue[bot[index]->queuecount - 1], text);
}


void CALLBACK QueueTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	int index = (idEvent & 0x7FFF) - 1;
	Send0x0E(bot[index]->queue[bot[index]->curqueuepos], index);
	free(bot[index]->queue[bot[index]->curqueuepos]);
	bot[index]->curqueuepos++;
	KillTimer(hWnd_main, idEvent);
	if (bot[index]->curqueuepos < bot[index]->queuecount) {
		SetTimer(hWnd_main, idEvent,
			GetQueueTime(strlen(bot[index]->queue[bot[index]->curqueuepos]), index),
			QueueTimerProc);
	} else {
		bot[index]->curqueuepos = 0;
		bot[index]->queuecount = 0;
	}
}


int GetQueueTime(int len, int index) {
	unsigned int tick = GetTickCount();
	int tmp;
	bot[index]->lastsendlen -= (tick - bot[index]->lastsendtick) / perbyte; //MS_PERBYTE;
	bot[index]->lastsendtick = tick;
	if (bot[index]->lastsendlen < 0) {
		bot[index]->lastsendlen = 0;
		tmp = 0;
	} else {
		tmp = perpacket/*MS_PERPACKET*/ + ((len + bot[index]->lastsendlen) * perbyte/*MS_PERBYTE*/);
	}
	bot[index]->lastsendlen += 50 + len;
	return tmp;
}


void ClearQueue(int index) {
	for (int i = bot[index]->curqueuepos; i != bot[index]->queuecount; i++)
		free(bot[index]->queue[i]);
	bot[index]->curqueuepos = 0;
	bot[index]->queuecount = 0;
	KillTimer(hWnd_main, (index + 1) | 0x8000);
}

