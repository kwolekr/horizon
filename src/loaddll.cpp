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
#include "loaddll.h"

HMODULE hBNCSUtil, hCRLib, hStorm;

BNCSUTIL bncsutil;
STORM storm;
lpfn_CheckRevisionEx CheckRevisionEx;

const char *names[NUM_BNCSUTILFXNS] = {
	"kd_quick",
	"checkRevisionFlat",
	"getExeInfo",
	"nls_init",
	"nls_free",
	"nls_account_create",
	"nls_account_logon",
	"nls_check_M2",
	"nls_account_change_proof",
	"nls_get_A",
	"nls_get_M1",
	"nls_get_S",
	"nls_get_K"
};

int stormordinals[NUM_STORMFXNS] = {
	624, 606, 628, 612, 609, 638,
	601, 636, 622, 603, 604, 621,
	262, 272, 266, 268, 265, 269,
	253, 252
};


///////////////////////////////////////////////////////////////////////////////


void InitalizeExternalLibs() {
	int i;
	hBNCSUtil = LoadLibrary("BNCSUtil.dll");
	if (!hBNCSUtil) {
		AddChat(vbRed, "Unable to load BNCSUtil.dll!", hWnd_status_re);	
	} else {
		for (i = 0; i != NUM_BNCSUTILFXNS; i++)
			*((void **)((char *)&bncsutil + i * sizeof(void *))) = (void *)GetProcAddress(hBNCSUtil, names[i]);
	}
	hCRLib = LoadLibrary("CheckRevision.dll");
	if (!hCRLib)
		AddChat(vbRed, "Unable to load CheckRevision.dll!", hWnd_status_re);
	CheckRevisionEx = (lpfn_CheckRevisionEx)GetProcAddress(hCRLib, "CheckRevisionEx");
	hStorm = LoadLibrary("Storm.dll");
	if (!hStorm) {
		AddChat(vbRed, "Unable to load Storm.dll!", hWnd_status_re);
	} else {
		for (i = 0; i != NUM_STORMFXNS; i++) {
			*(void **)(((char *)&storm + (i * sizeof(void *)))) =
				(void **)GetProcAddress(hStorm, MAKEINTRESOURCE(stormordinals[i]));
		}
	}
}		


void FreeExternalLibs() {
	FreeLibrary(hBNCSUtil);
	FreeLibrary(hCRLib);
	FreeLibrary(hStorm);
}


#pragma warning(disable : 4035)
bool CallCheckRevision(LPCHECKREVISIONPARAMS lpcrp, char *paramstr) {
	__asm {
		push ebx
		//mov ebx, dword ptr [CheckRevision] //FIXME
		cmp byte ptr [ebx], 55h
		jnz notstd
		mov edx, dword ptr [ebp + 12]
		mov ecx, edx
strlentop:
		mov al, byte ptr [ecx]
		inc ecx
		test al, al
		jnz strlentop
		dec ecx
		sub ecx, edx
		mov edx, dword ptr [ebp + 8]
pushtop:
		movzx eax, byte ptr [eax + ecx]
		sub eax, 'a'					
		movzx eax, byte ptr [blah + eax]
		push dword ptr [edx + eax]
		dec ecx				
		jnz pushtop
		call ebx
		jmp retlbl
notstd:
	//	push notstdstr
	//	call OutputErr
retlbl:
		pop ebx
		leave
		ret 8

blah:   __emit 0  //a
		__emit 8  //b
		__emit 36 //c
		__emit 12 //d
		__emit 0  //e
		__emit 24 //f
		__emit 0  //g
		__emit 0  //h
		__emit 28 //i
		__emit 0  //j
		__emit 0  //k
		__emit 0  //l
		__emit 16 //m
		__emit 20 //n
		__emit 0  //o
		__emit 0  //p
		__emit 0  //q
		__emit 0  //r
		__emit 4  //s
		__emit 0  //t
		__emit 40 //u
		__emit 32 //v
		__emit 0  //w
		__emit 0  //x
		__emit 0  //y
		__emit 0  //z
//notstdstr:
		//db "Only __stdcall calling convention supported!", 0
	}
}
#pragma warning(default : 4035)

