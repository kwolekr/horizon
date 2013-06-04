void __fastcall InsertByte(unsigned char data);
void __fastcall InsertWORD(unsigned short data);
void __fastcall InsertDWORD(unsigned long data);
void __fastcall InsertVoid(void *data, unsigned int len);
void __fastcall InsertNonNTString(char *data);
void __fastcall InsertNTString(char *data);
void __fastcall SendPacket(unsigned char PacketID, int index);
void __fastcall SendBNLSPacket(unsigned char PacketID, SOCKET sck);
void __fastcall SendPacketBNFTP(SOCKET sck);
//void SendPacketUDP(unsigned char PacketID, unsigned char cls, PPLAYER player, bool repeat);

