#define WM_DLCOMPLETE (WM_USER + 4357)

typedef struct _bnftpdl {
	int index;	
	char filename[64];
	bool performextract;
	char savefilename[MAX_PATH];
} BNFTPDL, *LPBNFTPDL;

bool DownloadFileFromBNFTP(char *filename, int index);
int InitiateDLAndWait(char *filename, char *savefilename, bool doextract, int index);
void __stdcall BNFTPDownloadWrapper(LPBNFTPDL pbnftpdl);
bool ExtractMPQ(char *archive, char *file, char *extractto);

