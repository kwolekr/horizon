/*-
 * Copyright (c) 2009 Ryan Kwolek 
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
#include "fxns.h"//removable
#include <zlib.h>
#pragma comment(lib, "zdll.lib")

#include "warden.h"

/*
seg000:00000078                 dd 0B000h               ; cbAllocSize
seg000:0000007C dword_7C        dd 4832h                ; offsetFunc1
seg000:00000080 off_80          dd 0A000h               ; reloc function table
seg000:00000084                 dd 14Dh                 ; reloc count
seg000:00000088                 dd 8758h                ; warden setup func
seg000:0000008C                 dd 1                    ; 1
seg000:00000090                 dd 1                    ; 1
seg000:00000094                 dd 843Ch                ; import address table
seg000:00000098                 dd 2                    ; import dll count
seg000:0000009C                 dd 3                    ; section count
seg000:000000A0                 dd 1000h                ; section length?
seg000:000000A4                 dd offset loc_697E      ; exception handler
seg000:000000A8                 dd 20h

seg000:000000AC                 dd 8000h                ; import addresses offest
seg000:000000B0                 dd 767h
seg000:000000B4                 dd 2

seg000:000000B8                 dd 9000h                ; that data
seg000:000000BC                 dd 448h
seg000:000000C0                 dd 4
*/


char *wextracterrstrs[] = {
	"Module MD5 hash mismatch",
	"Module misdecryption, SIGN doesn't match",
	"zlib!uncompress() failed",
	"kernel32!CreateFile() failed",
	"kernel32!WriteFile() failed"
};

SNPFNTABLE snpFnTable = {
	WdnCbkSendPacket,
	WdnCbkCheckModule,
	WdnCbkLoadModule,
	WdnCbkMemAlloc,
	WdnCbkMemFree,
	WdnCbkSetRC4,
	WdnCbkGetRC4
};

void *lpWdnModClass;
LPWARDENFNTABLE lpwdnFnTable;

void *snpTable = &snpFnTable;

char modhash[16];
char modname[64];
char moddecryptkey[16];
int modsize;
char *rawmodule;
int moddlprog;
void *currentmodule;
unsigned long startedtick;
unsigned long wdnkeyseed;

char *newrc4;
char *pendingwpacket;
int pendingwpacketlen;

int dling;


///////////////////////////////////////////////////////////////////////////////


void Parse0x5E(char *data, int index) {
	char buf[128];
	unsigned int len = *(short *)(data + 2) - 4;
	data += 4;
	RC4Crypt(bot[index]->wardenkey_in, (unsigned char *)data, len);
	switch (data[0]) {
		case 0:	  
			WardenParseCommand0(data, len, index);
			break;
		case 1:
			WardenParseCommand1(data, len, index);
			break;
		case 2:
			WardenParseCommand2(data, len, index);
			break;				
		case 5:
			WardenParseCommand5(data, len, index);
			break;
		default:
			sprintf(buf, "Unhandled warden command 0x%02x!", (unsigned char)data[0]);
			AddChat(vbRed, buf, bot[index]->hWnd_rtfChat);
			StrToHexOut(data, len, bot[index]->hWnd_rtfChat);
	}
}


/////////////////////////////////[Command Handlers]//////////////////////////////////


