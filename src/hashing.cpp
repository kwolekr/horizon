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
#include "hashing.h"

#define LSB4(num) (num)
#define ROL(nr, shift)	((nr << shift) | (nr >> (32 - shift)))

#define SWAP2(num) ((((num) >> 8) & 0x00FF) | (((num) << 8) & 0xFF00))
#define SWAP4(num) ((((num) >> 24) & 0x000000FF) \
				 | (((num) >> 8) & 0x0000FF00) \
				 | (((num) << 8) & 0x00FF0000) \
				 | (((num) << 24) & 0xFF000000))

#define LSB2(num) (num)
#define LSB4(num) (num)
#define MSB2(num) SWAP2(num)
#define MSB4(num) SWAP4(num)

typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
#define W3_KEYLEN 26
#define W3_BUFLEN (W3_KEYLEN << 1)

const unsigned char w2Map[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0xFF, 0x01, 0xFF, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0xFF, 0x0D, 0x0E, 0xFF, 0x0F, 0x10, 0xFF, 0x11, 0xFF, 0x12, 0xFF,
    0x13, 0xFF, 0x14, 0x15, 0x16, 0xFF, 0x17, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0xFF, 0x0D, 0x0E,
    0xFF, 0x0F, 0x10, 0xFF, 0x11, 0xFF, 0x12, 0xFF, 0x13, 0xFF, 0x14, 0x15,
    0x16, 0xFF, 0x17, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};

const unsigned char w3KeyMap[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x00, 0xFF, 0x01, 0xFF, 0x02, 0x03, 0x04, 0x05, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x07, 0x08, 0x09, 0x0A,
    0x0B, 0x0C, 0xFF, 0x0D, 0x0E, 0xFF, 0x0F, 0x10, 0xFF, 0x11, 0xFF, 0x12,
    0xFF, 0x13, 0xFF, 0x14, 0x15, 0x16, 0x17, 0x18, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0xFF, 0x0D,
    0x0E, 0xFF, 0x0F, 0x10, 0xFF, 0x11, 0xFF, 0x12, 0xFF, 0x13, 0xFF, 0x14,
    0x15, 0x16, 0x17, 0x18, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF 
};

