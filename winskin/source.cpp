//
// 窗体皮肤实现 - 在VC中简单实现绘制（五）
// 
// blog http://www.moguf.com/post/devwinskin05
//
// Created by 蘑菇房 moguf.com
//
#include <windows.h>
#include "atlbase.h"
#include "atlstr.h"
#include "atlimage.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);           // 主窗体消息处理
VOID    CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, int iCmdShow)
{
    static TCHAR szAppName[] = TEXT("winskin5");
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
        return 0;

	hwnd = CreateWindow(szAppName, TEXT("窗体皮肤实现 - 在VC中简单实现绘制（五）"),
		WS_OVERLAPPEDWINDOW | WS_VSCROLL,
		600, 400, 350, 250,
		NULL, NULL, hInstance, NULL);

	//设置分层窗口
	LONG style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
	style |= WS_EX_LAYERED;

	SetWindowLong(hwnd, GWL_EXSTYLE, style);


	// Load our PNG image
	CImage img;
	img.Load(L"time.png");

	// Get dimensions of Image
	int iWidth = img.GetWidth();
	int iHeight = img.GetHeight();

	// Make a memory DC + memory bitmap
	HDC hdcScreen = GetDC(NULL);
	HDC hDC = CreateCompatibleDC(hdcScreen);
	HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, iWidth, iHeight);
	HBITMAP hBmpOld = (HBITMAP)SelectObject(hDC, hBmp);

	// Draw image to memory DC
	img.Draw(hDC, 0, 0, iWidth, iHeight, 0, 0, iWidth, iHeight);

	// Call UpdateLayeredWindow
	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	POINT ptPos = { 0, 0 };
	SIZE sizeWnd = { iWidth, iHeight };
	POINT ptSrc = { 0, 0 };

	UpdateLayeredWindow(hwnd, hdcScreen, &ptPos, &sizeWnd, hDC, &ptSrc, 0, &blend, ULW_ALPHA);

	SelectObject(hDC, hBmpOld);
	DeleteObject(hBmp);
	DeleteDC(hDC);
	ReleaseDC(NULL, hdcScreen);

	SetWindowPos(hwnd, 0, 0, 0, iWidth, iHeight, SWP_NOZORDER | SWP_NOMOVE);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void OutputDebugPrintf(const char* strOutputString, ...)
{
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf_s(strBuffer, sizeof(strBuffer)-1, strOutputString, vlArgs);
	va_end(vlArgs);
	OutputDebugString(CA2W(strBuffer));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	RECT rw;            // 窗体大小，实际需要绘制非客户区的尺寸
	RECT rc;            // 客户区大小，用于绘制非客户区时扣除
	POINT pt;           //
	HRGN hTmp;          // 用于重新生成倒角的窗体区域
	BOOL bChanged;      //

	LRESULT test;

	static HRGN hRegion;            // 窗体样式句柄
	static BOOL bChangeSizeCalled;  // 窗体修改状态
	static RECT rWindowSize;        // 窗体尺寸状态


	switch (message) {
	case WM_CREATE:
		hRegion = 0;
		bChangeSizeCalled = FALSE;
		rWindowSize = { 0, 0, 0, 0 };
		// SetTimer(hwnd, 1, 33.33333, TimerProc);
		break;

	case WM_NCPAINT:
		GetWindowRect(hwnd, &rw);
		GetClientRect(hwnd, &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ClientToScreen(hwnd, &pt);
		OffsetRect(&rc, pt.x - rw.left, pt.y - rw.top);

		hdc = GetWindowDC(hwnd);
		ExcludeClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

		OffsetRect(&rw, -rw.left, -rw.top);
		// 使用这个方式比使用fillrect函数填充效果好，不闪烁
		SetBkColor(hdc, RGB(100, 200, 250));
		SetBkMode(hdc, TRANSPARENT);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rw, L"你好", 4, 0);

		ReleaseDC(hwnd, hdc);
		return 0;

	case WM_NCCALCSIZE:
		// 设置窗体非客户区尺寸
		((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].left += 1;
		((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].top += 100;
		((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].right -= 1;
		((LPNCCALCSIZE_PARAMS)lParam)->rgrc[0].bottom -= 1;
		return 0;

	case WM_NCACTIVATE:
		// 请求冲刷非客户区
		PostMessage(hwnd, WM_NCPAINT, 1, 0);
		return 0;

	case WM_NCHITTEST:
		test = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		OutputDebugPrintf("WM_NCHITTEST %d\n", test);
		if ((test == HTCLOSE) || (test == HTMAXBUTTON) ||
			(test == HTMINBUTTON) || (test == HTHELP))
			{
			return HTCAPTION;
			}
		return test;

    case WM_WINDOWPOSCHANGING:
        bChanged = FALSE;

        // 窗体位置发生改变，重新计算绘制样式
        if (!bChangeSizeCalled) {
            bChanged = (((LPWINDOWPOS)lParam)->flags & SWP_FRAMECHANGED);

            if ((((LPWINDOWPOS)lParam)->flags & SWP_NOMOVE) == 0) {
                rWindowSize.left = ((LPWINDOWPOS)lParam)->x;
                rWindowSize.top = ((LPWINDOWPOS)lParam)->y;
            }
            if ((((LPWINDOWPOS)lParam)->flags & SWP_NOSIZE) == 0) {
                bChanged = bChanged || (((LPWINDOWPOS)lParam)->cx != rWindowSize.right) || (((LPWINDOWPOS)lParam)->cy != rWindowSize.bottom);
                rWindowSize.right = ((LPWINDOWPOS)lParam)->cx;
                rWindowSize.bottom = ((LPWINDOWPOS)lParam)->cy;
            }

            bChanged = bChanged && ((rWindowSize.right * rWindowSize.bottom) != 0);

            if (bChanged) {
                bChangeSizeCalled = TRUE;
                __try {
                    hTmp = hRegion;
                    hRegion = CreateRectRgn(0, 0, rWindowSize.right, rWindowSize.bottom);
                    SetWindowRgn(hwnd, hRegion, TRUE);
                    if (hTmp)
                        DeleteObject(hTmp);
                } __finally {
                    bChangeSizeCalled = FALSE;
                }
            }
        }

        if (!bChanged)
            return DefWindowProc(hwnd, message, wParam, lParam);

        return 0;

    case WM_DESTROY:
        if (hRegion) {
            DeleteObject(hRegion);
            hRegion = 0;
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

VOID CALLBACK TimerProc(HWND hwnd, UINT message, UINT_PTR iTimerID, DWORD dwTime)
{
	// Load our PNG image
	CImage img;
	img.Load(L"time.png");

	// Get dimensions of Image
	int iWidth = img.GetWidth();
	int iHeight = img.GetHeight();

	// Make a memory DC + memory bitmap
	HDC hdcScreen = GetDC(NULL);
	HDC hDC = CreateCompatibleDC(hdcScreen);
	HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, iWidth, iHeight);
	HBITMAP hBmpOld = (HBITMAP)SelectObject(hDC, hBmp);

	// Draw image to memory DC
	img.Draw(hDC, 0, 0, iWidth, iHeight, 0, 0, iWidth, iHeight);

	RECT rtClient;
	GetClientRect(hwnd, &rtClient);

	POINT ptSrc = { 0, 0 };
	POINT ptPos = { rtClient.top, rtClient.top };
	SIZE szSize = { iWidth, iHeight };
	BLENDFUNCTION   blend = { AC_SRC_OVER, 0, 250, AC_SRC_ALPHA };
	UpdateLayeredWindow(hwnd, hdcScreen, &ptPos, &szSize, hDC, &ptSrc, RGB(255, 0, 0), &blend, ULW_ALPHA | ULW_COLORKEY);

	SelectObject(hDC, hBmpOld);
	DeleteObject(hBmp);
	DeleteObject(hDC);
	ReleaseDC(hwnd, hdcScreen);
}