#define NUM_BNCSUTILFXNS 13
#define NUM_STORMFXNS    20

//bncsutil
typedef bool (__stdcall *lpfn_kd_quick)(char *CDKey, long ClientToken, long ServerToken, long *PublicValue, long *ProductValue, char *HashBuffer, int BufferLen);
typedef bool (__stdcall *lpfn_checkRevisionFlat)(char *ValueString, char *File1, char *File2, char *File3, int mpqNumber, int *Checksum);
typedef int (__stdcall *lpfn_getExeInfo)(char *Filename, char *exeInfoString, int infoBufferSize, int *Version, int Platform);
typedef int (__stdcall *lpfn_nls_init)(char *Username, char *Password);
typedef int (__stdcall *lpfn_nls_free)(int NLS);
typedef int (__stdcall *lpfn_nls_account_create)(int NLS, char *buffer, int BufLen);
typedef int (__stdcall *lpfn_nls_account_logon)(int NLS, char *buffer, int BufLen);
typedef int (__stdcall *lpfn_nls_check_M2)(int NLS, char *M2, char *B, char *Salt);
typedef int (__stdcall *lpfn_nls_account_change_proof)(int NLS, char *buffer, char *NewPassword, char *B, char *Salt);
typedef int (__stdcall *lpfn_nls_get_A)(int NLS, char *Out);
typedef int (__stdcall *lpfn_nls_get_M1)(int NLS, char *Out, char *B, char *Salt);
typedef int (__stdcall *lpfn_nls_get_S)(int NLS, char *Out, char *B, char *Salt);
typedef int (__stdcall *lpfn_nls_get_K)(int NLS, char *Out, char *s);

//storm
typedef void (__stdcall *lpfnSBigNew)(BigBuffer *buf); // 624
typedef void (__stdcall *lpfnSBigDel)(BigBuffer ptr); // 606
typedef void (__stdcall *lpfnSBigPowMod)(BigBuffer result, BigBuffer base, BigBuffer expnt, BigBuffer mod); // 628
typedef void (__stdcall *lpfnSBigFromUnsigned)(BigBuffer result, DWORD num); // 612
typedef void (__stdcall *lpfnSBigFromBinary)(BigBuffer result, const void *in, int count); // 609
typedef void (__stdcall *lpfnSBigToBinaryBuffer)(BigBuffer in, void *result, DWORD incount, DWORD *outcount); // 638
typedef void (__stdcall *lpfnSBigAdd)(BigBuffer result, BigBuffer a, BigBuffer b); // 601
typedef void (__stdcall *lpfnSBigSub)(BigBuffer result, BigBuffer a, BigBuffer b); // 636
typedef void (__stdcall *lpfnSBigMul)(BigBuffer result, BigBuffer a, BigBuffer b); // 622
typedef int (__stdcall *lpfnSBigCompare)(BigBuffer a, BigBuffer b); // 603
typedef void (__stdcall *lpfnSBigCopy)(BigBuffer a, BigBuffer b); // 604
typedef void (__stdcall *lpfnSBigMod)(BigBuffer result, BigBuffer a, BigBuffer b); //621
typedef bool (__stdcall *lpfnSFileDestroy)(); //262
typedef bool (__stdcall *lpfnSFileSetLocale)(int locale); //272
typedef bool (__stdcall *lpfnSFileOpenArchive)(char *name, int flags, int unk, HANDLE *hArchive); //266
typedef bool (__stdcall *lpfnSFileOpenFileEx)(HANDLE hArchive, char *filename, int unk, HANDLE *hFile);	//268
typedef long (__stdcall *lpfnSFileGetFileSize)(HANDLE hFile, int *filesizehigh); //265
typedef bool (__stdcall *lpfnSFileReadFile)(HANDLE hFile, void *buffer, int size, int *bytesread, int unk); //269
typedef bool (__stdcall *lpfnSFileCloseFile)(HANDLE hFile);	//253
typedef bool (__stdcall *lpfnSFileCloseArchive)(HANDLE hArchive); //252

//checkrevision
typedef int (__stdcall *lpfn_CheckRevisionEx)
(char *FileExe, char *FileStormDll, char *FileBnetDll, char *HashText, int *Version,
 int *Checksum, char *exeInfo, char *PathToDLL, char *PathToLockdown01, char *PathToVideoBin);

typedef struct _bncsutil {
	lpfn_kd_quick           kd_quick;
	lpfn_checkRevisionFlat  checkRevisionFlat;
	lpfn_getExeInfo			getExeInfo;
	lpfn_nls_init			nls_init;
	lpfn_nls_free			nls_free;
	lpfn_nls_account_create nls_account_create;
	lpfn_nls_account_logon  nls_account_logon;
	lpfn_nls_check_M2		nls_check_M2;
	lpfn_nls_account_change_proof nls_account_change_proof;
	lpfn_nls_get_A			nls_get_A;
	lpfn_nls_get_M1			nls_get_M1;
	lpfn_nls_get_S			nls_get_S;
	lpfn_nls_get_K			nls_get_K;
} BNCSUTIL, *PBNCSUTIL;

typedef struct _storm { 
	lpfnSBigNew			   SBigNew;
	lpfnSBigDel			   SBigDel;
	lpfnSBigPowMod		   SBigPowMod;
	lpfnSBigFromUnsigned   SBigFromUnsigned;
	lpfnSBigFromBinary	   SBigFromBinary;
	lpfnSBigToBinaryBuffer SBigToBinaryBuffer;
	lpfnSBigAdd			   SBigAdd;
	lpfnSBigSub            SBigSub;
	lpfnSBigMul			   SBigMul;
	lpfnSBigCompare		   SBigCompare;
	lpfnSBigCopy		   SBigCopy;
	lpfnSBigMod			   SBigMod;
	lpfnSFileDestroy	   SFileDestroy;
	lpfnSFileSetLocale     SFileSetLocale;
	lpfnSFileOpenArchive   SFileOpenArchive;
	lpfnSFileOpenFileEx    SFileOpenFileEx;
	lpfnSFileGetFileSize   SFileGetFileSize;
	lpfnSFileReadFile	   SFileReadFile;
	lpfnSFileCloseFile	   SFileCloseFile;
	lpfnSFileCloseArchive  SFileCloseArchive;
} STORM, *LPSTORM;

extern HMODULE hBNCSUtil;
extern HMODULE hCRLib;
extern HMODULE hStorm;
extern BNCSUTIL bncsutil;
extern STORM storm;
extern lpfn_CheckRevisionEx CheckRevisionEx;

void InitalizeExternalLibs();
void FreeExternalLibs();


