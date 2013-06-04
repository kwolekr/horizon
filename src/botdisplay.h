LRESULT CALLBACK BotProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
void CALLBACK UpdateStatusLVTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void PopulateBotInfoLV(HWND hDialog, HWND hwndlvw);
void HandleDisplayLVCustDraw(HWND hDialog, LPNMLVCUSTOMDRAW nmCustDraw);

