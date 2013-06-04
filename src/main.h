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

#define szAppName "Horizon"
#define HORIZON_VERSION "0.8.6 Alpha (Build 860)"
#define HORIZON_ABOUTSTR  "Horizon " HORIZON_VERSION "\r\nA Win32 Battle.net binary chat bot\r\nBy BreW"

#define vbBlack  0x00000000
#define vbRed    0x000000FF
#define vbGreen  0x0000FF00
#define vbBlue   0x00FF0000
#define vbYellow 0x0000FFFF
#define vbCyan   0x00FFFF00
#define vbPurple 0x00FF00FF
#define vbWhite  0x00FFFFFF
#define vbLtGray 0x00AAAAAA
#define vbGray   0x00888888
#define vbDkGray 0x00666666  
#define vbOrange 0x000080FF
#define vbRGreen 0x0064FA32
#define vbRBlue  0x00FFBB50
#define vbRCream 0x00BED7DC
#define vbRDBlue 0x00C08000
#define vbDGreen 0x0082AF0A

//local flags
#define BFS_USEPLUG	       0x01
#define BFS_FORCECREATE    0x02
#define BFS_USEJOINGREET   0x04
#define BFS_USELEAVEGREET  0x08
#define BFS_AUTOREGEMAIL   0x10
#define BFS_CHANGEPASS	   0x20
#define BFS_CHANGEEMAIL	   0x40
#define BFS_USEPROXY       0x80
#define BFS_USELEGACY	   0x100
#define BFS_USESRP		   0x200
#define BFS_STARCRAFT      0x400
#define BFS_DIABLO2        0x800
#define BFS_WARCRAFT3      0x1000
#define BFS_REQACCINFO     0x2000
#define BFS_REQCLANINFO	   0x4000
#define BFS_REQFLIST       0x8000
#define BFS_REQCLANCREATE  0x10000
#define BFS_CREATEINVITE   0x20000
#define BFS_FIRSTDATARX	   0x40000

#define BFS_STATICATTRIBUTEMASK 0xFF

//global flags
#define GFS_UIGRADIENT     0x01
#define GFS_UIMERGELVRTB   0x02
#define GFS_UIWARNEXITAPP  0x04
#define GFS_UIWARNEXITPROF 0x08
#define GFS_UITEXTPING     0x10
#define GFS_UIATMOSPHERE   0x20
#define GFS_USEGLASS	   0x40
#define GFS_USEANTIFLOOD   0x80
#define GFS_USEFLOODDEF    0x100
#define GFS_USENUMFILTER   0x200
#define GFS_SHOWACCINFO    0x400
#define GFS_JOINPUBCHAN    0x800
#define GFS_LOGCHAT        0x1000
#define GFS_RTFLOG         0x2000
#define GFS_SHOWVOIDUL     0x4000
#define GFS_URLDETECT      0x8000
#define GFS_DECODE		   0x10000
#define GFS_GETICONS	   0x20000

//cr types
#define CRTYPE_LOCAL    0
#define CRTYPE_EXTERNAL 1
#define CRTYPE_BNLS     2
#define CRTYPE_NONE     3

//idle types
#define IDLETYPE_NONE	 0
#define IDLETYPE_MP3	 1
#define IDLETYPE_UPTIME	 2
#define IDLETYPE_MESSAGE 3

//ping types
#define PINGTYPE_NEG1 0
#define	PINGTYPE_ZERO 1
#define PINGTYPE_NORM 2
#define PINGTYPE_CUST 3

//statusbar sections
#define SBPART_USERNAME 0
#define SBPART_CHANNEL  1
#define SBPART_PING     2
#define SBPART_FLAGS    3
#define SBPART_CLIENT   4

//tab positions
#define TABPOS_TOP	  0
#define TABPOS_BOTTOM 1
#define TABPOS_LEFT   2
#define TABPOS_RIGHT  3

//channel list tabs
#define LVTAB_CHANNEL 0
#define LVTAB_FRIENDS 1
#define LVTAB_CLAN    2 

//AddChatEx flags
#define AC_ATTR_BOLD	   0x80000000
#define AC_ATTR_ITALIC	   0x40000000
#define AC_ATTR_UNDERLINE  0x20000000
#define AC_ATTR_STRIKETHRU 0x10000000

//text modifier flags
#define TM_ALTCAPS		0x01
#define TM_LEETSPEAK	0x02
#define TM_HEX			0x04
#define TM_XR			0x08
#define TM_CONVERTTABS	0x10