void WardenParseCommand0(char *data, int len, int index) {
	char buf[128];
	unsigned char outbuf;
	*(__int32 *)(modname) = 'udom';	
	*(__int32 *)(modname + 4) = '\\sel';
	for (int i = 0; i != 16; i++)
		sprintf(modname + 8 + (i << 1), "%02x", (unsigned char)data[i + 1]);
	modname[40] = '.';
	*(int *)(modname + 41) = 'dom';
	sprintf(buf, "Warden module %s requested.", modname + 8);
	AddChat(vbYellow, buf, bot[index]->hWnd_rtfChat);
	if (currentmodule) {
		if (cmphash(modhash, data + 1)) {
			AddChat(vbGreen, "Module already loaded, using that.", bot[index]->hWnd_rtfChat);
			goto send1;
		} else {
			WardenUnloadModule();
			AddChat(vbYellow, "New warden module requested, loading that.", bot[index]->hWnd_rtfChat);
			goto contwprocessing;
		}
	} else {
contwprocessing:
		__asm {
			cld
			mov esi, data
			inc esi
			lea edi, [modhash]
			movsd
			movsd
			movsd
			movsd
			lea edi, [moddecryptkey]
			movsd
			movsd
			movsd
			movsd
			lea edi, [modsize]
			movsd
		}  
		/**(__int32 *)(modhash)        = *(__int32 *)(data + 1);
		*(__int32 *)(modhash + 0x04) = *(__int32 *)(data + 5);
		*(__int32 *)(modhash + 0x08) = *(__int32 *)(data + 9);
		*(__int32 *)(modhash + 0x0C) = *(__int32 *)(data + 13);

		*(__int32 *)(moddecryptkey)	       = *(__int32 *)(data + 17);
		*(__int32 *)(moddecryptkey + 0x04) = *(__int32 *)(data + 21);
		*(__int32 *)(moddecryptkey + 0x08) = *(__int32 *)(data + 25);
		*(__int32 *)(moddecryptkey + 0x0C) = *(__int32 *)(data + 29);

		modsize = *(__int32 *)(data + 33);
		*/
		if (GetFileAttributes(modname) == INVALID_FILE_ATTRIBUTES) {
			if (dling) {
				AddChat(vbYellow, "Module already being downloaded, waiting!", bot[index]->hWnd_rtfChat); 
				goto send1;
			}
			AddChat(vbYellow, "Module not found, downloading...", bot[index]->hWnd_rtfChat);
			rawmodule = (char *)malloc(modsize);
			moddlprog = 0;
			outbuf = 0;
			startedtick = GetTickCount();
			dling = 1;
		} else {
			WardenPrepareModule(modname);
			WardenModuleInitalize((LPWMODHEADER)currentmodule);
send1:
			outbuf = 1;
		}
	}
	RC4Crypt(bot[index]->wardenkey_out, &outbuf, 1);
	InsertByte(outbuf);
	SendPacket(0x5E, index);
}


void WardenParseCommand1(char *data, int len, int index) {
	char buf[128];
	if (rawmodule) {					
		memcpy(rawmodule + moddlprog, data + 3, *(__int16 *)(data + 1));
		moddlprog += *(__int16 *)(data + 1);
		//sprintf(buf, "%d/%d bytes (%d%%)", moddlprog, modsize,
		//	int((float)moddlprog / (float)modsize * 100.f));
		//AddChat(vbGreen, buf, bot[index]->hWnd_rtfChat);
	}		
	if (moddlprog >= modsize) {
		dling = 0;
		sprintf(buf, "Module downloaded, dl @ %d kB/s",
			(int)((float)(modsize) / (float)(GetTickCount() - startedtick)));
		AddChat(vbGreen, buf, bot[index]->hWnd_rtfChat);
		int fail = WardenDecryptInflateModule(modname, rawmodule, modsize, moddecryptkey, index);
		if (fail) {																					
			sprintf(buf, "Warden module decryption/inflation failure %d (%s).", 
				fail, wextracterrstrs[fail - 1]);
			AddChat(vbRed, buf, bot[index]->hWnd_rtfChat);
		} else {
			WardenPrepareModule(modname);
			WardenModuleInitalize((LPWMODHEADER)currentmodule);
			*buf = 1;
			RC4Crypt(bot[index]->wardenkey_out, (unsigned char *)buf, 1);
			InsertByte(*buf);
			SendPacket(0x5E, index);
		}
	}
}


