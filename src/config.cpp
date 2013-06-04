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
#include "gui.h"
#include "resource.h"
#include "botprofiles.h"
#include "config.h"
#include "hashtable.h"
#include "atmosphere.h"
#include <commdlg.h>

#define NUMPAGES 6
#define NUM_CLIENTSTRS 11
#define NUM_PLATFORMSTRS 3
#define NUM_TABPOSSTRS 4
#define NUM_PAGE2CHKS 10
#define NUM_PAGE4CHKS 10
#define NUM_TXTS 38
#define NUM_TXTNONGLOBALS 19

HWND hWnd_page[NUMPAGES];
HWND hWnd_currpage;
HWND hWnd_lblclr1, hWnd_lblclr2;
HBRUSH ghBrushes[2];

HFONT hFontTmp;		   

int activeprofile;

char *pagetitles[] = {
	"Basic Connection",
	"Advanced Connection",
	"Interface",
	"Messaging",
	"Misceallenous",
	"Profiles"
};

const char *clientstrs[] = {
	"STAR",
	"SEXP",
	"SSHR",
	"JSTR",

	"DRTL",
	"DSHR",
	"D2DV",
	"D2XP",

	"W2BN",
	"WAR3",
	"W3XP"
};

const char *platformstrs[] = {
	"IX86",
	"PMAC",
	"XMAC"
};

const char *tabposstrs[] = {
	"Top",
	"Bottom",
	"Left",
	"Right"
};

struct chkdescriptor_nolocal {
	char *key;
	int res;
};
	   
struct chkdescriptor {
	char *key;
	int res;
	bool globalsetting;
};

struct txtdescriptor {
	char *section;
	char *key;
	int res;
	int page;
};

const struct chkdescriptor f2confchks[] = {
	{"MergeLVandRTB", CONFIG_CHKMERGELVRTB, true}, 
	{"Gradient", CONFIG_CHKGRADIENT, true},
	{"WarnClosingApp", CONFIG_CHKWARNEXIT, true},
	{"WarnClosingProfile", CONFIG_CHKWARNPROFILE, true},
	{"TextPing", CONFIG_CHKTEXTPING, true},
	{"UseAtmosphere", CONFIG_CHKMAXIMIZEMDI, true},
	{"ClearRTB", CONFIG_CHKCLEARRTB, true},	
	{"UseGlass", CONFIG_CHKUSEGLASS, true},
	{"UseRTBImg", CONFIG_CHKUSERTBIMG, false},
	{"UseLVImg", CONFIG_CHKUSELVIMG, false}	
};

const struct chkdescriptor_nolocal f4confchks[] = {
	{"UseAntiflood", CONFIG_CHKUSEANTIFLOOD},
	{"UseFloodProtection", CONFIG_CHKUSEFLOODPROTECT},
	{"FilterNumber", CONFIG_CHKFILTERNUMS},
	{"ShowVoidUL", CONFIG_CHKSHOWVOID},
	{"ShowAccInfo", CONFIG_CHKSHOWINFO},
	{"JoinPubChan", CONFIG_CHKJOINPUBCHAN},
	{"URLDetect", CONFIG_CHKURLDETECT},
	{"LogChat", CONFIG_CHKLOGCHAT},
	{"RTFLogs", CONFIG_CHKFORMATLOGS},
	{"Decode", CONFIG_CHKDECODE}
};	  									

const struct txtdescriptor txtdescriptors[] = {
	{"Main", "Username",   CONFIG_TXTUSER,		 0},
	{"Main", "Password",   CONFIG_TXTPASS,		 0},
	{"Main", "Server",     CONFIG_CBOSERVER,	 0},
	{"Main", "BNLSServer", CONFIG_CBOBNLSSERVER, 0},
	{"Main", "Client",     CONFIG_CBOCLIENT,     0},
	{"Main", "Home",	   CONFIG_TXTHOME,		 0},
	{"Main", "CDKey",	   CONFIG_CBOCDKEY,      0},
	{"Main", "ExpKey",	   CONFIG_TXTEXPKEY,     0},
	{"Main", "Ping",	   CONFIG_TXTPINGCUSTOMVAL, 0},
	{"Main", "CDKeyOwner", CONFIG_TXTCDKEYOWNER, 1},
	{"Main", "Platform",   CONFIG_CBOPLATFORM,   1},
	{"Main", "NewPass",	   CONFIG_TXTNEWPASS,    1},
	{"Main", "Email",	   CONFIG_TXTEMAIL1,     1},
	{"Main", "NewEmail",   CONFIG_TXTEMAIL2,     1},
	{"Main", "Proxy",	   CONFIG_CBOPROXY,      1},
	{"GnI", "JoinGreet",   CONFIG_TXTJOINGREET,  3},
	{"GnI", "LeaveGreet",  CONFIG_TXTLEAVEGREET, 3},
	{"GnI", "IdleMsg",     CONFIG_TXTIDLE,       3},
	{"GnI", "IdleInterval", CONFIG_TXTIDLEINTERVAL, 3}, //19

	{"Font", "FaceName",   CONFIG_CBOFONT,       2},
	{"Font", "Size",       CONFIG_TXTFONTSIZE,   2},
	{"Main", "CountryName", CONFIG_TXTCOUNTRYNAME, 1},
	{"Main", "CountryAbbriv", CONFIG_TXTCOUNTRYABBRIV, 1},
	{"Interface", "MaxRTBLen", CONFIG_TXTMAXRTBLEN, 2},
	{"Interface", "CharsToClear", CONFIG_TXTCHARSTOCLEAR, 2},
	{"Misc", "PerPacket", CONFIG_TXTPERPACKET,  4},			   
	{"Misc", "PerByte", CONFIG_TXTPERBYTE,      4},
	{"Misc", "NumFilter", CONFIG_TXTNUMFILTER,  4},
	{"Misc", "ReconnectRate", CONFIG_TXTRCRATE, 4},
	{"Misc", "XRChannel", CONFIG_TXTXRCHANNEL,  4},
	{"Misc", "XRFactor", CONFIG_TXTXRFACTOR,    4},
	{"Verbytes", "STAR", CONFIG_TXTVERBYTESTAR, 1},
	{"Verbytes", "SEXP", CONFIG_TXTVERBYTESTAR, 1},
	{"Verbytes", "D2DV", CONFIG_TXTVERBYTED2DV, 1},
	{"Verbytes", "D2XP", CONFIG_TXTVERBYTED2DV, 1},
	{"Verbytes", "WAR3", CONFIG_TXTVERBYTEWAR3, 1},
	{"Verbytes", "W3XP", CONFIG_TXTVERBYTEWAR3, 1},
	{"Verbytes", "W2BN", CONFIG_TXTVERBYTEW2BN, 1}  //38

};


