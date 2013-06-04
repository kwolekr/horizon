#define MS_PERPACKET  400
#define MS_PERBYTE	  35

void AddQueue(char *text, int index);
void CALLBACK QueueTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
int GetQueueTime(int len, int index);
void ClearQueue(int index);