const unsigned char w3TranslateMap[] = {
    0x09, 0x04, 0x07, 0x0F, 0x0D, 0x0A, 0x03, 0x0B, 0x01, 0x02, 0x0C, 0x08,
    0x06, 0x0E, 0x05, 0x00, 0x09, 0x0B, 0x05, 0x04, 0x08, 0x0F, 0x01, 0x0E,
    0x07, 0x00, 0x03, 0x02, 0x0A, 0x06, 0x0D, 0x0C, 0x0C, 0x0E, 0x01, 0x04,
    0x09, 0x0F, 0x0A, 0x0B, 0x0D, 0x06, 0x00, 0x08, 0x07, 0x02, 0x05, 0x03,
    0x0B, 0x02, 0x05, 0x0E, 0x0D, 0x03, 0x09, 0x00, 0x01, 0x0F, 0x07, 0x0C,
    0x0A, 0x06, 0x04, 0x08, 0x06, 0x02, 0x04, 0x05, 0x0B, 0x08, 0x0C, 0x0E,
    0x0D, 0x0F, 0x07, 0x01, 0x0A, 0x00, 0x03, 0x09, 0x05, 0x04, 0x0E, 0x0C,
    0x07, 0x06, 0x0D, 0x0A, 0x0F, 0x02, 0x09, 0x01, 0x00, 0x0B, 0x08, 0x03,
    0x0C, 0x07, 0x08, 0x0F, 0x0B, 0x00, 0x05, 0x09, 0x0D, 0x0A, 0x06, 0x0E,
    0x02, 0x04, 0x03, 0x01, 0x03, 0x0A, 0x0E, 0x08, 0x01, 0x0B, 0x05, 0x04,
    0x02, 0x0F, 0x0D, 0x0C, 0x06, 0x07, 0x09, 0x00, 0x0C, 0x0D, 0x01, 0x0F,
    0x08, 0x0E, 0x05, 0x0B, 0x03, 0x0A, 0x09, 0x00, 0x07, 0x02, 0x04, 0x06,
    0x0D, 0x0A, 0x07, 0x0E, 0x01, 0x06, 0x0B, 0x08, 0x0F, 0x0C, 0x05, 0x02,
    0x03, 0x00, 0x04, 0x09, 0x03, 0x0E, 0x07, 0x05, 0x0B, 0x0F, 0x08, 0x0C,
    0x01, 0x0A, 0x04, 0x0D, 0x00, 0x06, 0x09, 0x02, 0x0B, 0x06, 0x09, 0x04,
    0x01, 0x08, 0x0A, 0x0D, 0x07, 0x0E, 0x00, 0x0C, 0x0F, 0x02, 0x03, 0x05,
    0x0C, 0x07, 0x08, 0x0D, 0x03, 0x0B, 0x00, 0x0E, 0x06, 0x0F, 0x09, 0x04,
    0x0A, 0x01, 0x05, 0x02, 0x0C, 0x06, 0x0D, 0x09, 0x0B, 0x00, 0x01, 0x02,
    0x0F, 0x07, 0x03, 0x04, 0x0A, 0x0E, 0x08, 0x05, 0x03, 0x06, 0x01, 0x05,
    0x0B, 0x0C, 0x08, 0x00, 0x0F, 0x0E, 0x09, 0x04, 0x07, 0x0A, 0x0D, 0x02,
    0x0A, 0x07, 0x0B, 0x0F, 0x02, 0x08, 0x00, 0x0D, 0x0E, 0x0C, 0x01, 0x06,
    0x09, 0x03, 0x05, 0x04, 0x0A, 0x0B, 0x0D, 0x04, 0x03, 0x08, 0x05, 0x09,
    0x01, 0x00, 0x0F, 0x0C, 0x07, 0x0E, 0x02, 0x06, 0x0B, 0x04, 0x0D, 0x0F,
    0x01, 0x06, 0x03, 0x0E, 0x07, 0x0A, 0x0C, 0x08, 0x09, 0x02, 0x05, 0x00,
    0x09, 0x06, 0x07, 0x00, 0x01, 0x0A, 0x0D, 0x02, 0x03, 0x0E, 0x0F, 0x0C,
    0x05, 0x0B, 0x04, 0x08, 0x0D, 0x0E, 0x05, 0x06, 0x01, 0x09, 0x08, 0x0C,
    0x02, 0x0F, 0x03, 0x07, 0x0B, 0x04, 0x00, 0x0A, 0x09, 0x0F, 0x04, 0x00,
    0x01, 0x06, 0x0A, 0x0E, 0x02, 0x03, 0x07, 0x0D, 0x05, 0x0B, 0x08, 0x0C,
    0x03, 0x0E, 0x01, 0x0A, 0x02, 0x0C, 0x08, 0x04, 0x0B, 0x07, 0x0D, 0x00,
    0x0F, 0x06, 0x09, 0x05, 0x07, 0x02, 0x0C, 0x06, 0x0A, 0x08, 0x0B, 0x00,
    0x0F, 0x04, 0x03, 0x0E, 0x09, 0x01, 0x0D, 0x05, 0x0C, 0x04, 0x05, 0x09,
    0x0A, 0x02, 0x08, 0x0D, 0x03, 0x0F, 0x01, 0x0E, 0x06, 0x07, 0x0B, 0x00,
    0x0A, 0x08, 0x0E, 0x0D, 0x09, 0x0F, 0x03, 0x00, 0x04, 0x06, 0x01, 0x0C,
    0x07, 0x0B, 0x02, 0x05, 0x03, 0x0C, 0x04, 0x0A, 0x02, 0x0F, 0x0D, 0x0E,
    0x07, 0x00, 0x05, 0x08, 0x01, 0x06, 0x0B, 0x09, 0x0A, 0x0C, 0x01, 0x00,
    0x09, 0x0E, 0x0D, 0x0B, 0x03, 0x07, 0x0F, 0x08, 0x05, 0x02, 0x04, 0x06, 
    0x0E, 0x0A, 0x01, 0x08, 0x07, 0x06, 0x05, 0x0C, 0x02, 0x0F, 0x00, 0x0D,
    0x03, 0x0B, 0x04, 0x09, 0x03, 0x08, 0x0E, 0x00, 0x07, 0x09, 0x0F, 0x0C,
    0x01, 0x06, 0x0D, 0x02, 0x05, 0x0A, 0x0B, 0x04, 0x03, 0x0A, 0x0C, 0x04,
    0x0D, 0x0B, 0x09, 0x0E, 0x0F, 0x06, 0x01, 0x07, 0x02, 0x00, 0x05, 0x08
};


///////////////////////////////////////////////////////////////////////////////


