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


///////////////////////////////////////////////////////////////////////////////


void ChainInsertItem(LPUSER lpUser, bool first, int index) {
	if (first) {
		if (!bot[index]->lastadded) {
			bot[index]->lastadded = lpUser;
			lpUser->nextlvuser = NULL;
		} else {
			lpUser->nextlvuser = bot[index]->firstlvuser;
			bot[index]->firstlvuser->lastlvuser = lpUser;	
		}
		lpUser->lastlvuser = NULL;
		bot[index]->firstlvuser = lpUser;
	} else {
		lpUser->nextlvuser = NULL;
		lpUser->lastlvuser = bot[index]->lastadded;
		if (!bot[index]->lastadded)
			bot[index]->firstlvuser = lpUser;
		else 												
			bot[index]->lastadded->nextlvuser = lpUser;				
		bot[index]->lastadded = lpUser;
	}
}


void ChainRemoveItem(LPUSER lpUser, int index) {
	if (lpUser->nextlvuser)
			((LPUSER)lpUser->nextlvuser)->lastlvuser = lpUser->lastlvuser; 
	else
		bot[index]->lastadded = (LPUSER)lpUser->lastlvuser;

	if (lpUser->lastlvuser)
		((LPUSER)lpUser->lastlvuser)->nextlvuser = lpUser->nextlvuser;
	else
		bot[index]->firstlvuser = (LPUSER)lpUser->nextlvuser;
}


void ChainMoveToTop(LPUSER lpUser, int index) {
	if (lpUser->nextlvuser)
		((LPUSER)lpUser->nextlvuser)->lastlvuser = lpUser->lastlvuser;
	else
		bot[index]->lastadded = (LPUSER)lpUser->lastlvuser;

	if (lpUser->lastlvuser)
		((LPUSER)lpUser->lastlvuser)->nextlvuser = lpUser->nextlvuser;

	lpUser->lastlvuser = NULL;
	lpUser->nextlvuser = bot[index]->firstlvuser;
	((LPUSER)bot[index]->firstlvuser)->lastlvuser = lpUser;
	bot[index]->firstlvuser = lpUser;
}

