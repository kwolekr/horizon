#define WINAMP_PREV	 40044
#define WINAMP_PLAY  40045
#define WINAMP_PAUSE 40046
#define WINAMP_STOP	 40047
#define WINAMP_NEXT	 40048

void GetWinampPath();
bool InitWinamp();
void LoadWinamp();
void QuitWinamp();
void PushWinampButton(WPARAM button);
void SetWinampVolume(int volume);
void GetWinampSong(char *output);
void PlayWinampSong(char *songname);

