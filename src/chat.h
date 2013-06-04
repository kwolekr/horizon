#define CMD_VER		     0xd25d59b4
#define CMD_VERSION      0x68c27e33
#define CMD_CQ		     0x11cc33f1
#define CMD_PROFILE      0x18702740
#define CMD_FORCEJOIN    0x115bc6d0
#define CMD_FJOIN        0x46c911dd
#define CMD_LEAVECHAT    0xd9120159
#define CMD_RJ		     0x09dd0f33
#define CMD_REJOIN       0xc1430ce0
#define CMD_UPTIME       0xccd130cc
#define CMD_HOME	     0x2245b5ae
#define CMD_SPOKE	     0xd9d01474
#define CMD_ACCINFO      0xf09abb8a
#define CMD_ACCEPT       0x5dafdc72
#define CMD_CRASH	     0xb88dbb3f
#define CMD_QUERY		 0x81827973
#define CMD_SETCWD		 0x31ef993d
#define CMD_ACTIVEPING   0x2aa070f0
#define CMD_SETNAME	     0xbf618956
#define CMD_SETPASS	     0xd6f5c294
#define CMD_SETHOME	     0x75ffec33
#define CMD_CONNECT      0x00d819ef
#define CMD_RC		     0x85db0731
#define CMD_RECONNECT    0x6e1f5b96
#define CMD_DISCONNECT   0x9da9fec5
#define CMD_PLAY         0x3999aff5
#define CMD_STOP         0x930ca7f2
#define CMD_MP3		     0x4f5e1150
#define CMD_PAUSE        0x9d8cddc8
#define CMD_VOL		     0xfd6483b1
#define CMD_SETVOLUME    0x54adf58e
#define CMD_NEXT         0x4bcd2941
#define CMD_PREV         0x9e3351f2
#define CMD_WINAMP       0x38749102
#define CMD_DISPLIST	 0xf5e3cd66
#define CMD_JOINGAME	 0x374e6725
#define CMD_LEAVEGAME	 0x8a832478
#define CMD_STARTGAME	 0x72bffcd2
#define CMD_XRENCRYPT	 0x9097a9a9
#define CMD_XRDECRYPT	 0xf064ea07							
#define CMD_CLANRANK	 0x9643c3f1										 
#define CMD_CRANK		 0xf71d9e82
#define CMD_CHIEFTAIN	 0xc2182016
#define CMD_GETMOTD		 0x29ce1b9b
#define CMD_CREMOVE		 0x83f7a21b
#define CMD_CMI			 0xad266c5c
#define CMD_OP			 0x1e9f3fdf
#define CMD_DISBAND		 0x0d459d4c
#define CMD_CHECKCLAN	 0xbb04ac63
#define CMD_CLEARCHAT	 0x13d48e0f
#define CMD_SLAP		 0x23075ba0
#define CMD_PACKETLOG    0xe2086721

void Parse0x0F(char *data, int index);
void EID_JOIN(char *user, char *text, unsigned long flags, unsigned long ping, bool joined, int index);	   
void EID_LEAVE(char *user, int flags, int ping, int index);
void EID_WHISPERFROM(char *user, char *text, unsigned long flags, int index);
void EID_TALK(char *user, char *text, unsigned long flags, int index);
void EID_CHANNEL(char *text, unsigned long flags, unsigned long ping, int index);
void EID_FLAGSUPDATE(char *user, char *text, unsigned long flags, unsigned long ping, int index);
void EID_WHISPERTO(char *user, char *text, unsigned long flags, int index);
void EID_EMOTE(char *user, char *text, unsigned long flags, int index);
void EID_INFO(char *data, char *text, int index);
//void CALLBACK ChatIdleTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void CALLBACK ChatIdleTimerProc(int idEvent);
void CALLBACK ChatOtherTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
bool ParseCommand(char *command, int index);

void UptimeCmdHandle(char *buf, char uptimetype, int index);
void SendGreet(char *user, int flags, int ping, bool leave, int index);