///////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK ConfigProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	int i;
	TCITEM ti;
	HWND hWnd_tmp;
	char blah[64];
	switch (message) {
		case WM_INITDIALOG:
			activeprofile = lParam;
			hWnd_tmp = GetDlgItem(hDialog, CONFIG_LSTPROFILES);
			for (i = 0; i != numprofiles; i++) 
				SendMessage(hWnd_tmp, LB_ADDSTRING, 0, (LPARAM)bot[i]->pname);

			SendMessage(hWnd_tmp, LB_SETCURSEL, lParam == -1 ? 0 : lParam, 0);

			hWnd_tmp = GetDlgItem(hDialog, IDC_TAB1);
			SetWindowLong(hWnd_tmp, GWL_STYLE, GetWindowLong(hWnd_tmp, GWL_STYLE) | TCS_HOTTRACK);

			//ImageList_Add(hImageList, hbIcons, NULL);
			SendMessage(hWnd_tmp, TCM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)hCfgTabiml);

			ti.mask = TCIF_TEXT | TCIF_IMAGE; 
			for (i = 0; i != NUMPAGES; i++) {
				ti.iImage = i;
				ti.pszText = pagetitles[i];
				TabCtrl_InsertItem(hWnd_tmp, i, &ti);
				hWnd_page[i] = CreateDialogParam(GetModuleHandle(NULL),
					MAKEINTRESOURCE(IDD_CONFIGPAGE0 + i), hDialog,
					(DLGPROC)ConfigCtlProc, i);
			}
			TabChange(hDialog);
			InitalizeWindowEnabledStates(hDialog);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case CONFIG_CMDOK:
					SaveSettings(hDialog, activeprofile);
					LoadGlobalSettings();
					SetGlobalAttributes();
					if (bot[activeprofile]->ploaded) {
						//i = GetBotFromProfile(activeprofile);
						AssignStructFromConfig(activeprofile, bot[activeprofile]->pname);
					}
					
					GetDlgItemText(hWnd_page[2], CONFIG_CBOFONT, blah, sizeof(blah));
					SetNewFont(blah, GetDlgItemInt(hWnd_page[2], CONFIG_TXTFONTSIZE, NULL, true));
					ResetTableContents(channels, TABLESIZE_CHANNELS);
					LoadAtmosphere();
					if (gfstate & GFS_UIATMOSPHERE)
						EnableAtmosphere();
					else
						DisableAtmosphere();
				case CONFIG_CMDCANCEL:							 
					DeleteObject(hFontTmp);
					DeleteObject(ghBrushes[0]);
					DeleteObject(ghBrushes[1]);
					DestroyWindow(hDialog);
					break;
				case CONFIG_LSTPROFILES:
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						activeprofile = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
						for (i = 0; i != 5; i++)
							UpdateConfigPage(hWnd_page[i], activeprofile, i);
					}
					break;
			}	    
			break; 
		case WM_NOTIFY:
			if (LOWORD(wParam) == IDC_TAB1) {
				if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
					TabChange(hDialog);
			}
			break;
		case WM_SIZE:
			UpdateWindow(hDialog);
			break;
		case WM_DESTROY:
			if (hFontTmp)
				DeleteObject(hFontTmp);
			hFontTmp = NULL;
			hWnd_Config = NULL;
			break;
		case WM_CLOSE:
			DestroyWindow(hDialog);
			break;
		default:
			DefWindowProc(hDialog, message, wParam, lParam);
    }
	return false;
}


