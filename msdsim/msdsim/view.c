/***********************************************************************

			  (c) Copyright 2020 ariatech, Inc.
						  All rights reserved.

   This program contains confidential and proprietary information  of
   ntelia, and any reproduction, disclosure, or use in whole or in part
   is expressly  prohibited, except as may be specifically authorized
   by prior written agreement or permission of ntelia.

************************************************************************

*======================================================================*
*  FILE : view.c
*  작성 : 조대현 연구원 (devismylife@ariatech.kr)
*======================================================================*/

#include "msdsim_c.h"

HWND g_hPath;
HWND g_hMemberList;
HWND g_hIdx;
HWND g_hIndexStatic;
HWND g_hSelect;
HWND g_hSelectedList;
HWND g_hRemove;
HWND g_hSend;
HWND g_hFirst;
HWND g_hIp;
HWND g_hPort;
HWND g_hConnect;
HWND g_hDisConnect;
static HWND g_hSelectVarStatic;
HWND g_hIpStatic;
HWND g_hPortStatic;
HWND g_hNotiStatic;
HWND g_hStop;
HWND g_hSendRunTimeDescriptorStatic;
HWND g_hSendRunTimeStatic;
HWND g_hStartTime, g_hStopwatch;
static HWND g_hStartTimeDescriptor, g_hStopwatchDescriptor;
HWND g_hBookEndTime;
HWND g_hSetBookEndTime;
HWND g_hCancelBookEndTime;
HWND g_hBookEndTime_Hour;
HWND g_hBookEndTime_Min;
HWND g_hBookEndTime_Sec;
HWND g_hDebugStatic;
static HWND g_hBookEndTime_HourStatic;
static HWND g_hBookEndTime_MinStatic;
static HWND g_hBookEndTime_SecStatic;

static void makeMemberListWindow(HWND hWnd);
static void makeSelectedListWindow(HWND hWnd);
static void makeConnectWindow(HWND hWnd);
static void debugWindow(HWND hWnd);
static void setFont(void);

void makeMainWindow(HWND hWnd)
{
	makeMemberListWindow(hWnd);
	makeSelectedListWindow(hWnd);
	makeConnectWindow(hWnd);
	debugWindow(hWnd);
	tpsStopUI(hWnd);
	//tpsSendUI(hWnd); // TEST
	setFont();
}

