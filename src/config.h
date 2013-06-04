#define CFGTAB_CONNECTION 0
#define CFGTAB_ADVCONNECT 1
#define CFGTAB_INTERFACE  2
#define CFGTAB_MESSAGING  3
#define CFGTAB_MISC       4
#define CFGTAB_PROFILE    5

LRESULT CALLBACK ConfigProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConfigCtlProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
void TabChange(HWND hDialog);
void SaveSettings(HWND hDialog, int index);
void UpdateConfigPage(HWND hDialog, int index, int page);
int OpenFileDialog(char *buf, int len, char *title);
int OpenChooseColorDialog();
int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam);
void ToggleEnableStates(HWND hDialog, int control);
void InitalizeWindowEnabledStates(HWND hDialog);
void UpdateSampleFont(HWND hWnd_cbo);

