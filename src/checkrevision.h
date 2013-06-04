#define SET_MASKED_BITS(x, mask, bits) ((x) = ((x & ~(mask)) | ((bits) & mask)))

struct seed_table {
	unsigned long seed1;
	unsigned long seed2;
};

typedef bool (__cdecl *lcr_shuf) (char *, int, const char *, int);
typedef void (__cdecl *lcr_hash) (SHA1_CTX *ctx, HMODULE);

bool DoCheckRevision(char *ChecksumFormula, LPFILETIME lpMpqFt, char *mpqName, unsigned long *Checksum,
					 unsigned long *exeVersion, char *exeInfo, int index);
int ExtractCRMPQNumber(char *mpqName);
void GetCheckRevisionLib(char *mpqName, LPFILETIME lpMpqFt, int index);
int patch_dword(unsigned long AddressToPatch, unsigned long Value);
int patch_word(unsigned long AddressToPatch, WORD Value);
int prepare_backend(HMODULE hLockdown, int file_lock);
unsigned long get_fileversion(const char *file_path);
void init_context(SHA1_CTX *context, char *shuffled, int len);
bool hash_videodump(SHA1_CTX *context, const char *videobuf);
void double_hash(SHA1_CTX *context, unsigned char result[20]);
unsigned long finish_sub(unsigned long *arg_one, unsigned long *arg_two);
int finish(unsigned char *arg_output, unsigned int *arg_output_length, unsigned char *arg_heap, int arg_10h);

unsigned long CheckRevision(LPCTSTR lpszFileName0, LPCTSTR lpszFileName1, LPCTSTR lpszFileName2,
							LPCTSTR lpszMpqFileName, LPCTSTR lpszValueString);
unsigned long GetExeInfo(char *lpszFileName, char *lpExeInfoString);
//BOOL CheckRevision(LPCTSTR lpszFileName1, LPCTSTR lpszFileName2, LPCTSTR lpszFileName3,
//		LPCTSTR lpszValueString, DWORD * lpdwVersion, DWORD * lpdwChecksum,
//		LPSTR lpExeInfoString, LPCTSTR lpszMpqFileName);
int CheckRevisionLD(const char *file_game, const char *file_strm, const char *file_bttl,
						   const char *server_hash, unsigned long &out_version, unsigned long &out_checksum,
						   char *out_digest, const char *file_lock, const char *file_vdmp);

