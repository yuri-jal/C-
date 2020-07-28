/***********************************************************************

			  (c) Copyright 2020 ariatech, Inc.
						  All rights reserved.

   This program contains confidential and proprietary information  of
   ntelia, and any reproduction, disclosure, or use in whole or in part
   is expressly  prohibited, except as may be specifically authorized
   by prior written agreement or permission of ntelia.

************************************************************************

*======================================================================*
*  FILE : writeValue.c
*  작성 : 김유리 인턴 연구원
*======================================================================*/

#include "msdsim_c.h"

#define ID_WV_OKBTN   201
#define ID_WV_CCBTN   202
#define ID_WV_TPSEDIT 203
#define ID_WV_STRBTN  204
#define ID_WV_DECBTN  205

static HWND makeDefaultWindow(WNDCLASS *pWndClass, LPCTSTR lpszClass, LPCTSTR title, HINSTANCE hInst);
static LRESULT CALLBACK writeWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
static int OnCtlColor(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnStrBtn(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT OnDecBtn(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnOkBtn(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void tpsEditorCheck(void);

static HWND owner, tpsEdit, okBtn, cancleBtn;
static HWND strBtn, decBtn;
HWND hTmp;
static RECT ownerArea;
static RECT clientArea; // = { 0, 0, 우하단좌표, 우하단좌표 }
static BOOL isRunning;
static int  ret;
static char *a_sendBuf; // getValueWindow's argument

/*
리턴 값
 - 0: 취소 버튼 클릭 (0을 리턴 후 서버에 0을 전송)
 - 자연수: TPS를 입력 후 전송 버튼 클릭 (TPS를 리턴 후 서버에 TPS를 전송)
 - -5: 문자열을 입력 후 전송 버튼 클릭 (-5을 리턴 후 서버에 길이 및 sendBuf를 전송)
*/
int getValueWindow(HWND parents, char *sendBuf)
{
	HWND hGetValWnd;
	WNDCLASS WndClass = { 0, };//초기화
	MSG msg;

	isRunning = TRUE;
	owner = parents;
	a_sendBuf = sendBuf;
	ret = 0;
	GetWindowRect(owner, &ownerArea);
	hGetValWnd = makeDefaultWindow(&WndClass, TEXT("inputValueDialog"), TEXT("TPS 입력"), g_hInst);
	ShowWindow(hGetValWnd, TRUE);
	EnableWindow(owner, FALSE);

	// Message pumping
	while (isRunning)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Sleep(50);
		}
	}

	return ret;
}

static HWND makeDefaultWindow(WNDCLASS *pWndClass, LPCTSTR lpszClass, LPCTSTR title, HINSTANCE hInst)
{
	HWND hWnd;
	int x, y;
	int w = 300, h = 150;

	// regist class
	pWndClass->cbClsExtra = 0;
	pWndClass->cbWndExtra = 0;
	pWndClass->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	pWndClass->hCursor = LoadCursor(NULL, IDC_ARROW);
	pWndClass->hIcon = LoadIcon(NULL, IDI_APPLICATION);
	pWndClass->hInstance = hInst;
	pWndClass->lpfnWndProc = (WNDPROC)writeWndProc;
	pWndClass->lpszClassName = lpszClass;
	pWndClass->lpszMenuName = NULL;
	//pWndClass->style = CS_NOCLOSE;
	RegisterClass(pWndClass);

	// (x, y) calc
	x = (ownerArea.right - ownerArea.left - w) / 2;
	y = (ownerArea.bottom - ownerArea.top - h) / 2;

	// window create => 화면 밖이거나 듀얼모니터의 경우는 고려하지 않음. 개선 필요
	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, lpszClass, title, WS_OVERLAPPEDWINDOW | WS_POPUP,
		ownerArea.left + x, ownerArea.top + y, w, h,
		owner, (HMENU)NULL, hInst, NULL);
\

	return hWnd;
}

static LRESULT CALLBACK writeWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_CREATE:
		return OnCreate(hWnd, wParam, lParam);

	case WM_CTLCOLORSTATIC:
		return OnCtlColor(hWnd, wParam, lParam);

	case WM_CLOSE:
		SendMessage(hWnd, WM_COMMAND, MAKEWORD(ID_WV_CCBTN, 0), 0);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_WV_OKBTN:
			ret = OnOkBtn(hWnd, wParam, lParam);
			// not break

		case ID_WV_CCBTN: // OKBTN이 눌리지 않으면 ret은 0
			EnableWindow(owner, TRUE);
			isRunning = FALSE;
			DestroyWindow(hWnd);
			break;

		case ID_WV_TPSEDIT:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				//if (SendMessage(tpsEdit, EM_LINELENGTH, 0, 0) <= 0) {
					//EnableWindow(okBtn, FALSE);
				//}//tpsEdit에 입력되어있는 글자크기를 EM_LINELENGTH 비교하여 0이거나 작으면 확인버튼 비활성화
				//else {
					if (SendMessage(decBtn, BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						tpsEditorCheck();
						SendMessage(tpsEdit, EM_LINELENGTH, 0, 0);
					}
					if (SendMessage(tpsEdit, EM_LINELENGTH, 0, 0) <= 0) {
						EnableWindow(okBtn, FALSE);
					}//0보다 크면 활성화
					else {
						EnableWindow(okBtn, TRUE);
					}
				
			}
			break;

		case ID_WV_STRBTN:
			OnStrBtn(hWnd, wParam, lParam);
			SetWindowText(hTmp, TEXT("값"));
			//BOOL SetWindowText(HWND hWnd, LPCSTR lpString);
			//hWnd의 이름을, lpString으로 바꾸는 함수
			//반환값은 변환에 성공하면 True(1)을 실패하면 False(0)을 반환한다.
			//BOOL SetDlgltemText(HWND hDlg,int nIDDlgItem ,LPCTSTR IpString);
			//hDlg	:컨트롤을 가지고 있는 윈도우의 핸들
			//nIDDlgItem	:컨트롤의 ID
			//IpString		:컨트롤에 설정할 NULL 종료 문자열
			//리턴	:성공하면 0이 아닌 값을 리턴하며 실패시 0을 리턴한다.	
			break;

		case ID_WV_DECBTN:
			OnDecBtn(hWnd, wParam, lParam);
			SetWindowText(hTmp, TEXT("TPS"));
			break;
		}
		return 0;

	case WM_DESTROY:
		EnableWindow(owner, TRUE);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

static LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int x, y, w, h;

	HFONT hfont;

	GetClientRect(hWnd, &clientArea);

	// create font
	hfont = CreateFont(18, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));

	// static
	x = 10;
	w = (int)((clientArea.right - 20) * 0.33 - 10);
	y = 10;
	h = (int)((clientArea.bottom - 60) / 2);
	hTmp = CreateWindow(
		TEXT("static"), TEXT("TPS"), WS_CHILD | WS_VISIBLE | SS_CENTER,
		x, y, w, h,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	SendMessage(hTmp, WM_SETFONT, (WPARAM)hfont, 0);


	// edit
	x = (int)((clientArea.right - 20) * 0.33);
	w = (int)((clientArea.right - 20) * 0.66 - 10);
	tpsEdit = CreateWindow(
		TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		x, y, w, h,
		hWnd, (HMENU)ID_WV_TPSEDIT, g_hInst, NULL
	);
	SendMessage(tpsEdit, WM_SETFONT, (WPARAM)hfont, 0);

	// Radio Button
	x = (int)(((clientArea.right / 2) - 100) * 0.8);
	y = (int)((clientArea.bottom - 60) / 2) + 10 + 10;
	decBtn = CreateWindow(TEXT("button"), TEXT("10진수"),
		WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON | WS_GROUP,
		x, y, 80, 20,
		hWnd, (HMENU)ID_WV_DECBTN, g_hInst, NULL);
	SendMessage(decBtn, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(decBtn, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
	SendMessage(tpsEdit, EM_LIMITTEXT, (WPARAM)8, 0);

	x = (int)(((clientArea.right / 2) - 100) * 0.8 + (clientArea.right / 2));
	y = (int)((clientArea.bottom - 60) / 2) + 10 + 10;
	strBtn = CreateWindow(TEXT("button"), TEXT("문자열"),
		WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
		x, y, 80, 20,
		hWnd, (HMENU)ID_WV_STRBTN, g_hInst, NULL);
	SendMessage(strBtn, WM_SETFONT, (WPARAM)hfont, 0);

	// okbutton
	x = (clientArea.right / 2) - 75;
	w = 75;
	y = clientArea.bottom - 30;
	h = 25;
	okBtn = CreateWindow(
		TEXT("button"), TEXT("확인"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, w, h,
		hWnd, (HMENU)ID_WV_OKBTN, g_hInst, NULL
	);
	SendMessage(okBtn, WM_SETFONT, (WPARAM)hfont, 0);
	EnableWindow(okBtn, FALSE);//확인버튼 비활성화
	// ccbutton
	x = (int)((clientArea.right / 2) + ((clientArea.right / 2) - 75) / 2);
	cancleBtn = CreateWindow(
		TEXT("button"), TEXT("취소"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, w, h,
		hWnd, (HMENU)ID_WV_CCBTN, g_hInst, NULL
	);
	SendMessage(cancleBtn, WM_SETFONT, (WPARAM)hfont, 0);

	return 0;
}

static int OnCtlColor(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND hTargetWnd = (HWND)lParam;
	HDC hTargetDC = (HDC)wParam;

	SetBkMode(hTargetDC, TRANSPARENT);
	return (BOOL)CreateSolidBrush(RGB(255, 255, 255));
}

static void tpsEditorCheck(void)
{
	TCHAR buf[256], *p;
	int tps;

	GetWindowText(tpsEdit, buf, 256);
	for (p = buf; *p; p++)
	{
		if (*p < TEXT('0') || *p > TEXT('9'))
		{
			*p = TEXT('\0');
			SetWindowText(tpsEdit, buf);
			break;
		}
	}

	if (p - buf > 8)
	{
		buf[8] = L'\0';
		SetWindowText(tpsEdit, buf);
	}
	tps = _ttoi(buf);
	if (p != buf && tps < 1)
	{
		SetWindowText(tpsEdit, TEXT("1"));
	}
}

static LRESULT OnStrBtn(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SendMessage(decBtn, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
	SendMessage(strBtn, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
	SendMessage(tpsEdit, EM_LIMITTEXT, (WPARAM)32, 0);
	return 0;
}

static LRESULT OnDecBtn(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	tpsEditorCheck();
	SendMessage(strBtn, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
	SendMessage(decBtn, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
	SendMessage(tpsEdit, EM_LIMITTEXT, (WPARAM)8, 0);
	return 0;
}

static int OnOkBtn(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[256] = TEXT("");
	char ansiBuf[256] = "";
	int len, res = 0;

	GetWindowText(tpsEdit, buf, 255);
	if (SendMessage(decBtn, BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		*a_sendBuf = '\0';
		res = _ttoi(buf);
	}
	else
	{
		tcstombs(ansiBuf, buf);
		len = strlen(ansiBuf);
		*(int32_t *)(char *)a_sendBuf = len;
		strcpy(&a_sendBuf[sizeof(int32_t)], ansiBuf);
		res = -5;
	}

	return res;
}