LRESULT CALLBACK ConfigCtlProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hWndtmp;
	FILE *file;
	char str[64], asdf[MAX_PATH], newprofile[32];
	int i, index, i2;
	switch (message) {
		case WM_INITDIALOG:
			index = activeprofile;
			switch (lParam) {
				case 0:
					hWndtmp = GetDlgItem(hDialog, CONFIG_CBOCLIENT);
					for (i = 0; i != NUM_CLIENTSTRS; i++)
						SendMessage(hWndtmp, CB_ADDSTRING, 0, (LPARAM)clientstrs[i]);
					hWndtmp = GetDlgItem(hDialog, CONFIG_CBOCDKEY);
					*str = 0;
					file = fopen("CDKeys.txt", "r");
					if (file) {
						while (!feof(file)) {
							fgets(str, sizeof(str), file);
							str[strlen(str) - 1] = 0;
							SendMessage(hWndtmp, CB_ADDSTRING, 0, (LPARAM)str);
						}
						fclose(file);
					}
					hWndtmp = GetDlgItem(hDialog, CONFIG_CBOSERVER);
					file = fopen("Servers.txt", "r");
					if (file) {
						while (!feof(file)) {
							fgets(str, sizeof(str), file);
							if ((strlen(str) == 9) && (*(int *)(str + 2) == 'SLNB')) {
								hWndtmp = GetDlgItem(hDialog, CONFIG_CBOBNLSSERVER);
							} else {
								str[strlen(str) - 1] = 0;
								SendMessage(hWndtmp, CB_ADDSTRING, 0, (LPARAM)str);	
							}
						}
						fclose(file);
					}
					break;
				case 1:
					hWndtmp = GetDlgItem(hDialog, CONFIG_CBOPLATFORM);
					for (i = 0; i != NUM_PLATFORMSTRS; i++)
						SendMessage(hWndtmp, CB_ADDSTRING, 0, (LPARAM)platformstrs[i]);
					hWndtmp = GetDlgItem(hDialog, CONFIG_CBOPROXY);
					*str = 0;
					file = fopen("Proxies.txt", "r");
					if (file) {
						while (!feof(file)) {
							fgets(str, sizeof(str), file);
							str[strlen(str) - 1] = 0;
							SendMessage(hWndtmp, CB_ADDSTRING, 0, (LPARAM)str);
						}  
						fclose(file);
					}
					break;					 
				case 2:
					HDC hdc;

					//TODO: clean this
					LOGFONT logfont;
					logfont.lfCharSet = DEFAULT_CHARSET;
					logfont.lfFaceName[0] = 0;
					logfont.lfPitchAndFamily = 0;
					hdc = GetDC(hWnd_status_re);
					EnumFontFamiliesEx(hdc, &logfont,
						(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hDialog, 0);
					ReleaseDC(hWnd_status_re, hdc);
					hWndtmp = GetDlgItem(hDialog, CONFIG_CBOTABPOS);
					for (i = 0; i != NUM_TABPOSSTRS; i++)
						SendMessage(hWndtmp, CB_ADDSTRING, 0, (LPARAM)tabposstrs[i]);
					hWnd_lblclr1 = GetDlgItem(hDialog, CONFIG_LBLCLR1);
					hWnd_lblclr2 = GetDlgItem(hDialog, CONFIG_LBLCLR2);
					ghBrushes[0] = CreateSolidBrush(gradclr1);
					ghBrushes[1] = CreateSolidBrush(gradclr2);
					SendMessage(GetDlgItem(hDialog, CONFIG_SLDGLASS),
						TBM_SETRANGE, false, MAKELONG(0, 255));
					break;
				case 3:
					break;
				case 4:
					hWndtmp = GetDlgItem(hDialog, CONFIG_CBOCLIENTS2);
					for (i = 0; i != NUM_CLIENTSTRS; i++)
						SendMessage(hWndtmp, CB_ADDSTRING, 0, (LPARAM)clientstrs[i]);
					SendMessage(hWndtmp, CB_SETCURSEL, 0, 0);
					break;
				case 5:
					for (i = 0; i != numprofiles; i++) {
						SendMessage(GetDlgItem(hDialog, bot[i]->ploadonstart ?
							CONFIG_LSTTOLOAD : CONFIG_LSTOTHERPROFILES), 
							LB_ADDSTRING, 0, (LPARAM)bot[i]->pname);
					}
			}
			hWnd_page[lParam] = hDialog;
			UpdateConfigPage(hDialog, index, lParam);
			SetWindowPos(hDialog, NULL, 125, 24, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			break;
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == hWnd_lblclr1) {
				return (long)ghBrushes[0];
			} else if ((HWND)lParam == hWnd_lblclr2) {
				return (long)ghBrushes[1];
			}
			break;
		case WM_COMMAND:
			index = 0;
			switch (LOWORD(wParam)) {
				case CONFIG_CMDRTBSELIMG:
					if (OpenFileDialog(asdf, MAX_PATH, "RichEdit Image"))
						WriteStuff(bot[index]->pname, "Interface", "RTBImg", asdf);
					break;
				case CONFIG_CMDLVSELIMG:
					if (OpenFileDialog(asdf, MAX_PATH, "ListView Image"))
						WriteStuff(bot[index]->pname, "Interface", "LVImg", asdf);
					break;
				case CONFIG_CMDSELECTHASHPATH:
					i2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_MISC], CONFIG_CBOCLIENTS2),
							CB_GETCURSEL, 0, 0);
					if (i2 != CB_ERR) {
						for (i = 1; i != 4; i++) {
							sprintf(str, "%sHash%d", clientstrs[i2], i);
							if (OpenFileDialog(asdf, MAX_PATH, str))
								WriteStuffConfig("Hashes", str, asdf);
						}			 
					}
					break;
				case CONFIG_CMDNEWPROFILE:
					GetWindowText(GetDlgItem(hWnd_page[CFGTAB_PROFILE], CONFIG_TXTNEWPROFILE),
						newprofile, sizeof(newprofile));
					if (*newprofile) {
						sprintf(asdf + 128, "profiles\\%s.ini", newprofile);
						if (activeprofile != -1) {
							sprintf(asdf, "profiles\\%s.ini", bot[activeprofile]->pname);
							CopyFile(asdf, asdf + 128, true);
						} else {
							HANDLE hFile;
							hFile = CreateFile(asdf + 128, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
							CloseHandle(hFile);
						}
						//TODO: refactor this
						//LoadProfileNames();
						SendMessage(GetDlgItem(hDialog, CONFIG_LSTOTHERPROFILES),
							LB_ADDSTRING, 0, (LPARAM)newprofile);
						hWndtmp = GetDlgItem(hWnd_Config, CONFIG_LSTPROFILES);
						SendMessage(hWndtmp, LB_ADDSTRING, 0, (LPARAM)newprofile);
						i = SendMessage(hWndtmp, LB_FINDSTRINGEXACT, -1, (LPARAM)newprofile);
						if (i != -1)
							SendMessage(hWndtmp, LB_SETCURSEL, i, 0);
					}
					break;
				case CONFIG_CMDDELETEPROFILE:
					sprintf(str, "profiles\\%s.ini", bot[activeprofile]->pname);
					sprintf(asdf, "Are you sure you'd like to delete %s??", str);
					if (MessageBox(0, asdf, "Sure?", MB_OKCANCEL | MB_ICONWARNING) == IDOK)	{
						DeleteFile(str);
						SendMessage(GetDlgItem(hWnd_Config, CONFIG_LSTPROFILES),
							LB_DELETESTRING, activeprofile, 0);
						hWndtmp = GetDlgItem(hDialog, bot[activeprofile]->ploadonstart ?
									CONFIG_LSTTOLOAD : CONFIG_LSTOTHERPROFILES);
						i = SendMessage(hWndtmp, LB_FINDSTRINGEXACT, -1,
								(LPARAM)bot[activeprofile]->pname);
						if (i != -1)
							SendMessage(hWndtmp, LB_DELETESTRING, i, 0);
						SendMessage(hWndtmp, LB_SETCURSEL, 0, 0);
						////////////////////////////////////SAME HERE!!!!!//////////
						//LoadProfileNames();
					}
					break;
				case CONFIG_CMDLOADPROFILE: 
					LoadProfile(activeprofile);
					break;
				case CONFIG_CMDUNLOADPROFILE:
					UnloadProfile(activeprofile);
					break;
				case CONFIG_CMDOPENPROFILEDIR:
					strcpy(asdf, CurrDir);
					strcat(asdf, "\\profiles");
					ShellExecute(NULL, NULL, asdf, NULL, "C:\\", SW_SHOWNORMAL);
					break;
				case CONFIG_CMDOPENPROFILEINI:
					sprintf(asdf, "%s\\profiles\\%s.ini", CurrDir, bot[activeprofile]->pname);
					ShellExecute(NULL, NULL, asdf, NULL, "C:\\", SW_SHOWNORMAL);
					break;
				case CONFIG_CMDTOLOADSTARTUP:
					hWndtmp = GetDlgItem(hDialog, CONFIG_LSTOTHERPROFILES);
					i = SendMessage(hWndtmp, LB_GETCURSEL, 0, 0);
					if (i != -1) {
						SendMessage(hWndtmp, LB_GETTEXT, i, (LPARAM)asdf);
						SendMessage(hWndtmp, LB_DELETESTRING, i, 0);
						SendMessage(GetDlgItem(hDialog, CONFIG_LSTTOLOAD), LB_ADDSTRING, 0, (LPARAM)asdf);
						WriteStuff(asdf, "Main", "LoadOnStartup", "1");
					}
					break;
				case CONFIG_CMDTONOTLOADSTARTUP:
					hWndtmp = GetDlgItem(hDialog, CONFIG_LSTTOLOAD);
					i = SendMessage(hWndtmp, LB_GETCURSEL, 0, 0);
					if (i != -1) {
						SendMessage(hWndtmp, LB_GETTEXT, i, (LPARAM)asdf);
						SendMessage(hWndtmp, LB_DELETESTRING, i, 0);
						SendMessage(GetDlgItem(hDialog, CONFIG_LSTOTHERPROFILES), LB_ADDSTRING, 0, (LPARAM)asdf);
						WriteStuff(asdf, "Main", "LoadOnStartup", "0"); 
					}
				case CONFIG_TXTNEWPROFILE:
					if (HIWORD(wParam) == EN_CHANGE)
						EnableWindow(GetDlgItem(hDialog, CONFIG_CMDNEWPROFILE),
							GetWindowTextLength(GetDlgItem(hDialog, CONFIG_TXTNEWPROFILE)));
					break;
				case CONFIG_CBOFONT:
					if (HIWORD(wParam) == CBN_SELCHANGE)
						UpdateSampleFont((HWND)lParam);
					break;
				case CONFIG_TXTFONTSIZE:
					if (HIWORD(wParam) == EN_KILLFOCUS)
						UpdateSampleFont((HWND)lParam);
					break;
				case CONFIG_LBLCLR1:
				case CONFIG_LBLCLR2:
					if (HIWORD(wParam) == STN_CLICKED) {
						index = LOWORD(wParam) - CONFIG_LBLCLR1;
						DeleteObject(ghBrushes[index]);
						int ret = OpenChooseColorDialog();
						if (ret != -1) {
							ghBrushes[index] = CreateSolidBrush(ret);	
							RedrawWindow((HWND)lParam, NULL, NULL, RDW_INVALIDATE);
						}
					}
			}
			ToggleEnableStates(hDialog, wParam);
			break;
	default:
		return DefWindowProc(hDialog, message, wParam, lParam);
	}
	return false;
}


