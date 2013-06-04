LRESULT CALLBACK ProfileProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
void Send0x26(char *username, int index);
void Send0x26AccInfo(int index);
void Parse0x26(char *data, int index);
void Send0x27(int index);