void *GetAddress(void *addr, char **mods, int modindex, int nummods, bool war3) {
	/* lpModuleImage[]
		0 - starcraft.exe
		1 - storm.dll
		2 - battle.snp
		
		3 - war3.exe
		4 - storm.dll
		5 - game.dll
	*/

	if (modindex > nummods)
		return 0;
	int wdnmodsnum;
	if (modindex) {	//module
		switch (*(int *)mods[modindex - 1]) {
			case 'emag': //game.dll
			case 'ttab': //battle.snp
				wdnmodsnum = 2;
				break;
			case 'rots': //storm.dll
				wdnmodsnum = 1;
				break;	
			default:
				return 0;
		}
	} else { //absolute addr
		if ((int)addr >= 0x400000 && (int)addr <= 0x600000)	{
			wdnmodsnum = 0;
			addr = (void *)((int)addr - 0x400000);
		} else {
			return 0;
		}
	}
	if (war3) 
		wdnmodsnum += 3;
	if (!lpModuleImage[wdnmodsnum]) {
		char blah[256], sdfg[16], asdf[MAX_PATH];
		sprintf(sdfg, "%sHash%d", war3 ? "WAR3" : "STAR", (wdnmodsnum % 3) + 1);
		GetStuffConfig("Hashes", sdfg, blah);
		AddChatf(asdf, hWnd_status_re, vbYellow, "Loading %s...", blah);
		lpModuleImage[wdnmodsnum] = (char *)((int)LoadLibraryEx(blah, NULL,
										LOAD_LIBRARY_AS_DATAFILE) & 0xFFFFF000);
		if (!lpModuleImage[wdnmodsnum])
			AddChatf(asdf, hWnd_status_re, vbRed, "Failed to load [%s] as datafile!", blah);
	}
	return lpModuleImage[wdnmodsnum] + (int)addr;
}

					
void WardenParseCommand2(char *data, int len, int index) {
	char buf[512];
	char asdf[64];
	char *tosend = buf + 7;
	char *mods[8];
	void *address;
	int pos = 1, nummods = 0, length, lentoread;
	ZeroMemory(buf, sizeof(buf));
	//StrToHexOut(data, len, hWnd_status_re);
	while (data[pos]) {
		mods[nummods] = (char *)malloc(data[pos] + 1);
		memcpy(mods[nummods], data + pos + 1, data[pos]);
		*(mods[nummods] + data[pos]) = 0;
		nummods++;
		pos = pos + data[pos] + 1;
		if (nummods == 8)
			break;
	}
	pos++;
	int wc3 = ((bot[index]->fstate & BFS_WARCRAFT3) && 1); ///////////////////
	//if (!lpModuleImage[wc3]) {
	//	AddChat(vbRed, "Requested module is not loaded!", bot[index]->hWnd_rtfChat);
	//	goto freenret;
	//}																 
	while (pos + 1 < len) { 
		//data[i] ^= data[len - 1];
		if (pos >= 256) {
			AddChat(vbRed, "WARNING! WARDEN BUFFER OVER THRESHHOLD!", bot[index]->hWnd_rtfChat);
			break;
		}
		pos++; //skip command id
		if ((unsigned char)data[pos] <= nummods && !data[pos + 4]) { //memcheck
			/*
			(BYTE) String Index
			(DWORD) Address 
			(BYTE) Length to Read 
			*/
			//mods[data[pos]];
		
			//address = *(__int32 *)(data + pos + 1) - (data[pos] ? 0 : 0x400000)	+ lpModuleImage[wc3];
			address = GetAddress(*(void **)(data + pos + 1), mods, data[pos], nummods, wc3);
			if (address) {
				lentoread = data[pos + 5];
				if (IsBadReadPtr(address, lentoread)) {
					AddChatf(asdf, hWnd_status_re, vbRed,
						"[%d] Unreadable memcheck, addr 0x%08x, len %d. at pos %d!",
						index, address, lentoread, pos);
					StrToHexOut(data, len, hWnd_status_re);
					goto pagecheck;
				}
				*tosend++ = 0;
				memcpy(tosend, address, lentoread);
				tosend += lentoread;
			} else {
				AddChatf(asdf, hWnd_status_re, vbRed,
					"[%d] Unreadable memcheck, addr 0x%08x, len %d. at pos %d!",
					index, address, lentoread, pos);
				StrToHexOut(data, len, hWnd_status_re);
				goto pagecheck;
			}
			pos += 6;
		} else { //pagecheck
pagecheck:
			/*
			(DWORD) Seed 
			(DWORD)[5] SHA1 
			(DWORD) Address 
			(BYTE) Length to Read
			*/
			*tosend = (char)0xE9; //0;
			tosend++;
			pos += 29;
		}
	}
	length = tosend - buf;
	*buf = 2;
	*(__int16 *)(buf + 1) = length - 7;
	*(__int32 *)(buf + 3) = WardenGenerateChecksum(buf + 7, length - 7);
	RC4Crypt(bot[index]->wardenkey_out, (unsigned char *)buf, length);
	InsertVoid(buf, length);
	SendPacket(0x5E, index);
//freenret:
	while (nummods--)
		free(mods[nummods]);
}


