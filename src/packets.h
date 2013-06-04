#define LEGACYSVRVERSION 1

void ParsePacket(unsigned char packetid, char *data, int index);
void Send0x05(int index);
void Parse0x05(char *data, int index);
void Send0x06(int index);
void Send0x12(int index);
void Parse0x1D(char *data, int index);
void Send0x1E(int index);
void Parse0x28(char *data, int index);
void Parse0x06(char *data, int index);
void Send0x07(unsigned long exeVersion, char *exeInfo, unsigned long checksum, int index);
void Parse0x07(char *data, int index);
void Send0x29(int index);
void Parse0x29(char *data, int index);
void Send0x30(int index);
void Parse0x30(char *data, int index);
void Send0x36(int index);
void Parse0x36(char *data, int index);

void Send0x50(int index);
void Parse0x50(char *data, int index);
void Send0x51(unsigned long ProductValue, unsigned long PublicValue, char *KeyHash,
			  unsigned long exeVersion, unsigned long Checksum, char *exeInfo,
			  int index);
void Parse0x51(char *data, int index);
void Send0x3A(int index);
void Parse0x3A(char *data, int index);
void Send0x3D(int index);
void Parse0x3D(char *data, int index);
void Send0x0A(int index);
void Parse0x0A(char *data, int index);

void Parse0x4E(char *data, int index);


void Send0x52(int index);
void Parse0x52(char *data, int index);
void Send0x53(int index);
void Parse0x53(char *data, int index);
void Parse0x54(char *data, int index);
void Send0x55(int index);
void Parse0x55(char *data, int index);
void Parse0x56(char *data, int index);

void Send0x31(int index);
void Parse0x31(char *data, int index);

void Send0x59(int index);
void Send0x5A(int index);
void Send0x5B(int index);

void Parse0x67(char *data, int index);
void Parse0x68(char *data, int index);
void Parse0x69(char *data, int index);

void Parse0x15(char *data, int index);
void Parse0x65(char *data, int index);
void Send0x1C(int index, bool starting);
void Parse0x1C(char *data, int index);
void Send0x22(int index);
void Send0x2C(int index);

void SendBNLS0x09(char *mpqName, char *ChecksumFormula, SOCKET sbnls, int index);
void SendBNLS0x18(char *mpqName, char *ChecksumFormula, SOCKET sck, int index);
void SendBNLS0x1A(char *mpqName, char *ChecksumFormula, PFILETIME pmpqFiletime, SOCKET sck, int index);

