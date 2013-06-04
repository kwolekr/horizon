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
#include "bnftp.h"
#include "loaddll.h"
#include "packets.h"
#include "hashing.h"
#include "connection.h"
#include "checkrevision.h"

DWORD dwMpqChecksumKeys[] = {
	0xE7F4CB62lu,
	0xF6A14FFClu,
	0xAA5504AFlu,
	0x871FCDC2lu,
	0x11BF6A18lu,
	0xC57292E6lu,						  
	0x7927D27Elu,
	0x2FEC8733lu
};

struct seed_table seeds[] = {
	{0xA1F3055A, 0x4551FB8F},	//00
	{0x5657124C, 0x81776C47},	//01
	{0x1780AB47, 0x0511663A},	//02
	{0x80B3A410, 0x8839FDF0},	//03
	{0xAF2179EA, 0xEE60E7D6},	//04
	{0x0837B808, 0xB43A6490},	//05
	{0x6F2516C6, 0x246A64BA},	//06
	{0xE3178148, 0x6F6536F1},	//07
	{0x0FCF90B6, 0x3D2C22F0},	//08
	{0xF2F09516, 0x8624FC60},	//09
	{0x378D8D8C, 0x9F30D4E7},	//10
	{0x07F8E083, 0x24A7F246},	//11
	{0xB0EE9741, 0x5AE1F560},	//12
	{0x7923C9AF, 0x3026FF25},	//13
	{0xCA11A05E, 0x0ED32EBF},	//14
	{0xD723C016, 0xFB88CB39},	//15
	{0xFD545590, 0x12BF7406},	//16
	{0xFB600C2E, 0x8B38612E},	//17
	{0x684C8785, 0x95F19E77},	//18
	{0x58BEDE0B, 0x2C0F3DCF},	//19
	{NULL, NULL}
};


///////////////////////////////////////////////////////////////////////////////


/*****************
 * files:        *
 * 0 - game exe  *
 * 1 - storm dll *
 * 2 - bnet dll  *
 * 3 - cr dll    *
 * 4 - video buf *
 *****************/