void WardenParseCommand5(char *data, int len, int index) {
	__int32 buf32;
	unsigned char tmpkey[0x102];
	if (!lpwdnFnTable)
		return;
	if (!lpwdnFnTable->wdnGenKeys) {
		AddChat(vbRed, "Unable to generate new RC4 keys (wdnGenKeys == NULL)!", bot[index]->hWnd_rtfChat);
		return;
	}
	newrc4 = NULL;
	__asm {
		push 4
		push offset wdnkeyseed
		mov ecx, lpWdnModClass 
		mov eax, lpwdnFnTable
		//call dword ptr [eax]
		call [eax]WARDENFNTABLE.wdnGenKeys
	}
	if (!newrc4) {
		AddChat(vbRed, "wdnGenKeys() failed!", bot[index]->hWnd_rtfChat);
		return;
	}
	char *tmpdata = (char *)malloc(len);
	memcpy(tmpdata, data, len);
	memcpy(tmpkey, newrc4 + 0x102, sizeof(tmpkey));
	RC4Crypt(tmpkey, (unsigned char *)tmpdata, len);
	memcpy(tmpkey, newrc4, sizeof(tmpkey));
	if (pendingwpacket)	{
		free(pendingwpacket);
		pendingwpacket = NULL;
	}
	__asm {
		lea eax, [buf32]
		push eax
		push len
		push tmpdata
		mov ecx, lpWdnModClass
		mov eax, lpwdnFnTable
		//call dword ptr [eax + 8]
		call [eax]WARDENFNTABLE.wdnHandlePacket
	}
	if (!pendingwpacket) {
		AddChat(vbRed, "wdnSendPacket() failed!", bot[index]->hWnd_rtfChat);
		return;
	}
	RC4Crypt(tmpkey, (unsigned char *)pendingwpacket, pendingwpacketlen);
	RC4Crypt((unsigned char *)bot[index]->wardenkey_out, (unsigned char *)pendingwpacket, pendingwpacketlen);
	memcpy(bot[index]->wardenkey_out, newrc4, 0x102);
	memcpy(bot[index]->wardenkey_in, (char *)newrc4 + 0x102, 0x102);
	InsertVoid(pendingwpacket, pendingwpacketlen);
	SendPacket(0x5E, index);
}


/////////////////////////////////[Warden Module Preparation]///////////////////////////////////////////


int WardenDecryptInflateModule(char *modulename, char *module, int modulelen, char *keyseed, int index) {
	unsigned long byts;
	unsigned char cryptkey[0x102];

	MD5((char *)module, modulelen, (char *)cryptkey);
	if (memcmp(modhash, cryptkey, 16))
		return 1;

	WardenKeyGenerate(cryptkey, (unsigned char *)keyseed, 0x10);
	RC4Crypt(cryptkey, (unsigned char *)module, modulelen);
	if (*(int *)(module + modulelen - 0x104) != 'SIGN')
		return 2;

	unsigned long extlen = *(int *)module;
	char *extmodule = (char *)malloc(extlen);
	if (uncompress((unsigned char *)extmodule, &extlen, (unsigned char *)module + 4, modulelen - 4)) {
		free(extmodule);
		return 3;
	}

	HANDLE hFile = CreateFile(modulename, GENERIC_WRITE, 0, NULL, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		free(extmodule);
		return 4;
	}

	if (!WriteFile(hFile, extmodule, extlen, &byts, NULL)) {
		free(extmodule);
		CloseHandle(hFile);
		return 5;
	}

	free(extmodule);
	CloseHandle(hFile);
	return 0;
}


