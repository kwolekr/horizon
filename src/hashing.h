void SHA1InitLD(SHA1_CTX* context);
void SHA1UpdateLD(SHA1_CTX* context, const unsigned char* data, unsigned int len);
void SHA1TransformLD(unsigned int state[5], const unsigned char buffer[64]);
void SHA1FinalLD(SHA1_CTX* context, unsigned char digest[20]);
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))
#define blk0(i) block->l[i]
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

void MD5(char *source, unsigned long len, char *output);
void SHA1(char *source, unsigned long len, char *output);
void BSHA1(const void *src, const int len, unsigned long *result);
void hashPassword(const char* password, char* outBuffer);
void doubleHashPassword(const char* password, unsigned int clientToken, unsigned int serverToken, char* outBuffer);
bool DecodeStarcraftKey(char *key);
bool DecodeWarcraft2Key(char *cdkey);
bool DecodeWC3Key(char *key, unsigned int *product2, unsigned int *value1, char *w3value2);
void HashCDKey(char *OutBuf, unsigned long ClientToken, unsigned long ServerToken, 
			   unsigned long ProductVal, unsigned long PublicVal, unsigned long PrivateVal);
void HashWAR3Key(unsigned long ClientToken, unsigned long ServerToken, unsigned long ProductVal, unsigned long PublicVal,char *PrivateVal,char *output);
inline void mult(int r, const int x, int* a, int dcByte);
void decodeKeyTable(int* keyTable);
char Get_Hex_Value(unsigned long v);
int Get_Num_Value(char c);