static void makeMemberListWindow(HWND hWnd)
{
	g_hPath = CreateWindow(
		TEXT("static"), TEXT("PATH: "), WS_CHILD | WS_VISIBLE,
		10, 70, 500, 20,
		hWnd, (HMENU)ID_PATH, g_hInst, NULL
	);

	g_hMemberList = CreateWindow(
		TEXT("listbox"), NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL |
		LBS_SORT | LBS_NOTIFY /*| LBS_DISABLENOSCROLL*/ | LBS_USETABSTOPS,
		10, 90, 575, 400,
		hWnd, (HMENU)ID_MEMBER_LIST, g_hInst, NULL
	);

	g_hIndexStatic = CreateWindow(
		TEXT("static"), TEXT("Index"), WS_CHILD | WS_VISIBLE,
		10, 510, 50, 20,
		hWnd, (HMENU)-1, g_hInst, NULL
	);
	EnableWindow(g_hIndexStatic, FALSE); // 배열 변수를 택했을 때만 활성화

	g_hIdx = CreateWindow(
		TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 
		70, 508, 150, 25, 
		hWnd, (HMENU)ID_IDX, g_hInst, NULL
	);
	SendMessage(g_hIdx, EM_LIMITTEXT, (WPARAM)8, 0); // 최대 8글자만 입력 가능
	EnableWindow(g_hIdx, FALSE); // 배열 변수를 택했을 때만 활성화

	g_hSelect = CreateWindow(
		TEXT("button"), TEXT("선택"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
		480, 508, 100, 25, 
		hWnd, (HMENU)ID_SELECT, g_hInst, NULL
	);

	g_hFirst = CreateWindow(
		TEXT("button"), TEXT("⮅"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, //| BS_OWNERDRAW,
		565, 70, 20, 20,
		hWnd, (HMENU)ID_FIRST, g_hInst, NULL
	);

	g_hNotiStatic = CreateWindow(
		TEXT("static"), TEXT(" "), WS_CHILD | WS_VISIBLE, 
		10, 470, 500, 20,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hStartTimeDescriptor = CreateWindow(
		TEXT("static"), TEXT("시작 시간"), WS_CHILD | WS_VISIBLE,
		50, 150, 150, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hStartTime = CreateWindow(
		TEXT("static"), TEXT("0일 전 00시 00분 00초"), WS_CHILD | WS_VISIBLE,
		230, 150, 300, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hStopwatchDescriptor = CreateWindow(
		TEXT("static"), TEXT("경과 시간"), WS_CHILD | WS_VISIBLE,
		50, 250, 150, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hStopwatch = CreateWindow(
		TEXT("static"), TEXT("00시간 00분 00초"), WS_CHILD | WS_VISIBLE,
		230, 250, 300, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hBookEndTime = CreateWindow(
		TEXT("static"), TEXT("남은 시간"), WS_CHILD | WS_VISIBLE,
		50, 350, 150, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hSetBookEndTime = CreateWindow(
		TEXT("button"), TEXT("설정"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		505, 355, 50, 30,
		hWnd, (HMENU)ID_SETBOOK, g_hInst, NULL
	);

	g_hCancelBookEndTime = CreateWindow(
		TEXT("button"), TEXT("해제"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		505, 355, 50, 30,
		hWnd, (HMENU)ID_CANCELBOOK, g_hInst, NULL
	);

	g_hBookEndTime_Hour = CreateWindow(
		TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
		230, 355, 30, 30,
		hWnd, (HMENU)ID_SETBOOK_H, g_hInst, NULL
	);
	SendMessage(g_hBookEndTime_Hour, EM_LIMITTEXT, (WPARAM)2, 0); // 최대 2글자만 입력 가능

	g_hBookEndTime_HourStatic = CreateWindow(
		TEXT("static"), TEXT("시간"), WS_CHILD | WS_VISIBLE,
		265, 350, 75, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);
	
	g_hBookEndTime_Min = CreateWindow(
		TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
		345, 355, 30, 30,
		hWnd, (HMENU)ID_SETBOOK_M, g_hInst, NULL
	);
	SendMessage(g_hBookEndTime_Min, EM_LIMITTEXT, (WPARAM)2, 0); // 최대 2글자만 입력 가능

	g_hBookEndTime_MinStatic = CreateWindow(
		TEXT("static"), TEXT("분"), WS_CHILD | WS_VISIBLE,
		380, 350, 40, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hBookEndTime_Sec = CreateWindow(
		TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
		425, 355, 30, 30,
		hWnd, (HMENU)ID_SETBOOK_S, g_hInst, NULL
	);
	SendMessage(g_hBookEndTime_Sec, EM_LIMITTEXT, (WPARAM)2, 0); // 최대 2글자만 입력 가능

	g_hBookEndTime_SecStatic = CreateWindow(
		TEXT("static"), TEXT("초"), WS_CHILD | WS_VISIBLE,
		460, 350, 40, 50,
		hWnd, (HMENU)-1, g_hInst, NULL
	);
}

static void makeSelectedListWindow(HWND hWnd)
{
	g_hSelectedList = CreateWindow(
		TEXT("listbox"), NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL |
		LBS_NOTIFY /*| LBS_DISABLENOSCROLL*/ | LBS_USETABSTOPS,
		600, 90, 575, 400,
		hWnd, (HMENU)ID_SELECTED_LIST, g_hInst, NULL
	);

	g_hSelectVarStatic = CreateWindow(
		TEXT("static"), TEXT("선택된 변수"), WS_CHILD | WS_VISIBLE,
		600, 70, 100, 20,
		hWnd, (HMENU)-1, g_hInst, NULL
	);

	g_hRemove = CreateWindow(
		TEXT("button"), TEXT("삭제"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		600, 508, 100, 25,
		hWnd, (HMENU)ID_DELETE, g_hInst, NULL
	);

	g_hSend = CreateWindow(
		TEXT("button"), TEXT("TPS 전송"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		1070, 508, 100, 25,
		hWnd, (HMENU)ID_SEND, g_hInst, NULL
	);

	g_hStop = CreateWindow(
		TEXT("button"), TEXT("전송 중지"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		1070, 508, 100, 25,
		hWnd, (HMENU)ID_STOP, g_hInst, NULL
	);
}

static void setFont(void)
{
	HFONT hfont;

	// default 맑은고딕 적용
	hfont = CreateFont(18, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));
	SendMessage(g_hPath, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hIdx, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hSelect, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hRemove, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hSend, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hSelectVarStatic, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hIndexStatic, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hFirst, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hIpStatic, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hIp, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hPortStatic, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hPort, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hConnect, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hDisConnect, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hNotiStatic, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hStop, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hSetBookEndTime, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hCancelBookEndTime, WM_SETFONT, (WPARAM)hfont, 0);
	
	// ListBox 한정 consolas체 적용
	hfont = CreateFont(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0,
		FIXED_PITCH | FF_DONTCARE, TEXT("Consolas"));
	SendMessage(g_hMemberList, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hSelectedList, WM_SETFONT, (WPARAM)hfont, 0);

	// 타이머 기술자 한정 37 point 글씨체 적용
	hfont = CreateFont(37, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0,
		FIXED_PITCH | FF_DONTCARE, TEXT("맑은 고딕"));
	SendMessage(g_hStartTimeDescriptor, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hStopwatchDescriptor, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hBookEndTime, WM_SETFONT, (WPARAM)hfont, 0);

	// 타이머 IDX 한정 30 point 글씨체 적용
	hfont = CreateFont(25, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0,
		FIXED_PITCH | FF_DONTCARE, TEXT("맑은 고딕"));
	SendMessage(g_hBookEndTime_Hour, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hBookEndTime_Min, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hBookEndTime_Sec, WM_SETFONT, (WPARAM)hfont, 0);

	// 타이머 한정 40 point 글씨체 적용
	hfont = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0,
		FIXED_PITCH | FF_DONTCARE, TEXT("맑은 고딕"));
	SendMessage(g_hStartTime, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hStopwatch, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hBookEndTime_HourStatic, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hBookEndTime_MinStatic, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hBookEndTime_SecStatic, WM_SETFONT, (WPARAM)hfont, 0);
}

static void makeConnectWindow(HWND hWnd)
{
	g_hIpStatic = CreateWindow(
		TEXT("static"), TEXT("IP"), WS_CHILD | WS_VISIBLE,
		10, 20, 20, 22,
		hWnd, (HMENU)ID_IPSTATIC, g_hInst, NULL
	);

	g_hIp = CreateWindow(
		TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		40, 20, 150, 22,
		hWnd, (HMENU)ID_IP, g_hInst, NULL
	);

	g_hPortStatic = CreateWindow(
		TEXT("static"), TEXT("PORT"), WS_CHILD | WS_VISIBLE | SS_CENTER,
		200, 20, 50, 22,
		hWnd, (HMENU)ID_PORTSTATIC, g_hInst, NULL
	);

	g_hPort = CreateWindow(
		TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		250, 20, 50, 22,
		hWnd, (HMENU)ID_PORT, g_hInst, NULL
	);

	g_hConnect = CreateWindow(
		TEXT("button"), TEXT("연결"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		310, 20, 50, 22,
		hWnd, (HMENU)ID_CONNECT, g_hInst, NULL
	);

	g_hDisConnect = CreateWindow(
		TEXT("button"), TEXT("연결 해제"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		310, 20, 75, 22,
		hWnd, (HMENU)ID_DISCONNECT, g_hInst, NULL
	);

	// 테스트 주소
	//SetWindowText(g_hIp, TEXT("172.30.49.168")); // 마곡 게이트웨이
	SetWindowText(g_hIp, TEXT("172.16.53.117")); // 본사 IoT
	SetWindowText(g_hPort, TEXT("9000"));
}

void connectUI(void)
{
	HWND *enableList[] = { &g_hPath, &g_hMemberList, &g_hSelect,
		&g_hSelectedList, &g_hRemove, &g_hSend, &g_hFirst, &g_hSelectVarStatic };
	HWND *disableList[] = { &g_hIp, &g_hIpStatic, &g_hPort, &g_hPortStatic };
	int i;

	for (i = 0; i < sizeof(enableList) / sizeof(enableList[0]); i++)
	{
		EnableWindow(*enableList[i], TRUE);
	}
	for (i = 0; i < sizeof(disableList) / sizeof(disableList[0]); i++)
	{
		EnableWindow(*disableList[i], FALSE);
	}
	ShowWindow(g_hDisConnect, TRUE);
	ShowWindow(g_hConnect, FALSE);
}

void disconnectUI(void)
{
	HWND *enableList[] = { &g_hIp, &g_hIpStatic, &g_hPort, &g_hPortStatic };
	HWND *disableList[] = { &g_hPath, &g_hMemberList, &g_hIdx, &g_hIndexStatic, &g_hSelect, 
		&g_hSelectedList, &g_hRemove, &g_hSend, &g_hFirst, &g_hSelectVarStatic };
	int i;

	for (i = 0; i < sizeof(enableList) / sizeof(enableList[0]); i++)
	{
		EnableWindow(*enableList[i], TRUE);
	}
	for (i = 0; i < sizeof(disableList) / sizeof(disableList[0]); i++)
	{
		EnableWindow(*disableList[i], FALSE);
	}
	ShowWindow(g_hDisConnect, FALSE);
	ShowWindow(g_hConnect, TRUE);
	clearSelectedList();
}

void tpsSendUI(HWND hWnd)
{
	HWND *disableList[] = { &g_hDisConnect, &g_hPath, &g_hMemberList, &g_hSelect,
		&g_hRemove, &g_hNotiStatic, &g_hIndexStatic, &g_hSelectVarStatic };
	HWND *hideList[] = { &g_hPath, &g_hMemberList, &g_hSelect, &g_hIndexStatic, &g_hIdx, 
		&g_hSend, &g_hFirst, &g_hNotiStatic, &g_hRemove };
	HWND *showList[] = { &g_hStop, &g_hStartTime, &g_hStartTimeDescriptor, 
		&g_hStopwatch, &g_hStopwatchDescriptor, &g_hBookEndTime, &g_hSetBookEndTime,
		&g_hBookEndTime_Hour, &g_hBookEndTime_HourStatic, &g_hBookEndTime_Min,
		&g_hBookEndTime_MinStatic, &g_hBookEndTime_Sec, &g_hBookEndTime_SecStatic,
		&g_hCancelBookEndTime, &g_hSetBookEndTime };
	DWORD nowStyle = GetClassLong(hWnd, GCL_STYLE);
	int i = 0;
	
	// 윈도우 Disable
	for (i = 0; i < sizeof(disableList) / sizeof(disableList[0]); i++)
	{
		EnableWindow(*disableList[i], FALSE);
	}

	// UI 구성 변경 
	// 멤버 리스트, TPS 전송 버튼 등 숨김
	for (i = 0; i < sizeof(hideList) / sizeof(hideList[0]); i++)
	{
		ShowWindow(*hideList[i], FALSE);
	}
	// 실행 시간, TPS 전송 수, 전송 중단 버튼 등 숨김
	for (i = 0; i < sizeof(showList) / sizeof(showList[0]); i++)
	{
		ShowWindow(*showList[i], TRUE);
	}
	// X버튼 비활성화
	SetClassLong(hWnd, GCL_STYLE, nowStyle | CS_NOCLOSE);
}

void tpsStopUI(HWND hWnd)
{
	HWND *enableList[] = { &g_hDisConnect, &g_hPath, &g_hMemberList, &g_hSelect,
		&g_hRemove, &g_hNotiStatic, &g_hIndexStatic, &g_hSelectVarStatic };
	HWND *hideList[] = { &g_hStop, &g_hStartTime, &g_hStartTimeDescriptor,
		&g_hStopwatch, &g_hStopwatchDescriptor, &g_hBookEndTime, &g_hSetBookEndTime,
		&g_hBookEndTime_Hour, &g_hBookEndTime_HourStatic, &g_hBookEndTime_Min,
		&g_hBookEndTime_MinStatic, &g_hBookEndTime_Sec, &g_hBookEndTime_SecStatic,
		&g_hCancelBookEndTime, &g_hSetBookEndTime };
	HWND *showList[] = { &g_hPath, &g_hMemberList, &g_hSelect, &g_hIndexStatic, &g_hIdx,
		&g_hSend, &g_hFirst, &g_hNotiStatic, &g_hRemove };
	DWORD nowStyle = GetClassLong(hWnd, GCL_STYLE);
	int i;

	// 윈도우 Enable
	for (i = 0; i < sizeof(enableList) / sizeof(enableList[0]); i++)
	{
		EnableWindow(*enableList[i], TRUE);
	}

	// UI 구성 변경 
	// 멤버 리스트, TPS 전송 버튼 등 숨김
	for (i = 0; i < sizeof(hideList) / sizeof(hideList[0]); i++)
	{
		ShowWindow(*hideList[i], FALSE);
	}
	// 실행 시간, TPS 전송 수, 전송 중단 버튼 등 숨김
	for (i = 0; i < sizeof(showList) / sizeof(showList[0]); i++)
	{
		ShowWindow(*showList[i], TRUE);
	}
	// X버튼 활성화
	SetClassLong(hWnd, GCL_STYLE, nowStyle & ~(CS_NOCLOSE));
}

void startBookEndUI(void)
{
	HWND *disableList[] = { &g_hBookEndTime_Hour, &g_hBookEndTime_Min, &g_hBookEndTime_Sec };
	HWND *hideList[] = { &g_hSetBookEndTime };
	HWND *showList[] = { &g_hCancelBookEndTime };
	int i;

	// 윈도우 Disable
	for (i = 0; i < sizeof(disableList) / sizeof(disableList[0]); i++)
	{
		EnableWindow(*disableList[i], FALSE);
	}

	// Hide
	for (i = 0; i < sizeof(hideList) / sizeof(hideList[0]); i++)
	{
		ShowWindow(*hideList[i], FALSE);
	}
	// Show
	for (i = 0; i < sizeof(showList) / sizeof(showList[0]); i++)
	{
		ShowWindow(*showList[i], TRUE);
	}
}

void cancelBookEndUI(void)
{
	HWND *enableList[] = { &g_hBookEndTime_Hour, &g_hBookEndTime_Min, &g_hBookEndTime_Sec };
	HWND *showList[] = { &g_hSetBookEndTime };
	HWND *hideList[] = { &g_hCancelBookEndTime };
	int i;

	// 윈도우 Enable
	for (i = 0; i < sizeof(enableList) / sizeof(enableList[0]); i++)
	{
		EnableWindow(*enableList[i], TRUE);
	}

	// Hide
	for (i = 0; i < sizeof(hideList) / sizeof(hideList[0]); i++)
	{
		ShowWindow(*hideList[i], FALSE);
	}
	// Show
	for (i = 0; i < sizeof(showList) / sizeof(showList[0]); i++)
	{
		ShowWindow(*showList[i], TRUE);
	}

	// Edit Clear
	SetWindowText(g_hBookEndTime_Hour, TEXT("\0"));
	SetWindowText(g_hBookEndTime_Min, TEXT("\0"));
	SetWindowText(g_hBookEndTime_Sec, TEXT("\0"));
}

static void debugWindow(HWND hWnd)
{
#ifdef DEBUG_MODE
	HFONT hfont = CreateFont(13, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));
	HWND hTmp;

	hTmp = CreateWindow(
		TEXT("static"), TEXT("Debug Mode On"), WS_CHILD | WS_VISIBLE,
		10, 601, 1180, 15,
		hWnd, (HMENU)ID_PATH, g_hInst, NULL
	);
	g_hDebugStatic = CreateWindow(
		TEXT("static"), TEXT(""), WS_CHILD | WS_VISIBLE,
		10, 625, 1180, 66,
		hWnd, (HMENU)ID_PATH, g_hInst, NULL
	);
	SendMessage(hTmp, WM_SETFONT, (WPARAM)hfont, 0);
	SendMessage(g_hDebugStatic, WM_SETFONT, (WPARAM)hfont, 0);
#endif
}