void TabChange(HWND hDialog) {
	int i = SendMessage(GetDlgItem(hDialog, IDC_TAB1), TCM_GETCURSEL, 0, 0);
	if (i != -1) { 
		ShowWindow(hWnd_currpage, SW_HIDE);
		ShowWindow(hWnd_page[i], SW_SHOW);
		hWnd_currpage = hWnd_page[i];
	}
}


void UpdateConfigPage(HWND hDialog, int index, int page) {
	char asdf[64];
	int i;
	if (index == -1)
		return;
	//if (bots[index]->ploaded)
	//	effindex = GetBotFromProfile(index);
	switch (page) {
		case 0:	//connection
			/*if (profiles[index]->loaded) {
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTUSER), bot[effindex]->username);
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTPASS), bot[effindex]->password);
				SetWindowText(GetDlgItem(hDialog, CONFIG_CBOSERVER), bot[effindex]->server);
				SetWindowText(GetDlgItem(hDialog, CONFIG_CBOBNLSSERVER), bot[effindex]->bnlsserver);
				SetWindowText(GetDlgItem(hDialog, CONFIG_CBOCLIENT), bot[effindex]->clientstr);
				SetWindowText(GetDlgItem(hDialog, CONFIG_CBOCDKEY), bot[effindex]->cdkey);
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTEXPKEY), bot[effindex]->expkey);
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTHOME), bot[effindex]->homechannel);
				SendMessage(GetDlgItem(hDialog, CONFIG_CHKPLUG), BM_SETCHECK, (bot[effindex]->fstate & BFS_USEPLUG) ? BST_CHECKED : BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDialog, CONFIG_CRLOCALHASHING + bot[effindex]->crtype), BM_SETCHECK, BST_CHECKED, 0);
				SendMessage(GetDlgItem(hDialog, CONFIG_PINGNEG1 + bot[effindex]->pingtype), BM_SETCHECK, BST_CHECKED, 0);
				SetDlgItemInt(hDialog, CONFIG_TXTPINGCUSTOMVAL, bot[effindex]->spoofedping, true);
			} else { */
			for (i = 0; !txtdescriptors[i].page; i++) {
				SetWindowText(GetDlgItem(hDialog, txtdescriptors[i].res), 
				GetStuff(bot[index]->pname, txtdescriptors[i].section, txtdescriptors[i].key, sizeof(asdf), asdf));
			}
			GetStuff(bot[index]->pname, "Main", "UsePlug", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CHKPLUG), BM_SETCHECK, *asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);
			GetStuff(bot[index]->pname, "Main", "CRType", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CRLOCALHASHING + *asdf - 0x30), BM_SETCHECK, BST_CHECKED, 0);
			GetStuff(bot[index]->pname, "Main", "PingType", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_PINGNEG1 + *asdf - 0x30), BM_SETCHECK, BST_CHECKED, 0);
			//}
			break;
		case 1:	//advanced connection
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTCOUNTRYNAME), countryname);
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTCOUNTRYABBRIV), countryabbriv); 
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTVERBYTESTAR), GetStuffConfig("Verbytes", "STAR", asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTVERBYTEWAR3), GetStuffConfig("Verbytes", "WAR3", asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTVERBYTED2DV), GetStuffConfig("Verbytes", "D2DV", asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTVERBYTEW2BN), GetStuffConfig("Verbytes", "W2BN", asdf));
			/*if (profiles[index]->loaded) {
				*(int *)asdf = bot[effindex]->platform;
				fastswap32((unsigned long *)asdf);
				asdf[4] = 0;
				SetWindowText(GetDlgItem(hDialog, CONFIG_CBOPROXY), bot[effindex]->proxy);
				SetWindowText(GetDlgItem(hDialog, CONFIG_CBOPLATFORM), asdf);
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTCDKEYOWNER), bot[effindex]->cdkeyowner);
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTNEWPASS), bot[effindex]->newpass);
				SetWindowText(GetDlgItem(hDialog, CONFIG_CBOPROXY), bot[effindex]->proxy);
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTEMAIL1), bot[effindex]->email1);
				SetWindowText(GetDlgItem(hDialog, CONFIG_TXTEMAIL2), bot[effindex]->email2);
				SendMessage(GetDlgItem(hDialog, CONFIG_CHKFORCECREATE), BM_SETCHECK, (bot[effindex]->fstate & BFS_FORCECREATE) ? BST_CHECKED : BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDialog, CONFIG_CHKAUTOREG), BM_SETCHECK, (bot[effindex]->fstate & BFS_AUTOREGEMAIL) ? BST_CHECKED : BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDialog, CONFIG_CHKREGBACK), BM_SETCHECK, (bot[effindex]->fstate & BFS_CHANGEEMAIL) ? BST_CHECKED : BST_UNCHECKED, 0);
				SendMessage(GetDlgItem(hDialog, CONFIG_CHKPROXY), BM_SETCHECK, (bot[effindex]->fstate & BFS_USEPROXY) ? BST_CHECKED : BST_UNCHECKED, 0);	
			} else {	*/
			for (i = 9; txtdescriptors[i].page == 1; i++) {
				SetWindowText(GetDlgItem(hDialog, txtdescriptors[i].res), 
				GetStuff(bot[index]->pname, txtdescriptors[i].section, txtdescriptors[i].key, sizeof(asdf), asdf));
			}
			GetStuff(bot[index]->pname, "Main", "ForceCreate", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CHKFORCECREATE), BM_SETCHECK, *asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);
			GetStuff(bot[index]->pname, "Main", "AutoRegEmail", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CHKAUTOREG), BM_SETCHECK, *asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);
			GetStuff(bot[index]->pname, "Main", "ChangeEmail", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CHKREGBACK), BM_SETCHECK, *asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);
			GetStuff(bot[index]->pname, "Main", "UseProxy", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CHKPROXY), BM_SETCHECK, *asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);
			//}
			break;
		case 2: //interface
			SetWindowText(GetDlgItem(hDialog, CONFIG_CBOFONT), GetStuffConfig("Font", "FaceName", asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTFONTSIZE), GetStuffConfig("Font", "Size", asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTMAXRTBLEN), GetStuffConfig("Interface", "MaxRTBLen", asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTCHARSTOCLEAR), GetStuffConfig("Interface", "CharsToClear", asdf));
			for (i = 0; i != NUM_PAGE2CHKS; i++) {
				if (f2confchks[i].globalsetting)
					GetStuffConfig("Interface", f2confchks[i].key, asdf);
				else
					GetStuff(bot[index]->pname, "Interface", f2confchks[i].key, sizeof(asdf), asdf);
				SendMessage(GetDlgItem(hWnd_page[CFGTAB_INTERFACE], f2confchks[i].res), BM_SETCHECK,
					*asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);
			}
			
			SendMessage(GetDlgItem(hDialog, CONFIG_CBOTABPOS), CB_SETCURSEL, tabpos, 0);
			GetStuffConfig("Interface", "BrushHatchType", asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_BRSHHATCH0 + (*asdf - 0x30)), BM_SETCHECK, BST_CHECKED, 0);

			GetStuffConfig("Interface", "PsuedotransparencyAlpha", asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_SLDGLASS), TBM_SETPOS, true, atoi(asdf));
			break;
		case 3: //messaging
			GetStuff(bot[index]->pname, "Main", "UseJoinGreet", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CHKUSEJOINGREET), BM_SETCHECK,
				*asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);
			GetStuff(bot[index]->pname, "Main", "UseLeaveGreet", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_CHKUSELEAVEGREET), BM_SETCHECK,
				*asdf - 0x30 ? BST_CHECKED : BST_UNCHECKED, 0);

			GetStuff(bot[index]->pname, "GnI", "IdleType", sizeof(asdf), asdf);
			SendMessage(GetDlgItem(hDialog, CONFIG_OPTIDLENONE + *asdf - 0x30), BM_SETCHECK, BST_CHECKED, 0);
			
			
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTJOINGREET), GetStuff(bot[index]->pname, "GnI", "JoinGreet", sizeof(asdf), asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTLEAVEGREET), GetStuff(bot[index]->pname, "GnI", "LeaveGreet", sizeof(asdf), asdf));	
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTIDLE), GetStuff(bot[index]->pname, "GnI", "IdleMsg", sizeof(asdf), asdf));
			SetWindowText(GetDlgItem(hDialog, CONFIG_TXTIDLEINTERVAL), GetStuff(bot[index]->pname, "GnI", "IdleInterval", sizeof(asdf), asdf));
			break;
		case 4: //misc
			for (i = 0; i != NUM_PAGE4CHKS; i++) {
				GetStuffConfig("Misc", f4confchks[i].key, asdf);
				SendMessage(GetDlgItem(hWnd_page[CFGTAB_MISC], f4confchks[i].res), BM_SETCHECK,
					(*asdf - 0x30) ? BST_CHECKED : BST_UNCHECKED, 0);
			}
			SetDlgItemInt(hDialog, CONFIG_TXTPERPACKET, perpacket, true);
			SetDlgItemInt(hDialog, CONFIG_TXTPERBYTE, perbyte, true);
			SetDlgItemInt(hDialog, CONFIG_TXTNUMFILTER, numfilter, true);
			SetDlgItemInt(hDialog, CONFIG_TXTRCRATE, rcrate, true);
			SetDlgItemInt(hDialog, CONFIG_TXTXRCHANNEL, xrchannel, true);
			SetDlgItemInt(hDialog, CONFIG_TXTXRFACTOR, xrfactor, true);


	}
}