bool WardenPrepareModule(char *filename) {
	unsigned long read;
	int i;
	HANDLE hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
		return false;
	int filelen = GetFileSize(hFile, NULL);
	char *tmpmod = (char *)malloc(filelen);
	ReadFile(hFile, tmpmod, filelen, &read, NULL);
	CloseHandle(hFile);
	LPWMODHEADER lpwmodh = (LPWMODHEADER)tmpmod;
	char *lmodule = (char *)VirtualAlloc(0, lpwmodh->cbModSize, MEM_COMMIT, PAGE_READWRITE); 
	memcpy(lmodule, tmpmod, 40);

	unsigned long srcloc  = lpwmodh->sectioncount * 12 + 0x28;
	unsigned long destloc = lpwmodh->sectionlen;
	int sections = 0;
	while (destloc < lpwmodh->cbModSize) {
		int sectionsize = *(unsigned __int16 *)(tmpmod + srcloc);
		srcloc += 2;
		sections++;
		if (sections & 1) {											
			memcpy(lmodule + destloc, tmpmod + srcloc, sectionsize);
			srcloc += sectionsize;
		}
		
		destloc += sectionsize;
	}

	srcloc = lpwmodh->rvaFxnTableReloc;
	destloc = 0;
	for (i = 0; i != lpwmodh->reloccount; i++) {
		if (lmodule[srcloc] < 0) {
			destloc = ((lmodule[srcloc + 0] & 0x07F) << 24) |
					  ((lmodule[srcloc + 1] & 0x0FF) << 16) |
					  ((lmodule[srcloc + 2] & 0x0FF) << 8)  |
					  ((lmodule[srcloc + 3] & 0x0FF));
			srcloc += 4;
			AddChat(vbRed, "[WARDEN] module is using an absolute address!!!", hWnd_status_re);
		} else {				   
			destloc += (lmodule[srcloc + 1] & 0x0FF) + (lmodule[srcloc] << 8);
			srcloc += 2;
		}
		*(__int32 *)(lmodule + destloc) += (__int32)lmodule;
	}

	for (i = 0; i != lpwmodh->dllcount; i++) {
		__int32 tmp;
		unsigned long procstart  = lpwmodh->rvaImports + i * 8;
		unsigned long procoffset = *(__int32 *)(lmodule + procstart + 4);
		char *libtoload = lmodule + *(__int32 *)(lmodule + procstart);
		HMODULE hLib = LoadLibrary(libtoload);
		int fnnameoffset = *(__int32 *)(lmodule + procoffset);
		while (fnnameoffset) {
			fnnameoffset = *(__int32 *)(lmodule + procoffset); //add esi, dword ptr [ebx]
			if (fnnameoffset > 0) {
				char *fnstr = lmodule + fnnameoffset;
				if (!strcmp(fnstr, "GetProcAddress"))
					tmp = (__int32)MyGetProcAddress;
				else if (!strcmp(fnstr, "SetUnhandledExceptionFilter"))
					tmp = (__int32)MySetUnhandledExceptionFilter;
				else if (!strcmp(fnstr, "TerminateProcess"))
					tmp = (__int32)MyTerminateProcess;
				else 
					tmp = (__int32)GetProcAddress(hLib, fnstr);
			} else {
				tmp &= ~0x80000000;
			}			 
			*(__int32 *)(lmodule + procoffset) = tmp;
			procoffset += 4;
		}
	}  
																  
	for (i = 0; i != lpwmodh->sectioncount; i++) {
		unsigned long oldprotect;
		LPWMODPROTECT lpwmodp = (LPWMODPROTECT)(lmodule + lpwmodh->sectioncount * 12 + 0x28);
		lpwmodp->base = lmodule + (int)lpwmodp->base;
		VirtualProtect(lpwmodp->base, lpwmodp->len, lpwmodp->protectdword, &oldprotect);
		if (oldprotect & 0xF0)
			FlushInstructionCache(GetCurrentProcess(), lpwmodp->base, lpwmodp->len);
	}
	if (lpwmodh->cbModSize < lpwmodh->rvaFxnTableReloc)
		return false;
	VirtualFree(lmodule + lpwmodh->rvaFxnTableReloc,
		lpwmodh->cbModSize - lpwmodh->rvaFxnTableReloc, MEM_RELEASE);
	currentmodule = (void *)lmodule;
	return true;
}