//size of hashtables - 1
#define TABLESIZE_USERS 0x3FF
#define TABLESIZE_CHANNELS 0x3F

//misc.
#define NUM_SBPARTS 5
#define NUM_LOADED_MODULES 6

#define WM_WSDATAARRIVAL (WM_USER + 1)
#define WM_SHELLNOTIFY (WM_USER + 5000)
#define SWAP(a,b) ((a) == (b)) ? ((a) = (a)) : ((a) ^= (b) ^= (a) ^= (b))
#define UserOp(x) ((x) & 0x0F)

typedef void *BigBuffer;

typedef struct {
	int current_position;
	char random_data[0x14];
	char random_source_1[0x14];
	char random_source_2[0x14];
} t_random_data;

typedef struct _chain {
	int numentries;
	void *entry[1];
} CHAIN, *LPCHAIN;

typedef struct _LVCOLOR {
	unsigned long cItem1;
	unsigned long cItem2;
	unsigned long cItem3;
	bool bBold;
} LVCOLOR, *LPLVCOLOR;

typedef struct _PROFILEINIT {
	int index;
	const char *username;
} PROFILEINIT, *LPPROFILEINIT;

typedef struct {
    unsigned int count[2];
    unsigned int state[5];
    unsigned char buffer[264];
} SHA1_CTX;

typedef struct _greetargs {
	char *user;
	int flags;
	int ping;
} GREETARGS, *LPGREETARGS;

typedef struct _clanjoinreq {
	unsigned __int32 tag;
	unsigned __int32 cookie;
	char inviter[32];
} CLANJOINREQ, *LPCLANJOINREQ;

typedef struct _user {
	char username[32];	   
	unsigned long ping;	   
	unsigned long flags;   
	unsigned long speak;   
	unsigned long client;  
	unsigned long jointick;	
	unsigned long lastspoke;  
	unsigned long clan;		  
	unsigned long chanpos;	 
	//bool haschar;			   
	char charname[32];		
	void *nextlvuser;			
	void *lastlvuser;
	
	LPLVCOLOR clrstruct;
} USER, *LPUSER;

typedef struct _channel {
	char channelname[64];
	char rtbPic[128];
	char lvwPic[128];
	char lblPic[128];
} CHANNEL, *LPCHANNEL;

typedef struct _checkrevisionparams {
	char *file1;		   //in	 | e 0
	char *file2;		   //in	 | s 4
	char *file3;		   //in	 | b 8
	char *videobuf;        //in	 | d 12
	char *mpqName;		   //in	 | m 16
	int mpqNumber;         //in	 | n 20
	char *ChecksumFormula; //in	 | f 24
	char *exeInfo;         //out | i 28
	int *lpexeVersion;     //out | v 32
	int *lpChecksum;       //out | c 36
	int null;              //in  | u 40
} CHECKREVISIONPARAMS, *LPCHECKREVISIONPARAMS;
	 
//typedef struct _profiles {
//	int hash;
//	char name[64];
//	bool loaded;
//	bool defload;
//} PROFILES, *LPPROFILES;