void SaveSettings(HWND hDialog, int index) {
	char tmp[256];
	int tmp2, i;

	////////////txtboxes
	for (i = 0; i != NUM_TXTS; i++) {
		HWND tmphwnd = GetDlgItem(hWnd_page[txtdescriptors[i].page], txtdescriptors[i].res);
		GetWindowText(tmphwnd, tmp, sizeof(tmp));
		if (i >= NUM_TXTNONGLOBALS)
			WriteStuffConfig(txtdescriptors[i].section, txtdescriptors[i].key, tmp);
		else
			WriteStuff(bot[index]->pname, txtdescriptors[i].section, txtdescriptors[i].key, tmp);
	}

	///////////generic chks
	for (i = 0; i != NUM_PAGE2CHKS; i++) {
		tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_INTERFACE], f2confchks[i].res), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
		if (f2confchks[i].globalsetting) {
			WriteStuffConfig("Interface", f2confchks[i].key, (char *)&tmp2);
		} else {
			WriteStuff(bot[index]->pname, "Interface", f2confchks[i].key, (char *)&tmp2);
		}
	}
	for (i = 0; i != NUM_PAGE4CHKS; i++) {
		tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_MISC], f4confchks[i].res), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
		WriteStuffConfig("Misc", f4confchks[i].key, (char *)&tmp2);
	}

	//////////pg 1 chks
	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_CONNECTION], CONFIG_CHKPLUG), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "UsePlug", (char *)&tmp2);
	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_MESSAGING], CONFIG_CHKUSEJOINGREET), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "UseJoinGreet", (char *)&tmp2);
	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_MESSAGING], CONFIG_CHKUSELEAVEGREET), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "UseLeaveGreet", (char *)&tmp2);


	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_ADVCONNECT], CONFIG_CHKFORCECREATE), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "ForceCreate", (char *)&tmp2);
	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_ADVCONNECT], CONFIG_CHKAUTOREG), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "AutoRegEmail", (char *)&tmp2);
	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_ADVCONNECT], CONFIG_CHKREGBACK), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "ChangeEmail", (char *)&tmp2);
	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_ADVCONNECT], CONFIG_CHKCHANGEPASS), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "ChangePass", (char *)&tmp2);
	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_ADVCONNECT], CONFIG_CHKPROXY), BM_GETCHECK, 0, 0) == BST_CHECKED ? 0x31 : 0x30;
	WriteStuff(bot[index]->pname, "Main", "UseProxy", (char *)&tmp2);

	//////////opts
	for (i = 0; i != 4; i++) {
		if (SendMessage(GetDlgItem(hWnd_page[CFGTAB_CONNECTION], CONFIG_CRLOCALHASHING + i), BM_GETCHECK, 0, 0) == BST_CHECKED)
			*(short *)tmp = 0x30 + i;
	}
	WriteStuff(bot[index]->pname, "Main", "CRType", tmp);
	for (i = 0; i != 4; i++) {
		if (SendMessage(GetDlgItem(hWnd_page[CFGTAB_CONNECTION], CONFIG_PINGNEG1 + i), BM_GETCHECK, 0, 0) == BST_CHECKED)
			*(short *)tmp = 0x30 + i;
	}
	WriteStuff(bot[index]->pname, "Main", "PingType", tmp);
	for (i = 0; i != 7; i++) {
		if (SendMessage(GetDlgItem(hWnd_page[CFGTAB_INTERFACE], CONFIG_BRSHHATCH0 + i), BM_GETCHECK, 0, 0) == BST_CHECKED)
			*(short *)tmp = 0x30 + i;
	}
	WriteStuffConfig("Interface", "BrushHatchType", tmp);
	for (i = 0; i != 4; i++) {
		if (SendMessage(GetDlgItem(hWnd_page[CFGTAB_MESSAGING], CONFIG_OPTIDLENONE + i), BM_GETCHECK, 0, 0) == BST_CHECKED)
			*(short *)tmp = 0x30 + i;
	}
	WriteStuff(bot[index]->pname, "GnI", "IdleType", tmp);

	LOGBRUSH lb;
	GetObject(ghBrushes[0], sizeof(LOGBRUSH), &lb);
	sprintf(tmp, "0x%08x", lb.lbColor);
	WriteStuffConfig("Interface", "GradientClr1", tmp);

	GetObject(ghBrushes[1], sizeof(LOGBRUSH), &lb); 
	sprintf(tmp, "0x%08x", lb.lbColor);
	WriteStuffConfig("Interface", "GradientClr2", tmp);

	sprintf(tmp, "%d", SendMessage(GetDlgItem(hWnd_page[CFGTAB_INTERFACE], CONFIG_SLDGLASS), TBM_GETPOS, 0, 0));
	WriteStuffConfig("Interface", "PsuedotransparencyAlpha", tmp);

	tmp2 = SendMessage(GetDlgItem(hWnd_page[CFGTAB_INTERFACE], CONFIG_CBOTABPOS), CB_GETCURSEL, 0, 0) + 0x30;
	WriteStuffConfig("Interface", "TabPos", (char *)&tmp2);
}


