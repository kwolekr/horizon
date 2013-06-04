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
#include "checkrevision.h"
#include "connection.h"
#include "packetbuffer.h"
#include "loaddll.h"
#include "bnftp.h"


///////////////////////////////////////////////////////////////////////////////


bool DownloadFileFromBNFTP(char *filename, int index) {
	char buf[512], sdfg[128];
	unsigned int totallen = 0;
	SOCKET sckFTP;
	SYSTEMTIME systime;

	if (!filename || !*filename)
		return false;

	if (!ConnectSocket(&sckFTP, bot[index]->server, 6112))
		return false;

	send(sckFTP, "\x02", 1, 0);
	
	InsertWORD(0x0100);	      //version
	InsertDWORD(bot[index]->platform);      //platform id
	InsertDWORD(bot[index]->client);      //client id
	InsertDWORD(0);		      //banner id
	InsertDWORD(0);		      //banner file extention
	InsertDWORD(0);		      //start position
	InsertDWORD(0);	    	  //1  filetime of local file
	InsertDWORD(0);			  //2
	InsertNTString(filename); //filename
	SendPacketBNFTP(sckFTP);
	AddChat(vbYellow, "[BNFTP] Sending request...", bot[index]->hWnd_rtfChat);

	int len = recv(sckFTP, buf, sizeof(buf), 0);
	if (len == 0 || len == -1) {
		AddChat(vbRed, !len ? "[BNFTP] Server disconnected gracefully, file not found." :
			"[BNFTP] Server disconnected forcefully.", bot[index]->hWnd_rtfChat);
		shutdown(sckFTP, 2);
		closesocket(sckFTP);
		return false;
	}
	char *dlfilename = buf + 24;
	unsigned int filesize = *(unsigned long *)(buf + 4);
	//FILETIME filetime = *(FILETIME *)(buf + 16);			  
	FILETIME filetime;
	filetime.dwLowDateTime  = *(int *)(buf + 16);
	filetime.dwHighDateTime = *(int *)(buf + 20);

	//GetStuff("Main", "BnftpDLPath", sdfg);
	//strcpy(sdfg, dlfilename);
	//FILE *file = fopen(sdfg, "wb");
	//if (!file)
	//	return false;
	char *asdf = (char *)malloc(filesize);
	HANDLE hFile = CreateFile(dlfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	do {
		DWORD byteswritten;
		len = recv(sckFTP, asdf, filesize, 0);
		if (!len || len == SOCKET_ERROR)
			return false;
		totallen += len;
		WriteFile(hFile, asdf, len, &byteswritten, NULL);
		//fwrite(asdf, 1, len, file);
		sprintf(sdfg, "[BNFTP] %d/%d bytes (%%%d)", totallen, filesize, (int)(((double)totallen / (double)filesize) * 100));
		AddChat(vbYellow, sdfg, bot[index]->hWnd_rtfChat);
	} while (totallen < filesize);
	//fclose(file);
	free(asdf);
	shutdown(sckFTP, 2);
	closesocket(sckFTP);
	if (!SetFileTime(hFile, &filetime, NULL, &filetime))
		return false;
	CloseHandle(hFile);
	FileTimeToSystemTime(&filetime, &systime);
	sprintf(sdfg, "[BNFTP] Saved %s (%d bytes, last modified %d/%d/%d %d:%d:%d.%d)",
		dlfilename, filesize, systime.wMonth, systime.wDay, systime.wYear,
		systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	AddChat(vbGreen, sdfg, bot[index]->hWnd_rtfChat);
	return true;
}


int InitiateDLAndWait(char *filename, char *savefilename, bool doextract, int index) {
	MSG msg;
	LPBNFTPDL lpbnftpdl = (LPBNFTPDL)malloc(sizeof(BNFTPDL));
	lpbnftpdl->index = index;
	lpbnftpdl->performextract = doextract;
	strncpy(lpbnftpdl->filename, filename, sizeof(lpbnftpdl->filename));
	strcpy(lpbnftpdl->savefilename, savefilename);
	HANDLE hThread = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)BNFTPDownloadWrapper,lpbnftpdl, 0, NULL);
	while (GetMessage(&msg, (HWND)NULL, 0, 0)) {
		if (msg.message == WM_DLCOMPLETE && msg.lParam == index && msg.hwnd == hWnd_main)
			break;
		if (!TranslateMDISysAccel(hWnd_Client, &msg)) {
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}
	CloseHandle(hThread);
	free(lpbnftpdl);
	return msg.wParam;
}


void __stdcall BNFTPDownloadWrapper(LPBNFTPDL pbnftpdl) {
	bool success;
	if (DownloadFileFromBNFTP(pbnftpdl->filename, pbnftpdl->index)) {
		if (pbnftpdl->performextract) {
			char filename[64];
			strcpy(filename, pbnftpdl->filename);
			*(int *)(filename + strlen(filename) - 4) = 'lld.';
			success = ExtractMPQ(pbnftpdl->filename, filename, pbnftpdl->savefilename);
			if (success) {
				DeleteFile(pbnftpdl->filename);
			} else {
				AddChat(vbRed, "Failed to extract mpq!", bot[pbnftpdl->index]->hWnd_rtfChat);		
			}
		}
		success = true;
	} else { ///crash here, pbnftpdl == 0xfeeefeee
		AddChat(vbRed, "[BNFTP] Failed to download!", bot[pbnftpdl->index]->hWnd_rtfChat);
		success = false;
	}
	PostMessage(hWnd_main, WM_DLCOMPLETE, success, pbnftpdl->index);
}


bool ExtractMPQ(char *archive, char *filetoget, char *extractto) {
	int highdword;
	HANDLE hMPQ, hMPQFile;	
	if (!storm.SFileSetLocale('enUS'))
		return false;
	if (!storm.SFileDestroy())
		return false;
	if (!storm.SFileOpenArchive(archive, 0, 0, &hMPQ))
		return false;
	if (!storm.SFileOpenFileEx(hMPQ, filetoget, 0, &hMPQFile))
		return false;
	int filesize = storm.SFileGetFileSize(hMPQFile, &highdword);
	if (filesize == -1)
		return false;
	char *asdf = (char *)malloc(filesize);
	if (!asdf)
		return false;
	if (!storm.SFileReadFile(hMPQFile, asdf, filesize, NULL, 0))
		return false;
	if (!storm.SFileCloseFile(hMPQFile))
		return false;
	if (!storm.SFileCloseArchive(hMPQ))
		return false;
	highdword = 'bw';
	FILE *file = fopen(extractto, (const char *)&highdword);
	if (!file)
		return false;
	fwrite(asdf, filesize, 1, file);
	fclose(file); 
	free(asdf);
	return true;
}


#if 0
	__asm {
		push 'enUS'
		call storm.SFileSetLocale
		test eax, eax
		jz errored

		call storm.SFileDestroy
		test eax, eax
		jz errored

		lea eax, [hMPQ]
		push eax
		push 0
		push 0
		mov eax, filename
		push eax
		call storm.SFileOpenArchive
		test eax, eax
		jz errored

		lea eax, [hMPQFile]
		push eax		 
		push 0
		push toget
		push hMPQ
		call storm.SFileOpenFileEx
		test eax, eax
		jz errored

		lea eax, [highdword]
		push eax
		push hMPQFile
		call storm.SFileGetFileSize
		cmp eax, -1
		je errored
		mov filesize, eax

		push eax
		call malloc
		pop ecx
		mov asdf, eax

		push 0
		push 0
		push filesize
		push eax
		push hMPQFile
		call storm.SFileReadFile
		test eax, eax
		jz errored

		push '\\'
		push toget
		call strchr
		add esp, 8
		mov ecx, toget
		test eax, eax
		jz over
		mov ecx, eax
		inc ecx

over:

		mov highdword, 'bw'
		lea eax, [highdword]
		push eax
		push ecx
		call fopen
		mov file, eax
		add esp, 8

		push eax
		push filesize
		push 1
		push asdf
		call fwrite
		add esp, 10h

		push file
		call fclose
		pop ecx

		push asdf
		call free
		pop ecx

		push hMPQFile
		call storm.SFileCloseFile
		test eax, eax
		jz errored

		push hMPQ
		call storm.SFileCloseArchive
		test eax, eax
		jz errored
	}
	return;
errored:
	sprintf(buf, "ExtractMPQ() failed! GetLastError: %d.", GetLastError());
	//AddChat(vbRed, buf);
}
#endif

