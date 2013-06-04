bool ConnectSocket(SOCKET *sck, char *server, unsigned short port);
void ConnectProfile(int index);
void ContinueConnectProcess(int index);
void DisconnectProfile(int index, bool graceful);
void HandleDataRecv(int i, SOCKET sck, LPARAM lParam);
void AutoReconnectStart(int index, int crcrate);
VOID CALLBACK AutoReconnectProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