int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam) {
	static char prevface[LF_FACESIZE];
	if (strcmp(prevface, (const char *)lpelfe->elfFullName)) {
		strcpy(prevface, (const char *)lpelfe->elfFullName);
		SendMessage(GetDlgItem((HWND)lParam, CONFIG_CBOFONT), CB_ADDSTRING, 0, (LPARAM)lpelfe->elfFullName);	
	}
	return 1;
}


int OpenFileDialog(char *buf, int len, char *title) {
	char buf2[MAX_PATH] = {0x20};
	OPENFILENAME openfilename;
	ZeroMemory(&openfilename, sizeof(OPENFILENAME));
	openfilename.lStructSize = sizeof(OPENFILENAME);
	openfilename.lpstrTitle  = title;
	openfilename.hwndOwner	 = hWnd_main; 			 
	openfilename.hInstance	 = g_hInst; 				 
	openfilename.lpstrFilter = (char *)"All Files\0*.*\0\0"; 
	openfilename.lpstrFile	 = buf2;
	openfilename.nMaxFile 	 = sizeof(buf2);
	int ret = GetOpenFileName(&openfilename);
	strncpy(buf, buf2, len);
	return ret;
}


int OpenChooseColorDialog() {
	CHOOSECOLOR cc;
	static COLORREF custcolors[16];
	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	cc.lStructSize    = sizeof(CHOOSECOLOR);
	cc.hwndOwner      = hWnd_main;
	cc.Flags          = CC_FULLOPEN;
	cc.lpTemplateName = "hi";
	cc.lpCustColors   = custcolors;
	if (!ChooseColor(&cc))
		return -1;
	return cc.rgbResult;
}