bool DoCheckRevision(char *ChecksumFormula, LPFILETIME lpMpqFt, char *mpqName,
					unsigned long *Checksum, unsigned long *exeVersion,
					char *exeInfo, int index) {
	char tmpIniKey[16], files[5][MAX_PATH], buf[64];
	int i;
	SOCKET sbnls;

	if (bot[index]->crtype != CRTYPE_NONE)
		GetCheckRevisionLib(mpqName, lpMpqFt, index);

	if (*mpqName)
		*(unsigned long *)(mpqName + strlen(mpqName) - 4) = 'lld.';

	switch (bot[index]->crtype) {
		case CRTYPE_LOCAL:
			for (i = 0; i != 3; i++) {
				sprintf(tmpIniKey, "%sHash%u", bot[index]->clientstr, i + 1); 
				GetStuffConfig("Hashes", tmpIniKey, files[i]);
			}
			if (*(unsigned long *)mpqName == '-rev') {
				*Checksum = CheckRevision(files[0], files[1], files[2], mpqName, ChecksumFormula);
				*exeVersion = GetExeInfo(files[0], exeInfo);
				//if (!CheckRevision(files[0], files[1], files[2], ChecksumFormula, exeVersion, Checksum, exeInfo, mpqName)) {
				//	AddChat(vbRed, "CheckRevision error!", bot[index]->hWnd_rtfChat);
				//	return;
				//}
			} else {
				if (*mpqName)
					*(unsigned long *)(mpqName + strlen(mpqName) - 4) = 'lld.';
				sprintf(tmpIniKey, "%sVidBuf", bot[index]->client == 'SEXP' ? "STAR" : bot[index]->clientstr);
				GetStuffConfig("Hashes", tmpIniKey, files[4]);						
				GetStuffConfig("Hashes", "CRDLLs", files[3]);
				strcat(files[3], mpqName);
				i = CheckRevisionLD(files[0], files[1], files[2], ChecksumFormula, (unsigned long &)exeVersion, 
						(unsigned long &)Checksum, exeInfo, files[3], files[4]);
				if (i) {
					sprintf(buf, "Failed Lockdown CheckRevision! [error %d]", i);
					AddChat(vbRed, buf, bot[index]->hWnd_rtfChat);
					return false;
				}
			}
			break;
		case CRTYPE_EXTERNAL:
			for (i = 0; i != 3; i++) {
				sprintf(tmpIniKey, "%sHash%u", bot[index]->clientstr, i + 1); 
				GetStuffConfig("Hashes", tmpIniKey, files[i]);
			}
			if (*(int *)mpqName == '-rev') {
				//paramstring "fesbnc"
				if (!bncsutil.checkRevisionFlat(ChecksumFormula, files[0], files[1], files[2],
					ExtractCRMPQNumber(mpqName), (int *)Checksum)) {
					AddChat(vbRed, "BNCSUtil!checkRevisionFlat() failed!", bot[index]->hWnd_rtfChat);
					return false;
				}
				//char asdfg[32];
				//sprintf(asdfg, "checksum: 0x%08x", *Checksum);
				//AddChat(vbBlue, asdfg, bot[index]->hWnd_rtfChat);
				bncsutil.getExeInfo(files[0], exeInfo, 128, (int *)exeVersion, 1);
			} else {
				switch (bot[index]->client) {
					case 'STAR':
					case 'SEXP':
					case 'SSHR':
					case 'JSTR':
						*(int *)tmpIniKey = 'RATS';
						break;
					case 'DRTL':
					case 'DSHR':
						*(int *)tmpIniKey = 'LTRD';
						break;
					case 'W2BN':
						*(int *)tmpIniKey = 'NB2W';
				}
				strcpy(tmpIniKey + 4, "VidBuf");
				GetStuffConfig("Hashes", tmpIniKey, files[4]);						
				GetStuffConfig("Hashes", "CRDLLs", files[3]);
				strcat(files[3], mpqName);
				//paramstring "esbfvcimud"
				if (!CheckRevisionEx(files[0], files[1], files[2], ChecksumFormula, (int *)exeVersion,
					(int *)Checksum, exeInfo, files[3], NULL, files[4])) {
					AddChat(vbRed, "CheckRevision!CheckRevisionEx() failed!", bot[index]->hWnd_rtfChat);
					return false;
				}
				
			}
			break;
		case CRTYPE_BNLS:
			//////note: can now support 0x09, 0x18, 0x1A

			AddChat(vbYellow, "[BNLS] Connecting...", bot[index]->hWnd_rtfChat);
			if (!ConnectSocket(&sbnls, bot[index]->bnlsserver, 9367)) {
				sprintf(buf, "[BNLS] Connect failure %d!", WSAGetLastError());
				AddChat(vbRed, buf, bot[index]->hWnd_rtfChat);
				return false;
			}
			AddChat(vbGreen, "[BNLS] Connected!", bot[index]->hWnd_rtfChat);
			SendBNLS0x09(mpqName, ChecksumFormula, sbnls, index);
			i = recv(sbnls, buf, sizeof(buf), 0);
			if (!i || i == -1) {
				sprintf(buf, "[BNLS] recv() error %d!", WSAGetLastError());
				AddChat(vbRed, buf, bot[index]->hWnd_rtfChat);
				return false;
			}
			shutdown(sbnls, 2);
			closesocket(sbnls);
			AddChat(vbGreen, "[BNLS] Received 0x09!", bot[index]->hWnd_rtfChat);
			if (*(int *)(buf + 3)) {
				*exeVersion = *(unsigned long *)(buf + 7);
				*Checksum   = *(unsigned long *)(buf + 11);
				strncpy(exeInfo, buf + 15, 64);
			} else {
				AddChat(vbRed, "[BNLS] BNLS CheckRevision failed!", bot[index]->hWnd_rtfChat);
				DisconnectProfile(index, false);
				return false;
			}
			break;
		case CRTYPE_NONE:
			;
	}
	return true;
}


int ExtractCRMPQNumber(char *mpqName) {
	char *asdf = strchr(mpqName, '.');
	if (asdf) {
		int tmp = *(asdf - 1) - '0';
		return (*(int *)mpqName == '-rev') ? tmp : (tmp + ((*(asdf - 2) - '0') * 10));
	}
	return -1;
}


