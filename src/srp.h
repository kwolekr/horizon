#define SWAP4(x) ((((x) >> 24) & 0x000000FF) \
				| (((x) >> 8) & 0x0000FF00) \
				| (((x) << 8) & 0x00FF0000) \
				| (((x) << 24) & 0xFF000000))
#define BSWAP(a,b,c,d) ((((((a << 8) | b) << 8) | c) << 8) | d)

void SRPInit(char *user, char *pass);
void SRPCalculateA(char *outbuf);
void SRPCalculateM1(char *outbuf, char *B, char *salt);