void WardenModuleInitalize(LPWMODHEADER lpwmodh) {
	int tmp = 1 - lpwmodh->unk2;
	if (tmp > (int)lpwmodh->unk1)
		return;
	__int32 lpfnInit = (int)lpwmodh + *(__int32 *)((int)lpwmodh + (int)lpwmodh->lpfnInit + (tmp << 2));
	__asm {
		mov ecx, offset snpTable
		call lpfnInit
		mov lpWdnModClass, eax
		mov eax, dword ptr [eax]
		mov lpwdnFnTable, eax
	}
}	


void WardenUnloadModule() {
	__asm {
		mov ecx, lpWdnModClass
		mov eax, lpwdnFnTable
		//call dword ptr [eax + 4]
		call [eax]WARDENFNTABLE.wdnUnloadModule
	}
	VirtualFree(currentmodule, 0, MEM_RELEASE);
	currentmodule = NULL;
}


void __stdcall nullsub(int p1, int p2) {

}


void __stdcall nullsub2(int p1) {

}


FARPROC WINAPI MyGetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
	char asdf[256], fname[MAX_PATH];
	char *sdfg = fname;
	*fname = 0;
	if (GetModuleFileName(hModule, fname, sizeof(fname))) {
		sdfg = strrchr(fname, '\\');
		if (sdfg) {
			*sdfg = 0;
			sdfg++;
		}
	}
	sprintf(asdf, "[WARDEN] module getting address to (0x%08x) %s!%s() ...", hModule, sdfg, lpProcName);
	AddChat(vbYellow, asdf, hWnd_status_re);
	if (!strcmp(lpProcName, "AddVectoredExceptionHandler")) {
		return (FARPROC)nullsub;
	} else if (!strcmp(lpProcName, "RemoveVectoredExceptionHandler")) {
		return (FARPROC)nullsub2;
	}
	
	return GetProcAddress(hModule, lpProcName);
}


LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MySetUnhandledExceptionFilter(
			LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
	char buf[128];
	sprintf(buf, "Module attempted to call SetUnhandledExceptionFilter(0x%08x)!", lpTopLevelExceptionFilter);
	AddChat(vbYellow, buf, hWnd_status_re);
	return NULL;
}


BOOL WINAPI MyTerminateProcess(HANDLE hProcess, UINT uExitCode) {
	char asdf[128];
	sprintf(asdf, "Module attempted to call TerminateProcess(0x%08x, %d)!", hProcess, uExitCode); 
	AddChat(vbRed, asdf, hWnd_status_re);
	return true;
}


/////////////////////////////////////[Warden Callbacks]////////////////////////////////////////////////


void __stdcall WdnCbkSendPacket(char *data, int len) {
	char asdf[128];
	pendingwpacket = (char *)malloc(len);
	memcpy(pendingwpacket, data, len);
	pendingwpacketlen = len;
	sprintf(asdf, "[WARDEN] module wants to send packet, %d bytes", len);
	AddChat(vbGreen, asdf, hWnd_status_re);
	//StrToHexOut(data, len, bot[actprof]->hWnd_rtfChat);
}


bool __stdcall WdnCbkCheckModule(char *modname, unsigned long _2) {
	char asdf[128];
	sprintf(asdf, "[WARDEN] module wants to check %s. _2: 0x%08x!", modname, _2);
	AddChat(vbGreen, asdf, hWnd_status_re);
	return true;
}


unsigned long __stdcall WdnCbkLoadModule(char *decryptkey, char *module, int modlen) {
	char asdf[128];
	sprintf(asdf, "[WARDEN] module wants to load a new module, %d bytes.", modlen);
	AddChat(vbYellow, asdf, hWnd_status_re);
	return NULL;
}


void *__stdcall WdnCbkMemAlloc(unsigned long len) {
	char asdf[128];
	void *blah = malloc(len);
	sprintf(asdf, "[WARDEN] module allocated %d bytes at 0x%08x.", len, blah);
	AddChat(vbYellow, asdf, hWnd_status_re);
	return blah;
}


void __stdcall WdnCbkMemFree(void *mem) {
	char asdf[64];
	free(mem);
	sprintf(asdf, "[WARDEN] module freed 0x%08x.", mem);
	AddChat(vbYellow, asdf, hWnd_status_re);
}