void GetCheckRevisionLib(char *mpqName, LPFILETIME lpMpqFt, int index) {
	char asdf[MAX_PATH], sdfg[128];
	GetStuffConfig("Hashes", "CRDLLs", asdf);
	strcat(asdf, mpqName);
	*(int *)(asdf + strlen(asdf) - 4) = 'lld.';
	if (!IsFileValid(asdf, lpMpqFt)) {
		sprintf(sdfg, "Missing %s or out of date, downloading from BNFTP.", mpqName);
		AddChat(vbYellow, sdfg, bot[index]->hWnd_rtfChat);
		if (!InitiateDLAndWait(mpqName, asdf, true, index)) {
			AddChat(vbRed, "[BNFTP] Download failed!", bot[index]->hWnd_rtfChat);
			return;
		}
		AddChat(vbGreen, "CheckRevision library successfully prepared.", bot[index]->hWnd_rtfChat);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
BOOL CheckRevision(LPCTSTR lpszFileName1, LPCTSTR lpszFileName2, LPCTSTR lpszFileName3, LPCTSTR lpszValueString, DWORD * lpdwVersion, DWORD * lpdwChecksum, LPSTR lpExeInfoString, LPCTSTR lpszMpqFileName) {
   HANDLE hFile, hFileMapping;
   char * s, lpszFileName[256], cOperations[16];
   int nHashFile, nVariable1[16], nVariable2[16], nVariable3[16], nVariable, i, k, nHashOperations;
   DWORD dwTotalSize, dwSize, j, dwBytesRead, dwVariables[4], dwMpqKey, * lpdwBuffer;
   LPSTR lpszFileNames[3];
   FILETIME ft;
   SYSTEMTIME st;
   LPBYTE lpbBuffer;
   VS_FIXEDFILEINFO *ffi;
   s = strchr((char *)lpszMpqFileName, '.');
   if (s == NULL)
      return FALSE;
   nHashFile = (int) (*(s - 1) - '0');
   if (nHashFile > 7 || nHashFile < 0)
      return FALSE;
   dwMpqKey = dwMpqChecksumKeys[nHashFile];
   lpszFileNames[0] = (LPSTR) lpszFileName1;
   lpszFileNames[1] = (LPSTR) lpszFileName2;
   lpszFileNames[2] = (LPSTR) lpszFileName3;
   s = (char *) lpszValueString;
   while (*s != '\0') {
      if (isalpha(*s))
         nVariable = (int) (toupper(*s) - 'A');
      else {
         nHashOperations = (int) (*s - '0');
         s = strchr(s, ' ');
         if (s == NULL)
            return FALSE;
         s++;
         break;
      }
      if (*(++s) == '=')
         s++;
      dwVariables[nVariable] = atol(s);
      s = strchr(s, ' ');
      if (s == NULL)
         return FALSE;
      s++;
   }
   for (i = 0; i < nHashOperations; i++) {
      if (!isalpha(*s))
         return FALSE;
      nVariable1[i] = (int) (toupper(*s) - 'A');
      if (*(++s) == '=')
         s++;
      if (toupper(*s) == 'S')
         nVariable2[i] = 3;
      else
         nVariable2[i] = (int) (toupper(*s) - 'A');
      cOperations[i] = *(++s);
      s++;
      if (toupper(*s) == 'S')
         nVariable3[i] = 3;
      else
         nVariable3[i] = (int) (toupper(*s) - 'A');
      s = strchr(s, ' ');
      if (s == NULL)
         break;
      s++;
   }
   dwVariables[0] ^= dwMpqKey;
   for (i = 0; i < 3; i++) {
      if (lpszFileNames[i][0] == '\0')
         continue;
      hFile = CreateFile(lpszFileNames[i], GENERIC_READ, FILE_SHARE_READ,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile == (HANDLE) INVALID_HANDLE_VALUE)
         return FALSE;
      hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
      if (hFileMapping == NULL) {
         CloseHandle(hFile);
         return FALSE;
      }
      lpdwBuffer = (LPDWORD) MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
      if (lpdwBuffer == NULL) {
         CloseHandle(hFileMapping);
         CloseHandle(hFile);
         return FALSE;
      }
      if (i == 0) {
         GetFileTime(hFile, &ft, NULL, NULL);
         FileTimeToSystemTime(&ft, &st);
         dwTotalSize = GetFileSize(hFile, NULL);
      }
      dwSize = (GetFileSize(hFile, NULL) / 1024lu) * 1024lu;
      for (j = 0; j < (dwSize / 4lu); j++) {
         dwVariables[3] = lpdwBuffer[j];
         for (k = 0; k < nHashOperations; k++) {
            switch (cOperations[k]) {
               case '+':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] + dwVariables[nVariable3[k]];
                  break;
               case '-':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] - dwVariables[nVariable3[k]];
                  break;
               case '^':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] ^ dwVariables[nVariable3[k]];
                  break;
               default:
                  return FALSE;
            }
         }
      }
      UnmapViewOfFile(lpdwBuffer);
      CloseHandle(hFileMapping);
      CloseHandle(hFile);
   }
   strcpy(lpszFileName, lpszFileName1);
   dwSize = GetFileVersionInfoSize(lpszFileName, &dwBytesRead);
   lpbBuffer = (LPBYTE) VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
   if (lpbBuffer == NULL)
      return FALSE;
   if (GetFileVersionInfo(lpszFileName, NULL, dwSize, lpbBuffer) == FALSE)
      return FALSE;
   if (VerQueryValue(lpbBuffer, "\\", (LPVOID *) &ffi, (PUINT) &dwSize) == FALSE)
      return FALSE;
   *lpdwVersion = ((HIWORD(ffi->dwProductVersionMS) & 0xFF) << 24)
		| ((LOWORD(ffi->dwProductVersionMS) & 0xFF) << 16)
		| ((HIWORD(ffi->dwProductVersionLS) & 0xFF) << 8)
		| (LOWORD(ffi->dwProductVersionLS) & 0xFF);
   VirtualFree(lpbBuffer, 0lu, MEM_RELEASE);
   s = (char *) &lpszFileName[strlen(lpszFileName)-1];
   while (*s != '\\' && s > (char *) lpszFileName)
      s--;
   s++;
   sprintf(lpExeInfoString, "%s %02u/%02u/%02u %02u:%02u:%02u %lu",
		s, st.wMonth, st.wDay, st.wYear % 100, st.wHour, st.wMinute, st.wSecond, dwTotalSize);
   *lpdwChecksum = dwVariables[2];
   return TRUE;
}
#endif