void HashCDKey(char *OutBuf,
			   unsigned long ClientToken, 
			   unsigned long ServerToken, 
			   unsigned long ProductVal,
			   unsigned long PublicVal, 
			   unsigned long PrivateVal) {
	DWORD dwHashBuff[6];
	DWORD dwHashResult[5];
	dwHashBuff[0] = ClientToken;
	dwHashBuff[1] = ServerToken;
	dwHashBuff[2] = ProductVal;
	dwHashBuff[3] = PublicVal;
	dwHashBuff[4] = 0;
	dwHashBuff[5] = PrivateVal;
	BSHA1(dwHashBuff, 24, dwHashResult);
	memcpy(OutBuf, dwHashResult, 20);
}


void HashWAR3Key(unsigned long ClientToken, 
				 unsigned long ServerToken, 
				 unsigned long ProductVal, 
				 unsigned long PublicVal,
				 char *PrivateVal,
				 char *output) { 
	char buffer[26];
	*(unsigned long *)buffer = ClientToken;
	*(unsigned long *)(buffer + 0x04) = ServerToken;
	*(unsigned long *)(buffer + 0x08) = ProductVal;
	*(unsigned long *)(buffer + 0x0C) = PublicVal;
	memcpy(buffer + 0x10, PrivateVal, 10); 
	SHA1(buffer, 26, output);
}


void SHA1(char *source, unsigned long len, char *output) {
	unsigned long hashlen = 20;
	CryptHashCertificate(NULL, 0, 0, (const unsigned char *)source,
		len, (unsigned char *)output, &hashlen);
}


void MD5(char *source, unsigned long len, char *output) {
	unsigned long hashlen = 16;
	CryptHashCertificate(NULL, CALG_MD5, 0, (const unsigned char *)source,
		len, (unsigned char *)output, &hashlen);
}


void BSHA1(const void *src, const int len, unsigned long *result) {
    unsigned long a = 0x67452301lu, b = 0xefcdab89lu;
    unsigned long c = 0x98badcfelu, d = 0x10325476lu;
    unsigned long e = 0xc3d2e1f0lu, g;
    unsigned char bBuffer [320] = {0};
    memcpy(bBuffer, src, len);
    unsigned long *lpdwBuffer = (unsigned long *) bBuffer;
    for (int i = 0; i < 80; ++i) {
		if (i < 64)
			lpdwBuffer [i + 16] = ROL(1, (lpdwBuffer [i] ^ lpdwBuffer [i + 8] ^ lpdwBuffer [i + 2] ^ lpdwBuffer [i + 13]) % 32);
		if (i < 20)
			g = lpdwBuffer[i] + ROL(a, 5) + e + ((b & c) | (~b & d)) + 0x5a827999lu;
		else if (i < 40)
			g = (d ^ c ^ b) + e + ROL(g, 5) + lpdwBuffer[i] + 0x6ed9eba1lu;
		else if (i < 60)
			g = lpdwBuffer[i] + ROL(g, 5) + e + ((c & b) | (d & c) | (d & b)) - 0x70e44324lu;
		else
			g = (d ^ c ^ b) + e + ROL(g, 5) + lpdwBuffer[i] - 0x359d3e2alu;
		e = d;
		d = c;
		c = ROL(b, 30);
		b = a;
		a  = g;
    }
    result [0] = 0x67452301lu + g;
    result [1] = 0xefcdab89lu + b;
    result [2] = 0x98badcfelu + c;
    result [3] = 0x10325476lu + d;
    result [4] = 0xc3d2e1f0lu + e;
}


bool DecodeStarcraftKey(char *key) {
   DWORD n, n2, v, v2;
   BYTE c2, c;
   bool bValid;
   int i;
   v = 3;
   for (i = 0; i < 12; i++) {
      c = key[i];
      n = Get_Num_Value(c);
      n2 = v * 2;
      n ^= n2;
      v += n;
   }
   v %= 10;
   if (Get_Hex_Value(v) == key[12])
      bValid = true;
   else
      bValid = false;
   v = 194;
   for (i = 11; v >= 7, i >= 0; i--) {
      c = key[i];
      n = v / 12;
      n2 = v % 12;
      v -= 17;
      c2 = key[n2];
      key[i] = c2;
      key[n2] = c;
   }
   v2 = 0x13AC9741;
   for (i = 11; i >= 0; i--) {
      c = toupper(key[i]);
      key[i] = c;
      if (c <= '7') {
         v = v2;
         c2 = (unsigned char)v & 0xFF;
         c2 &= 7;
         c2 ^= c;
         v >>= 3;
         key[i] = c2;
         v2 = v;
      }
      else if (c < 'A') {
         c2 = (BYTE) i;
         c2 &= 1;
         c2 ^= c;
         key[i] = c2;
      }
   }
   return bValid;
}