void InitalizeWindowEnabledStates(HWND hDialog) {
	int i;
	ToggleEnableStates(hWnd_page[1], CONFIG_CHKPROXY);
	ToggleEnableStates(hWnd_page[1], CONFIG_CHKCHANGEPASS);
	ToggleEnableStates(hWnd_page[3], CONFIG_CHKUSEJOINGREET);
	ToggleEnableStates(hWnd_page[3], CONFIG_CHKUSELEAVEGREET);
	ToggleEnableStates(hWnd_page[4], CONFIG_CHKFILTERNUMS);
	ToggleEnableStates(hWnd_page[2], CONFIG_CHKUSELVIMG);
	ToggleEnableStates(hWnd_page[2], CONFIG_CHKUSERTBIMG);
	ToggleEnableStates(hWnd_page[2], CONFIG_CHKCLEARRTB);
	ToggleEnableStates(hWnd_page[4], CONFIG_CHKUSEANTIFLOOD);
	ToggleEnableStates(hWnd_page[4], CONFIG_CHKUSEFLOODPROTECT);
	for (i = CONFIG_PINGNEG1; i != CONFIG_PINGNEG1 + 4; i++) {
		if (SendMessage(GetDlgItem(hWnd_page[0], i), BM_GETCHECK, 0, 0) == BST_CHECKED)
			ToggleEnableStates(hWnd_page[0], i);
	}
	for (i = CONFIG_OPTIDLENONE; i != CONFIG_OPTIDLENONE + 4; i++) {
		if (SendMessage(GetDlgItem(hWnd_page[3], i), BM_GETCHECK, 0, 0) == BST_CHECKED)
			ToggleEnableStates(hWnd_page[3], i);	
	}
}