unsigned long CheckRevision(LPCTSTR lpszFileName0, LPCTSTR lpszFileName1, LPCTSTR lpszFileName2,
							LPCTSTR lpszMpqFileName, LPCTSTR lpszValueString) {
   HANDLE hFile, hFileMapping;
   char *s, cOperations[16];
   int nHashFile, nVariable1[16], nVariable2[16], nVariable3[16], nVariable, i, k, nHashOperations;
   DWORD dwSize, j, dwVariables[4], dwMpqKey, *lpdwBuffer;
   LPSTR lpszFileNames[3];
   s = strchr((char *)lpszMpqFileName, '.');							 
   if (s == NULL)
      return 0;									
   nHashFile = (int) (*(s - 1) - '0');
   if (nHashFile > 7 || nHashFile < 0)
      return 0;
   dwMpqKey = dwMpqChecksumKeys[nHashFile];
   lpszFileNames[0] = (LPSTR) lpszFileName0;
   lpszFileNames[1] = (LPSTR) lpszFileName1;
   lpszFileNames[2] = (LPSTR) lpszFileName2;
   s = (char *)lpszValueString;
   while (*s != '\0') {
      if (isalpha(*s))												   
         nVariable = (int)(toupper(*s) - 'A');
      else {
         nHashOperations = (int)(*s - '0');
         s = strchr(s, ' ');
         if (s == NULL)
            return 0;
         s++;
         break;
      }
      if (*(++s) == '=')
         s++;
      dwVariables[nVariable] = atol(s);
      s = strchr(s, ' ');
      if (s == NULL)
         return 0;
      s++;
   }
   for (i = 0; i < nHashOperations; i++) {
      if (!isalpha(*s))
         return 0;
      nVariable1[i] = (int) (toupper(*s) - 'A');
      if (*(++s) == '=')
         s++;
      if (toupper(*s) == 'S')
         nVariable2[i] = 3;
      else
         nVariable2[i] = (int) (toupper(*s) - 'A');
      cOperations[i] = *(++s);
      s++;
      if (toupper(*s) == 'S')
         nVariable3[i] = 3;
      else
         nVariable3[i] = (int) (toupper(*s) - 'A');
      s = strchr(s, ' ');
      if (s == NULL)
         break;
      s++;
   }
   dwVariables[0] ^= dwMpqKey;
   for (i = 0; i < 3; i++) {
      if (lpszFileNames[i][0] == '\0')
         continue;
      hFile = CreateFile(lpszFileNames[i], GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile == (HANDLE) INVALID_HANDLE_VALUE)
         return 0;
      hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
      if (hFileMapping == NULL) {
         CloseHandle(hFile);
         return 0;
      }
      lpdwBuffer = (LPDWORD) MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
      if (lpdwBuffer == NULL) {
         CloseHandle(hFileMapping);
         CloseHandle(hFile);
         return 0;
      }
      dwSize = (GetFileSize(hFile, NULL) / 1024lu) * 1024lu;
      for (j = 0; j < (dwSize / 4lu); j++) {
         dwVariables[3] = lpdwBuffer[j];
         for (k = 0; k < nHashOperations; k++) {
            switch (cOperations[k]) {
               case '+':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] + dwVariables[nVariable3[k]];
                  break;
               case '-':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] - dwVariables[nVariable3[k]];
                  break;
               case '^':
                  dwVariables[nVariable1[k]] = dwVariables[nVariable2[k]] ^ dwVariables[nVariable3[k]];
                  break;
               default:
                  return 0;
            }
         }
      }
      UnmapViewOfFile(lpdwBuffer);
      CloseHandle(hFileMapping);
      CloseHandle(hFile);
   }
   return dwVariables[2];
}

