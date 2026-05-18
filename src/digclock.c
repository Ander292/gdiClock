#include <windows.h>
#define _UNICODE
#define UNICODE
#include "digclock.h"
#include "resources.h"

#define TIMER_ONE 1
#define POLL_FREQUENCY 100

#define CELL_SIZE 16

#define BITMAP_WIDTH 6
#define BITMAP_HEIGTH 8

#define DEFAULT_WIDTH (CELL_SIZE * BITMAP_WIDTH * 6 + CELL_SIZE/2)
#define DEFAULT_HEIGTH (CELL_SIZE* BITMAP_HEIGTH + CELL_SIZE / 2)
#define DIVIZER_X_START (CELL_SIZE * BITMAP_WIDTH * 2 - CELL_SIZE/4)
#define DIVIZER_X_END (DIVIZER_X_START + CELL_SIZE/2)

#define FormatColor(r, g, b) (COLORREF)((r) | (g) << 8 | (b) << 16)
#define sBoolean(i) ((i) ? L"True" : L"False")

wchar_t *szProgramName = L"Clock";

typedef struct window_size_info{
    int cx;
    int cy;
} wSizeInfo;

wSizeInfo wsInfo;
int time[6] = {0};
int tOld[6] = {0};
int sTime[6] = {0};

int iStyle = STYLE_FULL;

COLORREF cColor;


void DisplayFatalError(DWORD dwError){
    LPWSTR wcStr = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, 
        dwError,
        0,
        (LPWSTR)&wcStr,
        0,
        NULL
    );
    MessageBoxW(NULL, wcStr, L"Fatal error", MB_OK | MB_ICONERROR);

    LocalFree(wcStr);
    exit(dwError);
}


static inline void DrawNumber(HDC hdc, int mat[8][6], int startX, int startY, HBRUSH hDefBrush){
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 6; j++){
            RECT sR;
            sR.left = startX + j * CELL_SIZE;
            sR.right = sR.left + CELL_SIZE;
            sR.top = startY + i * CELL_SIZE;
            sR.bottom = sR.top + CELL_SIZE;
            
            int colorCode = mat[i][j];
            if(colorCode == 1){
                FillRect(hdc, &sR, hDefBrush);
            }
        }
    }
}

LRESULT CALLBACK MainWindowCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

    switch(uMsg){
        case WM_CREATE: {
            SetTimer(hWnd, TIMER_ONE, POLL_FREQUENCY, NULL);
        } return 0;

        case WM_SIZE: {
            wsInfo.cx = LOWORD(lParam);
            wsInfo.cy = HIWORD(lParam);
        } return 0;

        case WM_PAINT:{
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            HBRUSH hBrush = CreateSolidBrush(cColor);

            for(int i = 0; i < 6; i++){
                DrawNumber(hdc, nums[iStyle][time[i]], i * 6 * CELL_SIZE, 0, hBrush);
            }

            RECT dr1;
            dr1.left = DIVIZER_X_START;
            dr1.right = DIVIZER_X_END;
            dr1.top = 0;
            dr1.bottom = wsInfo.cy;
            FillRect(hdc, &dr1, hBrush);

            dr1.left += DIVIZER_X_START + CELL_SIZE/4;
            dr1.right += DIVIZER_X_START + CELL_SIZE/4;
            FillRect(hdc, &dr1, hBrush);

            DeleteObject(hBrush);
            EndPaint(hWnd, &ps);
        } return 0;

        case WM_KEYDOWN:{
            RECT r;
            GetWindowRect(hWnd, &r);
            switch(wParam){
                case 'P':{
                    WCHAR wczBuffer[256];
                    wsprintfW(wczBuffer, L"ClientSize: %dx%d || CellSize: %d\n"
                        L"WindowDim: x:%d | y:%d | dx:%d | dy:%d\n"
                        L"Style: %ls",
                        wsInfo.cx, wsInfo.cy, CELL_SIZE, r.left, r.top, r.right, r.bottom,
                        (iStyle ? (iStyle == 1 ? L"Reduced" : L"Dot") : L"Default"));
                    MessageBoxW(hWnd, wczBuffer, L"Program info", MB_OK | MB_ICONMASK);
                } break;
                case 'C':{
                    iStyle++;
                    if(iStyle > 2) iStyle = 0;
                    InvalidateRect(hWnd, NULL, TRUE);
                } break;
                case 'W':{ // For ctrl+w alternative to ALT+F4
                    BOOL ctrlIsDown = GetKeyState(VK_CONTROL) < 0;
                    if(ctrlIsDown) PostQuitMessage(0);
                } break;
            } break;
        } return 0;
        case WM_TIMER:
            switch(wParam){
                case TIMER_ONE: {
                    SYSTEMTIME st;
                    GetLocalTime(&st);
                    BOOL toRedraw = FALSE;
                    cColor = FormatColor(63 + 8 * st.wHour, 4 * st.wMinute, 4 * st.wSecond);
                    time[0] = st.wHour / 10 % 10;
                    time[1] = st.wHour % 10;
                    time[2] = st.wMinute / 10 % 10;
                    time[3] = st.wMinute % 10;
                    time[4] = st.wSecond / 10 % 10;
                    time[5] = st.wSecond % 10;
                    for(int i = 0; i < 6; i++){
                        if(time[i] != tOld[i]) toRedraw = TRUE;
                        tOld[i] = time[i];
                    }
                    if(toRedraw) InvalidateRect(hWnd, NULL, TRUE);
                } break;
                default: MessageBeep(MB_ICONWARNING); break;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){

    WNDCLASSW WindowClass = {0};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = hInstance;
    WindowClass.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_MAIN_ICON));
    WindowClass.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW));
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = szProgramName;
    
    if(!RegisterClassW(&WindowClass)) DisplayFatalError(GetLastError());

    RECT r = {0, 0, DEFAULT_WIDTH, DEFAULT_HEIGTH};
    AdjustWindowRect(&r, WS_VISIBLE | WS_SYSMENU, TRUE);
    
    HWND hWindow = CreateWindowW(
        szProgramName,
        szProgramName,
        WS_VISIBLE | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        r.right - r.left,
        r.bottom - r.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if(!hWindow) DisplayFatalError(GetLastError());

    ShowWindow(hWindow, nShowCmd);
    UpdateWindow(hWindow);

    MSG Message;
    while(GetMessageW(&Message, NULL, 0, 0)){
        TranslateMessage(&Message);
        DispatchMessageW(&Message);
    }
    return Message.wParam;
}