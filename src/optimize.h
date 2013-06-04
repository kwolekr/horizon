#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0600
#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0601
#define _CRT_SECURE_NO_WARNINGS 1
//#define NTDDI_VERSION NTDDI_WIN2K

#include <windows.h>
#include <winuser.h>
#include <wincrypt.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <winsock2.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Version.lib")

#pragma warning(disable : 4800)

#pragma optimize("gsy",on)
//#pragma comment(linker,"/RELEASE")
//#pragma comment(linker,"/ignore:4078")
//#pragma comment(linker,"/FILEALIGN:0x200")
//#pragma comment(linker,"/opt:nowin98")
#pragma comment(linker,"/ALIGN:4096")
//#pragma comment(linker,"/merge:.rdata=.data")
//#pragma comment(linker,"/merge:.text=.data")
//#pragma comment(linker,"/merge:.reloc=.data")