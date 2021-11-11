#include "framework.h"
#include "FolderListener.h"
#include <iostream>
#include <sstream>
#include <mmsystem.h>

#define MAX_LOADSTRING 100

HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hWnd_parent;
HWND hwndButton;
NOTIFYICONDATA iconData = {};
DWORD threadId;
HANDLE thread;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Browser(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI ListenerCall( _In_ LPVOID lpParam);

typedef struct TextData {
    LPWSTR text;
} TEXTDATA, *PTEXTDATA;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FOLDERLISTENER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FOLDERLISTENER));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(ICON_TRAY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FOLDERLISTENER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 
   hWnd_parent = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 400, 120, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd_parent)
   {
      return FALSE;
   }
   ShowWindow(hWnd_parent, nCmdShow);
   UpdateWindow(hWnd_parent);
   return TRUE;
}

//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //debug(message);
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case 0:
                DialogBox(hInst, MAKEINTRESOURCE(BROWSE_DIALOG), hWnd, Browser);
              
                break;
            case TRAY_MENU_EXIT_ITEM:
                Shell_NotifyIcon(NIM_DELETE, &iconData);
                TerminateThread(thread, 0);
                DestroyWindow(hWnd_parent);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            switch (HIWORD(wParam)) {
                case BN_CLICKED:
                    switch (wmId) {
                    case IDM_ABOUT:
                        break;
                    default:
                        break;
                    }
                }
            
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            hwndButton = CreateWindowW(
                L"BUTTON",  // Predefined class; Unicode assumed 
                L"Выбрать отслеживаемый путь",      // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
                10,         // x position 
                10,         // y position 
                350,        // Button width
                30,        // Button height
                hWnd_parent,     // Parent window
                NULL,       // No menu.
                (HINSTANCE)GetWindowLongPtr(hWnd_parent, GWLP_HINSTANCE),
                NULL);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_USER_SHELLICON:
        switch (LOWORD(lParam))
        {
            case WM_LBUTTONDOWN:
                POINT loc = {};
                GetCursorPos(&loc);
                debug(loc.x);
                auto menu = CreatePopupMenu();
               /* MENUITEMINFOW itemInfo = {};
                itemInfo.cbSize = sizeof(MENUITEMINFO);
                itemInfo.fMask = MIIM_FTYPE | MIIM_ID;
                itemInfo.fType = MFT_STRING;
                itemInfo.wID = TRAY_MENU_EXIT_ITEM;

                itemInfo.dwTypeData = const_cast<LPTSTR>(TEXT("&Выход"));
                debug(itemInfo.dwTypeData);
                itemInfo.cch = 5;*/
                //InsertMenuItemW(menu, 0, false, &itemInfo);
                AppendMenuW(menu, MF_STRING, TRAY_MENU_EXIT_ITEM, L"Выход");
                SetForegroundWindow(hWnd_parent);
                TrackPopupMenu(menu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, loc.x, loc.y, 0, hWnd_parent, NULL);
            break;
        }
        break;
    case 0:
        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Browser(HWND dlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                iconData.cbSize = sizeof(NOTIFYICONDATA);
                iconData.hWnd = hWnd_parent;
                iconData.uID = APP_TRAY;
                iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP ;
                iconData.uCallbackMessage = WM_USER_SHELLICON;
                iconData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(ICON_TRAY));
                LoadString(hInst, IDS_APP_TITLE, iconData.szTip, MAX_LOADSTRING);
                Shell_NotifyIcon(NIM_ADD, &iconData);
                
                int length = GetWindowTextLength(GetDlgItem(dlg, PATH)) + 1;
                LPWSTR text = new TCHAR[length];
                GetDlgItemText(dlg, PATH, text, length);
                if (GetFileAttributesW(text) == INVALID_FILE_ATTRIBUTES) {
                    DialogBox(hInst, MAKEINTRESOURCE(ERROR_DIALOG), dlg, About);
                    return (INT_PTR)TRUE;
                }
                PTEXTDATA data = (PTEXTDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TEXTDATA));
                if (data == NULL) debug(L"DATA is null");
                data->text = text;
                  
                thread = CreateThread(NULL, 0, ListenerCall, data, 0, NULL);
                if (thread == NULL) {
                    debug(L"thread is null");
                }
                EndDialog(dlg, LOWORD(wParam));
                ShowWindow(hWnd_parent, SW_HIDE);
            } 
            else if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(dlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Notice(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            ShowWindow(hWnd_parent, SW_SHOW);
            UpdateWindow(hWnd_parent);
            EndDialog(hDlg, LOWORD(wParam));
            TerminateThread(thread, 0);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

DWORD WINAPI ListenerCall( _In_ LPVOID lpParam) {
    PTEXTDATA data = static_cast<PTEXTDATA>( lpParam);
    FListener listener;
    listener.startListen(data->text);
    return 0;
}

void NotifyDirectory(LPTSTR) {
    MessageBeep(MB_OK);
    DialogBox(hInst, MAKEINTRESOURCE(NOTICE_DIALOG), hWnd_parent, Notice);
}

void debug(int num) {
    wchar_t* result = new wchar_t[10];
    swprintf(result, 10, L"%d", num);
    OutputDebugStringW(result);
    OutputDebugStringW(L"\n");
}

void debug(LPCWSTR str) {
    OutputDebugStringW(str);
    OutputDebugStringW(L"\n");
}