typedef struct _profile {
	char pname[32];
	bool ploaded;
	bool ploadonstart;
	//int pnamehash;
	HWND hWnd_main;
	HWND hWnd_rtfChat;
	//HWND hWnd_cboChat;
	HWND hWnd_lvwChannel;
	HWND hWnd_lblChannel;
	HWND hWnd_Statusbar;
	HWND hWnd_txtChat;
	HWND hWnd_tab;
	//HWND hWnd_cbotxtChat;
	SOCKET sck;		
	char username[32];		
	char password[32];
	char server[32];
	char bnlsserver[64];
	char cdkey[32];
	char expkey[32];
	char cdkeyowner[32];
	char homechannel[32];
	char proxy[32];
	char newpass[64];
	char email1[64];
	char email2[64];
	char realname[64];
	char currentchannel[64];
	char clientstr[8];
	t_random_data warden_data;
	unsigned char wardenkey_out[258];
	unsigned char wardenkey_in[258];
	unsigned long client;
	unsigned long platform;
	unsigned long verbyte;
	
	LPCLANJOINREQ invite;

	char crtype;   /////changed these to char from ints to save room
	char pingtype;
	char idletype;
	char spoofedping;

	char floodamt;
	char cdkeylen;
	unsigned short idleints;

	bool connected;
	unsigned long fstate;
	unsigned long ClientToken;
	unsigned long ServerToken;
	int bnetflags;
	int ping;
	int numusers;
	LPCHAIN *users;

	HBITMAP rtbPic;
	HBITMAP lvwPic;
	HBITMAP lblPic;

	int lvwidth;
	int txtheight;

	bool flooded;
	unsigned long floodtime;
	unsigned long lastjointime;
	LPUSER lastpersonjoined;
	
	int floodcount;
	int prefloodc;

	int sendcount;
	int recvcount;

	char **queue;
	int queuecount;
	int curqueuepos;
	unsigned int lastsendtick;
	int lastsendlen;

	char greetmsg[256];
	char leavemsg[256];
	char idlemsg[256];
	int idleinterval;

	int bancount;
	int joincount;
	int talkcount;

	unsigned int loadedtick;
	unsigned int connectedtick;
	
	int nls;
	int oldnls;

	int clan;
	char clanstr[8];

	char lastwhispered[64];
	char *lastmsgs[20];
	int curmsgpos;
	int	curmsgscrollpos;
	bool scrllock;

	LPUSER lastadded;
	LPUSER firstlvuser;
	LPUSER self;
	unsigned long tmptick;

	int curlvtab;
	int tabindex;

	int txtsendattrib;

	int recvlen;				
	char wsbuffer[2048];
	char *wsdata;
} BOT, *LPBOT;

extern LPBOT *bot;
//extern LPPROFILES *profiles;
extern int numbots, numprofiles, rcrate;
extern int actprof, curnewprof, tabpos;
extern int xrchannel, xrfactor;
extern char CurrDir[MAX_PATH];
extern HWND hWnd_main, hWnd_Tab, hWnd_Config, hWnd_Profile, hWnd_Client, hWnd_Bots;
extern HWND snaptopwindow, hWnd_Tooltip, hWnd_About, hWnd_mdiactive, hWnd_status, hWnd_status_re;
extern HWND hWnd_ClanCreate;
extern HICON g_hIcon;
extern HIMAGELIST lvwIML;
extern HFONT hFont, hFontBig, hFontBold;
//extern HPEN hPen;
extern HBRUSH hBrushclr;
extern CHARFORMAT cfFormat;

extern char countryabbriv[16], countryname[32];
extern int perpacket, perbyte, numfilter;
extern unsigned long gfstate;
extern char *lpModuleImage[NUM_LOADED_MODULES];

extern int loglevel;

extern HINSTANCE g_hInst;
extern LPCHAIN channels[64];
extern HMENU hMenu_main, hMenu_main_popup, hMenu_tab_popup, hMenu_windows_popup, 
	hMenu_open_popup, hMenu_rtf, hMenu_lvw, hMenu_client, hMenu_lvwColumns,
	hMenu_lvwBots, hMenu_txtExt, hMenu_lvwClan;
extern NOTIFYICONDATA note;
extern WNDPROC lpfnOldClientProc, lpfnMDIctllvw, lpfnMDIctlrtb, lpfnMDIctlcbo,
 lpfnMDIctllbl, lpfnMDIctlstb, lpfnMDIctltxt;

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIChildProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIClientProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIrtbProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam); 
LRESULT CALLBACK MDIcboProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam); 
LRESULT CALLBACK MDItxtProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam); 
LRESULT CALLBACK MDIlblProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam); 
LRESULT CALLBACK MDIlvwProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MDIstbProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK QuickSwitchProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AboutProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

void CreateStatWindow();

void AddChat(COLORREF Color, char *Text, HWND hwnd);
void AppendText(COLORREF Color, char *szFmt, HWND hwnd);
void AddChatf(char *buf, HWND hwnd, COLORREF clr, char *text, ...);
void ChopRTBText(HWND hwnd);
void AddLog(char *text, int index);
void Send0x0E(char *text, int index);
void LoadGlobalSettings();
void LoadGlobalFlags();
void SetGlobalAttributes();
void LoadFavorites();
void HandleReturnKey(HWND hwnd, int index);
void OutputStartupStatusMessages();
void HorizonInitialize(HWND hwnd);
int HorizonShutdown();
void MoveMainWindowControls(LPARAM lParam);

void SetDebugFlags();
int except_addlog(int exceptcode, LPEXCEPTION_POINTERS exceptinfo);
long __stdcall ExceptionHandler(struct _EXCEPTION_POINTERS *pExceptInfo);