unsigned long GetExeInfo(char *lpszFileName, char *lpExeInfoString) {
	FILETIME ft;
	SYSTEMTIME st;
	VS_FIXEDFILEINFO *ffi;
	DWORD dwBytesRead;
	DWORD dwSize = GetFileVersionInfoSize(lpszFileName, &dwBytesRead);
	char *lpbBuffer = (char *)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (!lpbBuffer)
		return 0;
	if (!GetFileVersionInfo(lpszFileName, NULL, dwSize, lpbBuffer))	{
		VirtualFree(lpbBuffer, 0, MEM_RELEASE);
		return 0;
	}
	if (!VerQueryValue(lpbBuffer, "\\", (LPVOID *) &ffi, (PUINT)&dwSize)) {
		VirtualFree(lpbBuffer, 0, MEM_RELEASE);
		return 0;														
	}
	unsigned long ret = ((HIWORD(ffi->dwProductVersionMS) & 0xFF) << 24) |
		   ((LOWORD(ffi->dwProductVersionMS) & 0xFF) << 16) |
	  	   ((HIWORD(ffi->dwProductVersionLS) & 0xFF) << 8) |
		   (LOWORD(ffi->dwProductVersionLS) & 0xFF);

	VirtualFree(lpbBuffer, 0, MEM_RELEASE);
	char *s = (char *)&lpszFileName[strlen(lpszFileName) - 1];
	while (*s != '\\' && s > (char *)lpszFileName)
		s--;
	s++;
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;
	GetFileTime(hFile, &ft, NULL, NULL);
	FileTimeToSystemTime(&ft, &st);
	DWORD dwTotalSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	sprintf(lpExeInfoString, "%s %02u/%02u/%02u %02u:%02u:%02u %lu",
		s, st.wMonth, st.wDay, st.wYear % 100, st.wHour, st.wMinute, st.wSecond, dwTotalSize);
	return ret;
}									   


int patch_dword(unsigned long AddressToPatch, unsigned long Value) {
    unsigned long OldProtect = 0;
    if(!VirtualProtect((LPVOID)AddressToPatch, 4, PAGE_EXECUTE_READWRITE, &OldProtect))
		return 1;
    *(unsigned long*)AddressToPatch = Value;
    if(!VirtualProtect((LPVOID)AddressToPatch, 4, OldProtect, &OldProtect))
		return 1;
    return 0;
}


int patch_word(unsigned long AddressToPatch, WORD Value) {
    unsigned long OldProtect = 0;
    if(!VirtualProtect((LPVOID)AddressToPatch, 4, PAGE_EXECUTE_READWRITE, &OldProtect))
		return 1;
    *(WORD *)AddressToPatch = Value;
    if(!VirtualProtect((LPVOID)AddressToPatch, 4, OldProtect, &OldProtect))
		return 1;
    return 0;
}

										   
int prepare_backend(HMODULE hLockdown, int file_lock) {
	seed_table patches = seeds[file_lock];
	if(file_lock != 1) {
		int fails = 0;
		fails += patch_dword((DWORD)hLockdown + 0x2067, patches.seed1);
		fails += patch_dword((DWORD)hLockdown + 0x206D, patches.seed2);
		fails += patch_dword((DWORD)hLockdown + 0x2086, patches.seed1);
		fails += patch_word((DWORD)hLockdown + 0x209F, (unsigned short)(patches.seed1 & 0xFF));
		fails += patch_word((DWORD)hLockdown + 0x20B4, (unsigned short)(patches.seed1 & 0xFF));
		return fails;
	}
	return 0;
}


