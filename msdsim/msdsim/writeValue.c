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
*  �ۼ� : ������ ���� ������
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
static RECT clientArea; // = { 0, 0, ���ϴ���ǥ, ���ϴ���ǥ }
static BOOL isRunning;
static int  ret;
static char *a_sendBuf; // getValueWindow's argument

/*
���� ��
 - 0: ��� ��ư Ŭ�� (0�� ���� �� ������ 0�� ����)
 - �ڿ���: TPS�� �Է� �� ���� ��ư Ŭ�� (TPS�� ���� �� ������ TPS�� ����)
 - -5: ���ڿ��� �Է� �� ���� ��ư Ŭ�� (-5�� ���� �� ������ ���� �� sendBuf�� ����)
*/
int getValueWindow(HWND parents, char *sendBuf)
{
	HWND hGetValWnd;
	WNDCLASS WndClass = { 0, };//�ʱ�ȭ
	MSG msg;

	isRunning = TRUE;
	owner = parents;
	a_sendBuf = sendBuf;
	ret = 0;
	GetWindowRect(owner, &ownerArea);
	hGetValWnd = makeDefaultWindow(&WndClass, TEXT("inputValueDialog"), TEXT("TPS �Է�"), g_hInst);
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

	// window create => ȭ�� ���̰ų� ��������� ���� ������� ����. ���� �ʿ�
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

		case ID_WV_CCBTN: // OKBTN�� ������ ������ ret�� 0
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
				//}//tpsEdit�� �ԷµǾ��ִ� ����ũ�⸦ EM_LINELENGTH ���Ͽ� 0�̰ų� ������ Ȯ�ι�ư ��Ȱ��ȭ
				//else {
					if (SendMessage(decBtn, BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						tpsEditorCheck();
						SendMessage(tpsEdit, EM_LINELENGTH, 0, 0);
					}
					if (SendMessage(tpsEdit, EM_LINELENGTH, 0, 0) <= 0) {
						EnableWindow(okBtn, FALSE);
					}//0���� ũ�� Ȱ��ȭ
					else {
						EnableWindow(okBtn, TRUE);
					}
				
			}
			break;

		case ID_WV_STRBTN:
			OnStrBtn(hWnd, wParam, lParam);
			SetWindowText(hTmp, TEXT("��"));
			//BOOL SetWindowText(HWND hWnd, LPCSTR lpString);
			//hWnd�� �̸���, lpString���� �ٲٴ� �Լ�
			//��ȯ���� ��ȯ�� �����ϸ� True(1)�� �����ϸ� False(0)�� ��ȯ�Ѵ�.
			//BOOL SetDlgltemText(HWND hDlg,int nIDDlgItem ,LPCTSTR IpString);
			//hDlg	:��Ʈ���� ������ �ִ� �������� �ڵ�
			//nIDDlgItem	:��Ʈ���� ID
			//IpString		:��Ʈ�ѿ� ������ NULL ���� ���ڿ�
			//����	:�����ϸ� 0�� �ƴ� ���� �����ϸ� ���н� 0�� �����Ѵ�.	
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
		VARIABLE_PITCH | FF_ROMAN, TEXT("���� ���"));

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
	decBtn = CreateWindow(TEXT("button"), TEXT("10����"),
		WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON | WS_GROUP,
		x, y, 80, 20,
		hWnd, (HMENU)ID_WV_DECBTN, g_hInst, NULL);
	SendMessage(decBtn, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(decBtn, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
	SendMessage(tpsEdit, EM_LIMITTEXT, (WPARAM)8, 0);

	x = (int)(((clientArea.right / 2) - 100) * 0.8 + (clientArea.right / 2));
	y = (int)((clientArea.bottom - 60) / 2) + 10 + 10;
	strBtn = CreateWindow(TEXT("button"), TEXT("���ڿ�"),
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
		TEXT("button"), TEXT("Ȯ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, w, h,
		hWnd, (HMENU)ID_WV_OKBTN, g_hInst, NULL
	);
	SendMessage(okBtn, WM_SETFONT, (WPARAM)hfont, 0);
	EnableWindow(okBtn, FALSE);//Ȯ�ι�ư ��Ȱ��ȭ
	// ccbutton
	x = (int)((clientArea.right / 2) + ((clientArea.right / 2) - 75) / 2);
	cancleBtn = CreateWindow(
		TEXT("button"), TEXT("���"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
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