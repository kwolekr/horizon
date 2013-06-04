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
#include "loaddll.h"
#include "hashing.h"
#include "srp.h"  									

void SRPCalculateS(char *outbuf, const char *B, const char *salt);
void SRPCalculateK(char *outbuf, const char *S);
unsigned int SRPCalculateU(const char *B);
void SRPCalculateX(BigBuffer x_c, const char *raw_salt);
void SRPCalculateV(BigBuffer v, BigBuffer x);

unsigned char N_raw[32] = {0x87, 0xc7, 0x23, 0x85, 0x65, 0xf6, 0x16, 0x12,
						   0xd9, 0x12, 0x32, 0xc7, 0x78, 0x6c, 0x97, 0x7e,
						   0x55, 0xb5, 0x92, 0xa0, 0x8c, 0xb6, 0x86, 0x21,
						   0x03, 0x18, 0x99, 0x61, 0x8b, 0x1a, 0xff, 0xf8};
											   
unsigned char I_raw[20] = {0x6c, 0x0E, 0x97, 0xED,
						   0x0A, 0xF9, 0x6B, 0xAB,
						   0xB1, 0x58, 0x89, 0xEB,
						   0x8B, 0xBA, 0x25, 0xA4,
						   0xF0, 0x8C, 0x01, 0xF8};

char usernamehash[20];
char usernpasshash[20]; 
BigBuffer g;
BigBuffer n;
BigBuffer a;
BigBuffer A; 
char A_raw[32];
int userlen, passlen;

char a_tmp[32] = {
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1
};


///////////////////////////////////////////////////////////////////////////////


void SRPInit(char *user, char *pass) {
	char asdf[128];			
	userlen = strlen(user);
	passlen = strlen(pass);
	ucasecpy(asdf, user);
	asdf[userlen] = ':';
	ucasecpy(asdf + userlen + 1, pass);
	SHA1(asdf, userlen + 1 + passlen, usernpasshash);
	ucasecpy(asdf, user);
	SHA1(asdf, userlen, usernamehash);
	storm.SBigNew(&n);
	storm.SBigNew(&a);
	storm.SBigNew(&g);
	storm.SBigFromBinary(n, N_raw, 32);
	storm.SBigFromBinary(a, a_tmp, 32);
	storm.SBigFromUnsigned(g, 0x2F);
}


void SRPCalculateA(char *outbuf) {
	unsigned long len = 32;
	storm.SBigNew(&A);
	storm.SBigPowMod(A, g, a, n);
	storm.SBigToBinaryBuffer(A, A_raw, len, &len);
	memcpy(outbuf, A_raw, 32);
}


void SRPCalculateM1(char *outbuf, char *B, char *salt) {
	char K[40], S[32], hashbuf[176];
	SRPCalculateS(S, B, salt);
	SRPCalculateK(K, S);
	memcpy(hashbuf, I_raw, 20);
	memcpy(hashbuf + 20, usernamehash, 20);
	memcpy(hashbuf + 40, salt, 32);
	memcpy(hashbuf + 72, A_raw, 32);
	memcpy(hashbuf + 104, B, 32);
	memcpy(hashbuf + 136, K, 40);
	SHA1(hashbuf, 176, outbuf);
}


void SRPCalculateS(char *outbuf, const char *B, const char *salt) {
	//S = ((N + B - v) % N) ^ (a + u * x) % N;
	BigBuffer tmp, S_exp, S_base, x, v, u_tmp;	
	///////////////////////x and v calculations////////
	storm.SBigNew(&x);
	storm.SBigNew(&v);
	SRPCalculateX(x, salt);		  //x calc
	storm.SBigPowMod(v, g, x, n); //v calc
	///////////////////////////////////base calc //////
	storm.SBigNew(&tmp);
	storm.SBigFromBinary(tmp, B, 32);		//b from raw	
	storm.SBigNew(&S_base);
	storm.SBigCopy(S_base, n);				  //mov eax, n
	storm.SBigAdd(S_base, S_base, tmp);		  //add eax, b
	storm.SBigSub(S_base, S_base, v);		  //sub eax, v
	storm.SBigMod(S_base, S_base, n);		  //mod eax, n
	////////////////////////////////////////////////////
	storm.SBigNew(&S_exp);
	storm.SBigNew(&u_tmp);
	storm.SBigCopy(S_exp, x);						  //mov ebx, x
	storm.SBigFromUnsigned(u_tmp, SRPCalculateU(B));  //mov ecx, u
	storm.SBigMul(S_exp, S_exp, u_tmp);				  //mul ebx, ecx
	storm.SBigAdd(S_exp, S_exp, a);					  //add ebx, a
	storm.SBigDel(u_tmp);
	storm.SBigDel(x);
	storm.SBigDel(v);
	storm.SBigDel(tmp);
	storm.SBigNew(&tmp);
	storm.SBigPowMod(tmp, S_base, S_exp, n);
	unsigned long len = 32;
	storm.SBigToBinaryBuffer(outbuf, tmp, len, &len);
	storm.SBigDel(S_base);
	storm.SBigDel(S_exp);
	storm.SBigDel(tmp);
}


void SRPCalculateX(BigBuffer x, const char *raw_salt) {
	char hash[20], temp[52];
	memcpy(temp, raw_salt, 32);
	memcpy(temp + 32, usernpasshash, 20);
	SHA1(temp, 52, hash);
	storm.SBigFromBinary(x, hash, 20);
}


void SRPCalculateK(char *outbuf, const char *S) {
	char odds[16], evens[16], oddhash[20], evenhash[20];
	char *saltptr = (char *)S;
	char *oddptr  = odds;
	char *evenptr = evens;
	int i;
	for (i = 0; i != 16; i++) {
		*(oddptr++) = *(saltptr++);
		*(evenptr++) = *(saltptr++);
	}
	SHA1(odds, 16, oddhash);
	SHA1(evens, 16, evenhash);
	saltptr = outbuf;
	oddptr  = oddhash;
	evenptr = evenhash;
	for (i = 0; i != 20; i++) {
		*(saltptr++) = *(oddptr++);
		*(saltptr++) = *(evenptr++);
	}
}	 


unsigned int SRPCalculateU(const char *B) {
	char hash[20];
	SHA1((char *)B, 32, hash);
	fastswap32((unsigned __int32 *)hash);
	return *(unsigned int *)hash; // this was the site of *the legendary bug*
}