void ToggleEnableStates(HWND hDialog, int control) {
	bool res;
	switch (control) {
		case CONFIG_CHKPROXY:
			EnableWindow(GetDlgItem(hDialog, CONFIG_CBOPROXY), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKPROXY) == BST_CHECKED);
			break;
		case CONFIG_CHKCHANGEPASS:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTNEWPASS), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKCHANGEPASS) == BST_CHECKED);
			break;
		case CONFIG_CHKUSEJOINGREET:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTJOINGREET), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKUSEJOINGREET) == BST_CHECKED);
			break;
		case CONFIG_CHKUSELEAVEGREET:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTLEAVEGREET), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKUSELEAVEGREET) == BST_CHECKED);
			break;
		case CONFIG_CHKFILTERNUMS:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTNUMFILTER), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKFILTERNUMS) == BST_CHECKED);
			break;
		case CONFIG_CHKUSELVIMG:
			EnableWindow(GetDlgItem(hDialog, CONFIG_CMDLVSELIMG), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKUSELVIMG) == BST_CHECKED);
			break;
		case CONFIG_CHKUSERTBIMG:
			EnableWindow(GetDlgItem(hDialog, CONFIG_CMDRTBSELIMG), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKUSERTBIMG) == BST_CHECKED);
			break;
		case CONFIG_CHKCLEARRTB:
			res = (IsDlgButtonChecked(hDialog, CONFIG_CHKCLEARRTB) == BST_CHECKED);
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTMAXRTBLEN), res); 
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTCHARSTOCLEAR), res); 
			break;
		case CONFIG_CHKUSEANTIFLOOD:
			res = (IsDlgButtonChecked(hDialog, CONFIG_CHKUSEANTIFLOOD) == BST_CHECKED);
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTPERPACKET), res); 
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTPERBYTE), res); 
			break;
		case CONFIG_CHKUSEFLOODPROTECT:
			EnableWindow(GetDlgItem(hDialog, CONFIG_CHKFILTERNUMS), 
				IsDlgButtonChecked(hDialog, CONFIG_CHKUSEFLOODPROTECT) == BST_CHECKED);
			break;

		//ping opts
		case CONFIG_PINGNEG1:
		case CONFIG_PING0:
		case CONFIG_PINGNORMAL:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTPINGCUSTOMVAL), false);
			break;
		case CONFIG_PINGCUSTOM:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTPINGCUSTOMVAL), true);
			break;

		//idle opts
		case CONFIG_OPTIDLEMSG:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTIDLE), true);
		case CONFIG_OPTIDLEMP3:
		case CONFIG_OPTIDLEUPTIME:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTIDLEINTERVAL), true);
			if (control != CONFIG_OPTIDLEMSG)
				EnableWindow(GetDlgItem(hDialog, CONFIG_TXTIDLE), false);
			break;
		case CONFIG_OPTIDLENONE:
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTIDLEINTERVAL), false);
			EnableWindow(GetDlgItem(hDialog, CONFIG_TXTIDLE), false);
			break;
	}
}


void UpdateSampleFont(HWND hWnd_cbo) {
	HDC hdc = GetDC(0);
	int height = -MulDiv(GetDlgItemInt(hWnd_page[2], CONFIG_TXTFONTSIZE, NULL, true), LogPixelsY, 72);
	char blah[64];
	if (hFontTmp)
		DeleteObject(hFontTmp);
	SendMessage(hWnd_cbo, CB_GETLBTEXT, SendMessage(hWnd_cbo, CB_GETCURSEL, 0, 0), (LPARAM)blah);
	hFontTmp = CreateFont(height, 0, 0, 0, FW_REGULAR, false, false, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, blah);
	SendMessage(GetDlgItem(hWnd_page[2], CONFIG_LBLFONT), WM_SETFONT, (WPARAM)hFontTmp, true);
	ReleaseDC(0, hdc);
}

