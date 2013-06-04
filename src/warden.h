typedef void (__stdcall *lpfnSendPacket)(char *data, int len);
typedef bool (__stdcall *lpfnCheckModule)(char *modname, unsigned long _2);
typedef unsigned long (__stdcall *lpfnLoadModule)(char *decryptkey, char *module, int modlen);
typedef void *(__stdcall *lpfnMemAlloc)(unsigned long len);
typedef void (__stdcall *lpfnMemFree)(void *mem);
typedef void (__stdcall *lpfnSetRC4)(void *lpKeys, unsigned long len);
typedef char *(__stdcall *lpfnGetRC4)(char *lpBuffer, unsigned long *len);

typedef void (__stdcall *lpfnGenKeys)(void *ppFncList, void *lpData, unsigned long dwSize);
typedef void (__stdcall *lpfnUnloadModule)(void *ppFncList);
typedef void (__stdcall *lpfnHandlePacket)(void *ppFncList, char *data, int len, unsigned long *dwBuffer);
typedef void (__stdcall *lpfnTick)(void *ppFncList, unsigned long _2);

/*typedef struct _wclass {
	void *lpModule;
	int   cbModSize;
	int   dllcount;
	void *moduleclass;
	void *lpfnInit;
	int   pktlenroundup;
	int   pktlenorig;
	void *pktdataorig;
	int   highflags;
} WCLASS, *LPWCLASS; */

typedef struct _wmodheader {
	unsigned long cbModSize;			//0x00
	void *lpfnFunc;						//0x04
	unsigned long rvaFxnTableReloc;		//0x08
	int reloccount;						//0x0C
	void *lpfnInit;						//0x10
	unsigned int unk1;					//0x14
	unsigned int unk2;					//0x18
	unsigned long rvaImports;			//0x1C
	int dllcount;					    //0x20
	int sectioncount;					//0x24
	unsigned long sectionlen;			//0x28
	void *lpfnExceptHandler;			//0x2C
	unsigned int unk3;					//0x30
	unsigned long rvaImportAddrOffset;  //0x34
	unsigned int unk4;					//0x38
	unsigned int unk5;					//0x3C
	unsigned long rvaGlobalVars;		//0x40
	unsigned int unk6;					//0x44
	unsigned int unk7;					//0x48
	unsigned short unk8;				//0x4C
} WMODHEADER, *LPWMODHEADER;

typedef struct _wmodprotect {
	void *base;
	unsigned long len;
	unsigned long protectdword;
} WMODPROTECT, *LPWMODPROTECT;

typedef struct _snpFnTable {
	lpfnSendPacket snpSendPacket;
	lpfnCheckModule snpCheckModule;
	lpfnLoadModule snpLoadModule;
	lpfnMemAlloc snpMemAlloc;
	lpfnMemFree snpMemFree;
	lpfnSetRC4 snpSetRC4;
	lpfnGetRC4 snpGetRC4;
	
} SNPFNTABLE, *LPSNPFNTABLE;

typedef struct _wardenFnTable {
	lpfnGenKeys wdnGenKeys;
	lpfnUnloadModule wdnUnloadModule;
	lpfnHandlePacket wdnHandlePacket;
	lpfnTick wdnTick;
} WARDENFNTABLE, *LPWARDENFNTABLE;

void Parse0x5E(char *data, int index);

void WardenParseCommand0(char *data, int len, int index);
void WardenParseCommand1(char *data, int len, int index);
void WardenParseCommand2(char *data, int len, int index);
void WardenParseCommand5(char *data, int len, int index);


void RC4Crypt(unsigned char *key, unsigned char *data, int length);
void WardenKeyInit(char *KeyHash, int index);
void WardenKeyDataInit(t_random_data *source, char *seed, int length);
void WardenKeyGenerate(unsigned char *key_buffer, unsigned char *base, unsigned int baselen);
void WardenKeyDataUpdate(t_random_data *source);
void WardenKeyDataGetBytes(t_random_data *source, char *buffer, int bytes);
char WardenKeyDataGetByte(t_random_data *source);

int WardenDecryptInflateModule(char *modulename, char *module, int modulelen, char *keyseed, int index);
bool WardenPrepareModule(char *filename);
void WardenModuleInitalize(LPWMODHEADER lpwmodh);
void WardenUnloadModule();

FARPROC WINAPI MyGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MySetUnhandledExceptionFilter(
			LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
BOOL WINAPI MyTerminateProcess(HANDLE hProcess, UINT uExitCode);

void __stdcall WdnCbkSendPacket(char *data, int len);
bool __stdcall WdnCbkCheckModule(char *modname, unsigned long _2);
unsigned long __stdcall WdnCbkLoadModule(char *decryptkey, char *module, int modlen);
void *__stdcall WdnCbkMemAlloc(unsigned long len);
void __stdcall WdnCbkMemFree(void *mem);
void __stdcall WdnCbkSetRC4(void *lpKeys, unsigned long len);
char *__stdcall WdnCbkGetRC4(char *buffer, unsigned long *len);

unsigned long WardenGenerateChecksum(char *data, int len);

