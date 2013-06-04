#define NUM_BNICONS 24

#define STATEICO_DISCONN 0
#define STATEICO_CONN    1 
#define STATEICO_CONNERR 2

typedef struct _menuitem {
	HMENU *lphMenu;
	int type;
	int id;
	const char *title;
} MENUITEM, *LPMENUITEM;

extern COLORREF gradclr1, gradclr2;
extern HICON bnicons[NUM_BNICONS], stateicons[3];
extern HIMAGELIST hCfgTabiml;
extern int LogPixelsY;
extern HDC wallmemdc;

void RegisterWindows();
void InitalizeMenus();
void InitIML();

void DrawGradientFill(HDC hdc, LPRECT rect, int clrfrom, int clrto);
void CreateMDIChildControls(HWND hwnd, int index);
void ShiftWindowMultiClip(int index, HWND cbotxthwnd);
void UpdateLVDrag(HWND hwnd);
unsigned long ParseCustomDraw(HWND hWnd, LPNMLVCUSTOMDRAW lParam);
unsigned long __stdcall SplashScreenProc(void *param);
void ContextLVTooltip(int i, LPMSG lpMsg);
int TooltipCustDraw(LPNMTTCUSTOMDRAW lpttcd);
void SetNewFont(char *fontname, int size);
int PixelSizeToLogicalSize(int pixelsize);
void InitGDIItems();
HTREEITEM AddItemToTree(HWND hwndTV, LPSTR lpszItem, int nLevel);
void PaintTransparentRect(HDC hdc, RECT *paintr, COLORREF color, int alpha);
int CalcRectWidth(HDC hdc);
void AlphaBlendWallpaperInit(int alpha);