unsigned long get_fileversion(const char *file_path) {
	unsigned long dwBytesRead;
	unsigned long dwSize = GetFileVersionInfoSize((char *)file_path, &dwBytesRead);
	unsigned char *lpbBuffer = (unsigned char *)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if(!lpbBuffer || !GetFileVersionInfo((char *)file_path, NULL, dwSize, lpbBuffer))
		return 0;
	VS_FIXEDFILEINFO *ffi;
	if(!VerQueryValue(lpbBuffer, "\\", (LPVOID*)&ffi, (PUINT)&dwSize))
		return 0;
	unsigned long dwVersion = ((HIWORD(ffi->dwProductVersionMS) & 0xFF) << 24) |
					((LOWORD(ffi->dwProductVersionMS) & 0xFF) << 16) |
					((HIWORD(ffi->dwProductVersionLS) & 0xFF) << 8) |
					(LOWORD(ffi->dwProductVersionLS) & 0xFF);
	VirtualFree(lpbBuffer, 0lu, MEM_RELEASE);
	return dwVersion;
}


void init_context(SHA1_CTX *context, char *shuffled, int len) {
	memset(context->buffer + 0x44, 0x36, 0x40);
	memset(context->buffer + 0x84, 0x5C, 0x40);
	SHA1InitLD(context);
	for (int x = 0; x < len; x++){
		context->buffer[0x44 + x] ^= shuffled[x];
		context->buffer[0x84 + x] ^= shuffled[x];
	}	
	SHA1UpdateLD(context, (const unsigned char *)context->buffer + 0x44, 0x40);	
}


bool hash_videodump(SHA1_CTX *context, const char *videobuf) {
	char *video_image = new char[30720];
	FILE *dump = fopen(videobuf, "rb");
		fread(video_image, sizeof(BYTE), 30720, dump);
	fclose(dump);
	for(int i = 0; i < 48; i++){
		SHA1UpdateLD(context, (unsigned char *)video_image + (i * 640), 0x0D0);		
	}
	delete [] video_image;
	return true;
}


void double_hash(SHA1_CTX *context, unsigned char result[20]) {
	unsigned char output[20];
	SHA1FinalLD(context, output);
	SHA1InitLD(context);
	SHA1UpdateLD(context, (const unsigned char *)context->buffer + 0x84, 0x40);
	SHA1UpdateLD(context, output, 0x14);
	SHA1FinalLD(context, result);
}


unsigned long finish_sub(unsigned long *arg_one, unsigned long *arg_two) {
	unsigned long edi = *arg_one;
	unsigned long esi = *arg_two;
	unsigned long eax = (edi & 0xffff);
	unsigned long ecx = (eax & 0xff00) >> 8;
	unsigned long edx = (eax & 0xff);
	ecx += edx;
	SET_MASKED_BITS(edx, 0xff, (ecx & 0xff));
	ecx = ecx >> 8;
	ecx += edx;
	bool cf = false;
	if((ecx & 0xff) == 0xff)
		cf = true;
	SET_MASKED_BITS(ecx, 0xff, ((ecx + 1) & 0xff));
	if(!cf)
		SET_MASKED_BITS(ecx, 0xff, ((ecx - 1) & 0xff));
	eax -= ecx;
	bool zf = false;
	unsigned char ah = (unsigned char)((eax & 0xff00) >> 8);
	if(ah == 0xff) {
		SET_MASKED_BITS(eax, 0xff00, 0x0100);
	} else {
		SET_MASKED_BITS(eax, 0xff00, 0x0000);
	}	   	
	SET_MASKED_BITS(eax, 0xff, ~((eax) & 0xff) + 1);
	*arg_one = (eax & 0xffff);
	*arg_two = (ecx & 0xffff);
	return *arg_one;
}


