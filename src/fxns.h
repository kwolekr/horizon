#define ShellEx(x) ShellExecute(NULL, NULL, x, NULL, NULL, SW_SHOW)
#define WM_WAKEUP (WM_USER + 4358)
#define NUM_FILESTOCHECK 11

#define QUERYVAL_XR		   0xf208d8b7
#define QUERYVAL_SCK       0x29152680
#define QUERYVAL_HWND	   0x5a4cc55a
#define QUERYVAL_STATE	   0x77a848c7
#define QUERYVAL_BANCOUNT  0x30b7a4cb
#define QUERYVAL_SENT	   0x5bacf98d
#define QUERYVAL_RECEIVED  0xaa19692e


extern char *friendstati[];

char *GetStuff(char *profilename, char *section, char *key, int size, char *output);
void WriteStuff(char *profilename, char *section, char *key, char* value);
char *GetStuffConfig(char *section, char *key, char *output);
void WriteStuffConfig(char *section, char *key, char* value);
char *TimeStamp(char *buf);

char *revstrchr(char *str, int tofind);
char *strrevex(char *string, char *result);
int charcount(char *str, int tofind);
void strcpychr(char *dest, char *src, int c);
char *lcase(char *str);
char *ucase(char *str);
char *lcasecpy(char *dest, char *src);
char *ucasecpy(char *dest, char *src);

void StrToHexOut(char *data, int len, HWND hwnd);

bool DecodeCDKey(char *CDKey, unsigned long *ProdVal, unsigned long *PubVal, unsigned long *PrivVal);
int GetBNLSClient(unsigned long client);

unsigned int __fastcall GetIcon(unsigned long client, unsigned long flags);
unsigned long GetLVColor(unsigned long flags);
unsigned long GetPingTextColor(unsigned long ping, unsigned long flags);
unsigned long GetColor(unsigned long flags);

int GetUptimeString(unsigned long time, char *outbuf);
int GetLngUptimeString(unsigned long time, char *outbuf);
int GetSmallUptimeString(unsigned long time, char *outbuf);

void FormatText(char *dest, char *src, int index, LPGREETARGS ga);

void UpdatelblChannel(int index);
void ReloadChannelList(int index);
void ResetChLVContents(int index);
void ClearLVTmpToolTipText(int index);

///////////////////////////////////////////////////////

void InsertSubItem(int index, int column, char *text, int icon, int botindex);
int LVFindItem(char *text, int index);
unsigned long GetItemlParam(long iIndex, HWND hwnd);
char *GetItemText(int iIndex, char *outbuf, unsigned int buflen, int index);
void AddLVItemSimple(char *text, unsigned long color, int index);
void InsertItem(char *szItem, int iIcon, int iIndex, LPLVCOLOR lpColor, int index);
char *GetItemTextLt(HWND hwnd, int index, char *buf, int size);



void XREncrypt(unsigned char *in, unsigned char *out, int channel, int factor);
void XRDecrypt(unsigned char *in, unsigned char *out, int channel, int factor);

void HexToStr(char *in, char *out);

void QueryFormatVars(char *output, char *value, int index);

//int __fastcall GetProfileFromBot(int bothash);
//int __fastcall GetBotFromProfile(int profilehash); 
int __fastcall GetTabFromIndex(int index);

void FreeNLS(int index);
void FreeAllLVStuff(int index);

void CheckCommCtrl32Ver();

void LoadBinaryImages();

char *GetServer(int index, bool wc3);
int GetPingIcon(unsigned long ping, unsigned long flags);

void CheckMissingFiles();

void __stdcall PingSpoofProc(int index);
void WaitForPingSpoof(int index);

char *TextTransformOutgoing(char *text, int txtattrib);
void TextTransformToAltCaps(char *text);
void TextTransformToLeetSpeak(char *text);


	 
int stricmp_(const char *s1, const char *s2);
char *strrev(char *str);
int __fastcall GetTabItemParam(int item);

bool IsFileValid(char *filename, LPFILETIME lpft);

#pragma warning(disable	: 4035)


inline void fastswap32(unsigned __int32 *num) {									
	__asm {
		mov ecx, dword ptr [num]
		mov eax, dword ptr [ecx]
		bswap eax
		mov dword ptr [ecx], eax
	}
}


inline bool cmphash(const char *h1, const char *h2) {
	__asm {
		cld
		mov esi, h1
		mov edi, h2
		cmpsd
		jnz done
		cmpsd
		jnz done
		cmpsd
		jnz done
		cmpsd
done:	  
		setz al
		movzx eax, al
	}
}


inline div_t udiv(unsigned int numer, unsigned int denom) {
	__asm {
		xor edx, edx
		mov eax, numer
		div denom
	}
}

#pragma warning(default : 4035)

