/***********************************************************************

			  (c) Copyright 2020 ariatech, Inc.
						  All rights reserved.

   This program contains confidential and proprietary information  of
   ntelia, and any reproduction, disclosure, or use in whole or in part
   is expressly  prohibited, except as may be specifically authorized
   by prior written agreement or permission of ntelia.

************************************************************************

*======================================================================*
*  FILE : main.c
*  작성 : 조대현 연구원 (devismylife@ariatech.kr)
*======================================================================*/

#include "msdsim_c.h"

HINSTANCE g_hInst;
HWND g_hMainWindow;
LPTSTR g_lpszClass = PROGRAM_TITLE;

static HWND makeDefaultWindow(WNDCLASS *pWndClass, LPCTSTR lpszClass, LPCTSTR title, HINSTANCE hInst);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	setlocale(LC_ALL, "ko_KR.UTF-8");
	hWnd = makeDefaultWindow(&WndClass, g_lpszClass, TEXT("통계 시뮬레이터"), g_hInst);
	if (hWnd == NULL)
	{
		MessageBox(NULL, TEXT("윈도우를 생성할 수 없습니다."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}
	else
	{
		g_hMainWindow = hWnd;
	}
	ShowWindow(hWnd, nCmdShow);

	// 메시지 루프
	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

static HWND makeDefaultWindow(WNDCLASS *pWndClass, LPCTSTR lpszClass, LPCTSTR title, HINSTANCE hInst)
{
	HWND hWnd;
	int w, h;
#ifdef DEBUG_MODE
	w = 1200;
	h = 700;
#else
	w = 1200;
	h = 600;
#endif

	// regist class
	pWndClass->cbClsExtra = 0;
	pWndClass->cbWndExtra = 0;
	pWndClass->hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1); //CreateSolidBrush(RGB(240, 240, 240));
	pWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
	pWndClass->hIcon = LoadIcon(NULL, IDI_APPLICATION);
	pWndClass->hInstance = hInst;
	pWndClass->lpfnWndProc = (WNDPROC)WndProc;
	pWndClass->lpszClassName = lpszClass;
	pWndClass->lpszMenuName = NULL;
	pWndClass->style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(pWndClass);

	// window create
	hWnd = CreateWindow(lpszClass, title, WS_OVERLAPPEDWINDOW,
		(GetSystemMetrics(SM_CXSCREEN) - w) / 2, (GetSystemMetrics(SM_CYSCREEN) - h) / 2,
		w, h,
		NULL, (HMENU)NULL, hInst, NULL);

	return hWnd;
}