bool DecodeWarcraft2Key(char *cdkey) {
    unsigned long r = 1, n, n2, v, v2, checksum = 0;
    unsigned char c1, c2, c;
	int i, j;
    for (i = 0; i < 16; i += 2) {
        c1 = w2Map[(int) cdkey[i]];
        n = c1 * 3;
        c2 = w2Map[(int) cdkey[i + 1]];
        n = c2 + n * 8;
        if (n >= 0x100) {
            n -= 0x100;
            checksum |= r;
        }
        n2 = n >> 4;
        cdkey[i] = Get_Hex_Value(n2);
        cdkey[i + 1] = Get_Hex_Value(n);
        r <<= 1;
    }
    v = 3;
    for (i = 0; i < 16; i++) {
        c = cdkey[i];
        n = Get_Num_Value(c);
        n2 = v * 2;
        n ^= n2;
        v += n;
    }
    v &= 0xFF;
    if (v != checksum) {
        return 0;
    }
    n = n ^ n;
    for (j = 15; j >= 0; j--) {
        c = cdkey[j];
        if (j > 8) {
            n = (j - 9);
        } else {
            n = (0xF - (8 - j));
        }
        n &= 0xF;
        c2 = cdkey[n];
        cdkey[j] = c2;
        cdkey[n] = c;
    }
    v2 = 0x13AC9741;
    for (j = 15; j >= 0; j--) {
        c = toupper(cdkey[j]);
        cdkey[j] = c;
        if (c <= '7') {
            v = v2;
            c2 = ((char) (v & 0xFF)) & 7 ^ c;
            v >>= 3;
            cdkey[j] = (char) c2;
            v2 = v;
        } else if (c < 'A') {
            cdkey[j] = ((char) j) & 1 ^ c;
        }
    }
    return 1;
}


char Get_Hex_Value(unsigned long v) {
   v &= 0x0F;
   if (v < 10)
      return ((unsigned char)v + 0x30);
   else
      return ((unsigned char)v + 0x37);
}


int Get_Num_Value(char c) {
   c = toupper(c);
   if (isdigit(c))
      return (c - 0x30);
   else
      return (c - 0x37);
}


void doubleHashPassword(const char* password, unsigned int clientToken, unsigned int serverToken, char* outBuffer) {
    char intermediate[28];
    unsigned int* lp;
	BSHA1(password, strlen(password), (unsigned long *)(intermediate + 8));
    lp = (unsigned int*) &intermediate;
    lp[0] = clientToken;
    lp[1] = serverToken;
	BSHA1(intermediate, 28, (unsigned long *)outBuffer);
}


