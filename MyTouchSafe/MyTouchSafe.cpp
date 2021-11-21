// MyTouchSafe.cpp : 定义应用程序的入口点。
//

#include <sstream>
#include "framework.h"
#include "MyTouchSafe.h"
using namespace std;

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


BOOL IsRunasAdmin()
{
    BOOL bElevated = FALSE;
    HANDLE hToken = NULL;

    // Get current process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return FALSE;

    TOKEN_ELEVATION tokenEle;
    DWORD dwRetLen = 0;

    // Retrieve token elevation information
    if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen))
    {
        if (dwRetLen == sizeof(tokenEle))
        {
            bElevated = tokenEle.TokenIsElevated;
        }
    }

    CloseHandle(hToken);
    return bElevated;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    if (!IsRunasAdmin()) {
        //MessageBox(0, L"Don't administrator", L"error", MB_TOPMOST | MB_ICONERROR);
        //exit(0);
    }

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYTOUCHSAFE, szWindowClass, MAX_LOADSTRING);

    if (FindWindow(szWindowClass, 0)) {
        MessageBox(0, L"Only one can be run", L"error", MB_TOPMOST | MB_ICONERROR);
        exit(0);
    }

    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYTOUCHSAFE));

    MSG msg;

    // 主消息循环:
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

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = 0;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MYTOUCHSAFE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = 0;
    return RegisterClassExW(&wcex);
}

HWND myhwnd;
HHOOK g_hook;
BYTE ks = 0;
int timer = 0;
LRESULT CALLBACK KeyHookPrpc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        LPKBDLLHOOKSTRUCT pKB = (LPKBDLLHOOKSTRUCT)lParam;
        switch (wParam) {
        case WM_KEYDOWN:
            if (pKB->vkCode == VK_VOLUME_DOWN) ks |= 1;
            if (pKB->vkCode == VK_VOLUME_UP) ks |= 2;
            if (ks != 3) break;
            timer = 0;
            SetTimer(myhwnd, 1, 100, 0);
            break;
        case WM_KEYUP:
            if (pKB->vkCode == VK_VOLUME_DOWN) ks &= 2;
            if (pKB->vkCode == VK_VOLUME_UP) ks &= 1;
            if (ks != 0) break;
            KillTimer(myhwnd, 1);
            break;
        }
    }
    return CallNextHookEx(g_hook, code, wParam, lParam);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   /*HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);*/

   auto hWnd = CreateWindowEx(
       WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
       szWindowClass, szTitle, WS_VISIBLE | WS_POPUP,
       0, 0, 1, 1, GetDesktopWindow(), nullptr, hInstance, nullptr
   );

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   myhwnd = hWnd;
   g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookPrpc, 0, 0);
   if (!g_hook) exit(0);

   return TRUE;
}

void CloseProcess() {
    auto h = FindWindow(L"ZrTouchClass", 0);
    if (!h) {
        OutputDebugString(L"no windows");
        return;
    }
    DWORD pro;
    if (!GetWindowThreadProcessId(h, &pro)) {
        OutputDebugString(L"no process");
        return;
    }
    auto hp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pro);
    if (!hp) {
        OutputDebugString(L"no access");
        return;
    }
    if (!TerminateProcess(hp, 0)) {
        OutputDebugString(L"no do");
        return;
    }
    OutputDebugString(L"ok");
}
//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
        if (++timer == 50) {
            OutputDebugString(L"close\n");
            CloseProcess();
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        UnhookWindowsHookEx(g_hook);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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