int finish(unsigned char *arg_output, unsigned int *arg_output_length,
			unsigned char *arg_heap, int arg_10h) {
	unsigned char *var_output_byte = arg_output;
	int var_return_value = 0, var_iterations1 = 0;
	for(var_return_value = 1; 1; var_iterations1++) {
		int var_heap_index = arg_10h;
		if(!var_heap_index)
			break;
		do {
			if(*(arg_heap + var_heap_index - 1) != 0)
				break;
			arg_10h = var_heap_index;
		} while(var_heap_index--);
		if(!var_heap_index)
			break;
		unsigned long eax = 0;
		unsigned long var_unknown_0C = 0;
		for(var_unknown_0C = 0; 1; eax = var_unknown_0C) {
			var_heap_index--;
			unsigned long cx = *(arg_heap + var_heap_index) & 0xFFFF;
			eax = (eax << 8) + (cx & 0xFFFF);
			unsigned long var_modified_heap_byte = eax;
			finish_sub(&var_modified_heap_byte, &var_unknown_0C);
			*(arg_heap + var_heap_index) = (unsigned char)var_modified_heap_byte;
			if(var_heap_index <= 0)
				break;
		}
		if((unsigned long)var_iterations1 < *arg_output_length) {
            *var_output_byte = (unsigned char)var_unknown_0C + 1;
		} else {
			var_return_value = 0;
		}
		var_output_byte++;
	}
	arg_output_length = (unsigned int*)(var_output_byte - arg_output); 
	return var_return_value;
}


int CheckRevisionLD(const char *file_game, const char *file_strm, const char *file_bttl,
						   const char *server_hash, unsigned long &out_version, unsigned long &out_checksum,
						   char *out_digest, const char *file_lock, const char *file_vdmp) {
	if(HMODULE hLockdown = LoadLibrary("backend.dll")) {
		char *digit_ptr = strchr((char*)file_lock, '.');
		if(!digit_ptr)
			return 1;
		int digit_1 = (int)(*(digit_ptr - 1) - '0');
		int digit_2 = (int)(*(digit_ptr - 2) - '0');
		if(digit_2 == 1)
			digit_1 += 10;
		if(digit_1 < 0 || digit_1 > 19)
			return 2;
		if(prepare_backend(hLockdown, digit_1))
			return 3;
		lcr_shuf shuffle_serverhash	= (lcr_shuf)((char*)hLockdown + 0x1A0E);
		lcr_hash hash_gamefile		= (lcr_hash)((char*)hLockdown + 0x24E1);
		if(!shuffle_serverhash || !hash_gamefile)
			return 4;
		unsigned long file_version = get_fileversion(file_game);
		if(!file_version)
			return 5;
		int hash_length = 0;
		hash_length = strlen(server_hash);
		if(!hash_length || hash_length < 16 || hash_length > 17)
			return 6;
		char shuffled_server_hash[64] = {0};
		if(!shuffle_serverhash(shuffled_server_hash, hash_length, server_hash, hash_length))
			return 7;
		SHA1_CTX context;
		init_context(&context, shuffled_server_hash, hash_length);
		HMODULE file_handles[4] = {0};
		file_handles[0] = LoadLibrary(file_lock);
		file_handles[1] = LoadLibrary(file_game);
		file_handles[2] = LoadLibrary(file_strm);
		file_handles[3] = LoadLibrary(file_bttl);									   
		if (!file_handles[0] || !file_handles[1] || !file_handles[2] || !file_handles[3])
			return 8;						 
		for (int x = 0; x <= 3; x++) {
			hash_gamefile(&context, file_handles[x]);
			FreeLibrary(file_handles[x]);
		}
		if (!hash_videodump(&context, file_vdmp))
			return 9;
		SHA1UpdateLD(&context, (const unsigned char *)"\x01\x00\x00\x00", 4);
		SHA1UpdateLD(&context, (const unsigned char *)"\x00\x00\x00\x00", 4);
		unsigned char dblhash_result[20];
		double_hash(&context, dblhash_result);
		memmove(&out_checksum, dblhash_result, 4);
		unsigned int ret = 17;
		if (!finish((unsigned char*)out_digest, &ret, dblhash_result + 4, 16))
			return 10;
		out_version = file_version;
		FreeLibrary(hLockdown);
		return 0;
	}
	return 11;
}