void __stdcall WdnCbkSetRC4(void *keys, unsigned long len) {
	char asdf[128];
	sprintf(asdf, "[WARDEN] module set RC4 keys at 0x%08x, %d bytes.", keys, len);
	AddChat(vbYellow, asdf, hWnd_status_re);
}


char *__stdcall WdnCbkGetRC4(char *buffer, unsigned long *len) {
	char asdf[128];
	sprintf(asdf, "[WARDEN] module got RC4 keys at 0x%08x, %d bytes.", buffer, *len);
	AddChat(vbYellow, asdf, hWnd_status_re);

	char *oldrc4 = newrc4;
	newrc4 = buffer;
	return oldrc4;
}


///////////////////////////////////////[Warden Crypto]/////////////////////////////////////////////////


void RC4Crypt(unsigned char *key, unsigned char *data, int length) {
	for (int i = 0; i < length; i++) {
		key[0x100]++;
		key[0x101] += key[key[0x100]];
		SWAP(key[key[0x101]], key[key[0x100]]);
		data[i] = data[i] ^ key[(key[key[0x101]] + key[key[0x100]]) & 0xFF];
	}
}


void WardenKeyInit(char *KeyHash, int index) {
	char temp[0x10];
	wdnkeyseed = (unsigned long)KeyHash;
	WardenKeyDataInit(&bot[index]->warden_data, KeyHash, 4);
	WardenKeyDataGetBytes(&bot[index]->warden_data, temp, 16);
	WardenKeyGenerate((unsigned char *)bot[index]->wardenkey_out, (unsigned char *)temp, 16);
	WardenKeyDataGetBytes(&bot[index]->warden_data, temp, 16);	
	WardenKeyGenerate((unsigned char *)bot[index]->wardenkey_in, (unsigned char *)temp, 16);
}


void WardenKeyDataInit(t_random_data *source, char *seed, int length) {
	int length1 = length >> 1;
	int length2 = length - length1;
	char *seed1 = seed;
	char *seed2 = seed + length1;
	memset(source, 0, sizeof(t_random_data));
	SHA1(seed1, length1, source->random_source_1);
	SHA1(seed2, length2, source->random_source_2);
	WardenKeyDataUpdate(source);
	source->current_position = 0;
}


void WardenKeyGenerate(unsigned char *key_buffer, unsigned char *base, unsigned int baselen) {
	unsigned char val = 0;
	unsigned int pos = 0;
	unsigned int i;
	for (i = 0; i < 0x100; i++)
		key_buffer[i] = i;
	key_buffer[0x100] = 0;
	key_buffer[0x101] = 0;
	for (i = 1; i <= 0x40; i++) {
		int tmp = i << 2;
		val += key_buffer[tmp - 4] + base[pos++ % baselen];
		SWAP(key_buffer[tmp - 4], key_buffer[val & 0xFF]);
		val += key_buffer[tmp - 3] + base[pos++ % baselen];
		SWAP(key_buffer[tmp - 3], key_buffer[val & 0xFF]);
		val += key_buffer[tmp - 2] + base[pos++ % baselen];
		SWAP(key_buffer[tmp - 2], key_buffer[val & 0xFF]);
		val += key_buffer[tmp - 1] + base[pos++ % baselen];
		SWAP(key_buffer[tmp - 1], key_buffer[val & 0xFF]);
	}
}


void WardenKeyDataUpdate(t_random_data *source) {
	char data[60];
	memcpy(data, source->random_source_1, 20);
	memcpy(data + 20, source->random_data, 20);
	memcpy(data + 40, source->random_source_2, 20);
	SHA1(data, 60, source->random_data);
}


void WardenKeyDataGetBytes(t_random_data *source, char *buffer, int bytes) {
	for (int i = 0; i < bytes; i++)
		buffer[i] = WardenKeyDataGetByte(source);
}


char WardenKeyDataGetByte(t_random_data *source) {
	int i = source->current_position;
	char value = source->random_data[i];
	i++;
	if (i >= 20) {
		i = 0;
		WardenKeyDataUpdate(source);
	}
	source->current_position = i;
	return value;
}


unsigned long WardenGenerateChecksum(char *data, int len) {
	unsigned long result[5];
	SHA1(data, len, (char *)result);
	return result[0] ^ result[1] ^ result[2] ^ result[3] ^ result[4];
}