void SHA1InitLD(SHA1_CTX* context) {   
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


void SHA1UpdateLD(SHA1_CTX* context, const unsigned char* data, unsigned int len) {
	unsigned int i, j;
    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
    context->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
        SHA1TransformLD(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            SHA1TransformLD(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


void SHA1FinalLD(SHA1_CTX* context, unsigned char digest[20]) {			
	unsigned char buffer[64];
	int i;
	buffer[0] = (unsigned char)0x80;	
	for (int p = 1; p < sizeof(buffer); p++) {
		buffer[p] = (unsigned char)0x00;
	}
	unsigned char finalcount[8];
    for (i = 0; i < 8; i++)
		finalcount[7-i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8) ) & 255);
	LARGE_INTEGER li;
	li.LowPart = context->count[1];
	li.HighPart = context->count[0];
	li.QuadPart >>= 0x03;		
	context->count[1] >>= 0x03;	
	unsigned int len = ((0xFFFFFFF7 - li.HighPart) & 0x3f) + 1;	
	SHA1UpdateLD(context, (unsigned char*)buffer, len);
	SHA1UpdateLD(context, finalcount, 8);
	for (i = 0; i < 20; i++)
		digest[i] = (unsigned char)((context->state[i>>2] >> ((3-((19-i) & 3)) * 8) ) & 255);
}


void SHA1TransformLD(unsigned int state[5], const unsigned char buffer[64]) {
	unsigned int a, b, c, d, e;
	typedef union {
		unsigned char c[64];
		unsigned int l[16];
	} CHAR64LONG16;
	CHAR64LONG16* block;
    block = (CHAR64LONG16*)buffer;
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;   
    a = b = c = d = e = 0;
}


bool DecodeWC3Key(char *cdkey, unsigned int *product, unsigned int *value1, char *w3value2) {
    char table[W3_BUFLEN];
    int values[4];
    int a, b = 0x21, i;
    char decode;
    memset(table, 0, W3_BUFLEN);
    memset(values, 0, (sizeof(int) * 4));
    for (i = 0; ((unsigned int) i) < W3_KEYLEN; i++) {
        cdkey[i] = toupper(cdkey[i]);
        a = (b + 0x07B5) % W3_BUFLEN;
        b = (a + 0x07B5) % W3_BUFLEN;
        decode = w3KeyMap[cdkey[i]];
        table[a] = (decode / 5);
        table[b] = (decode % 5);
    }   
    i = W3_BUFLEN;
    do {
        mult(4, 5, values + 3, table[i - 1]);
    } while (--i);
    decodeKeyTable(values);
	*product = values[0] >> 0xA;
	*product = MSB4(SWAP4(*product));
	for (i = 0; i < 4; i++) {
		values[i] = MSB4(values[i]);
	}
	*value1 = MSB4(LSB4(*(uint32_t*) (((char*) values) + 2)) & 0xFFFFFF00);
	*((uint16_t*) w3value2) = MSB2(*(uint16_t*) (((char*) values) + 6));
	*((uint32_t*) ((char*) w3value2 + 2)) = MSB4(*(uint32_t*) (((char*) values) + 8));
	*((uint32_t*) ((char*) w3value2 + 6)) = MSB4(*(uint32_t*) (((char*) values) + 12));
	return true;
}


inline void mult(int r, const int x, int* a, int dcByte) {
    while (r--) {
        int64_t edxeax = ((int64_t) (*a & 0x00000000FFFFFFFFl))
            * ((int64_t) (x & 0x00000000FFFFFFFFl));
        *a-- = dcByte + (int32_t) edxeax;
        dcByte = (int32_t) (edxeax >> 32);
    }
}


void decodeKeyTable(int* keyTable) {
    unsigned int eax, ebx, ecx, edx, edi, esi, ebp;
    unsigned int varC, var4, var8;
    unsigned int copy[4];
    unsigned char* scopy;
    int* ckt;
    int ckt_temp;
    var8 = 29;
    int i = 464;
    do {
        int j;
        esi = (var8 & 7) << 2;
        var4 = var8 >> 3;
        varC = keyTable[3 - var4];
        varC &= (0xF << esi);
        varC = varC >> esi;
        if (i < 464) {
            for (j = 29; (unsigned int) j > var8; j--) {
                ecx = (j & 7) << 2;
                ebp = (keyTable[0x3 - (j >> 3)]);
                ebp &= (0xF << ecx);
                ebp = ebp >> ecx;
                varC = w3TranslateMap[ebp ^ w3TranslateMap[varC + i] + i];
            }
        }
        j = --var8;
        while (j >= 0) {
            ecx = (j & 7) << 2;
            ebp = (keyTable[0x3 - (j >> 3)]);
            ebp &= (0xF << ecx);
            ebp = ebp >> ecx;
            varC = w3TranslateMap[ebp ^ w3TranslateMap[varC + i] + i];
            j--;
        }
        j = 3 - var4;
        ebx = (w3TranslateMap[varC + i] & 0xF) << esi;
        keyTable[j] = (ebx | ~(0xF << esi) & ((int) keyTable[j]));
    } while ((i -= 16) >= 0);
    eax = 0;
    edx = 0;
    ecx = 0;
    edi = 0;
    esi = 0;
    ebp = 0;
    for (i = 0; i < 4; i++) {
        copy[i] = LSB4(keyTable[i]);
    }
    scopy = (unsigned char*) copy;
    for (edi = 0; edi < 120; edi++) {
        unsigned int location = 12;
        eax = edi & 0x1F;
        ecx = esi & 0x1F;
        edx = 3 - (edi >> 5);
        location -= ((esi >> 5) << 2);
        ebp = *(int*) (scopy + location);
        ebp = LSB4(ebp);
        ebp &= (1 << ecx);
        ebp = ebp >> ecx;
        ckt = (keyTable + edx);
        ckt_temp = *ckt;
        *ckt = ebp & 1;
        *ckt = *ckt << eax;
        *ckt |= (~(1 << eax) & ckt_temp);
        esi += 0xB;
        if (esi >= 120)
            esi -= 120;
    }
}

