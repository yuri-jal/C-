/***********************************************************************

			  (c) Copyright 2020 ariatech, Inc.
						  All rights reserved.

   This program contains confidential and proprietary information  of
   ntelia, and any reproduction, disclosure, or use in whole or in part
   is expressly  prohibited, except as may be specifically authorized
   by prior written agreement or permission of ntelia.

************************************************************************

*======================================================================*
*  FILE : msdsim_c.h
*  작성 : 조대현 연구원 (devismylife@ariatech.kr), 김유리 인턴 연구원
*======================================================================*/

// compile directive >>>>>>>>>>
#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "comctl32.lib")
#pragma warning(4 : 4996)
// compile directive <<<<<<<<<<


// include >>>>>>>>>>
#include <tchar.h>
#include <stdio.h>
#include <stdint.h>
#include <locale.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <time.h>
#include <CommCtrl.h>
// <<<<<<<<<< include


// define >>>>>>>>>>
//#define DEBUG_MODE
#define PROGRAM_TITLE    TEXT("통계 시뮬레이터")
#define PROGRAM_TITLEA   "통계 시뮬레이터"
#define PROGRAM_TITLEW   L"통계 시뮬레이터"
#define MAX_IDENTI       (32 + 1)

#define ID_MAIN          100
#define ID_PATH          101
#define ID_MEMBER_LIST   102
#define ID_IDX           103
#define ID_SELECT        104
#define ID_SELECTED_LIST 105
#define ID_DELETE        106
#define ID_SEND          107
#define ID_FIRST         108
#define ID_IP            109
#define ID_IPSTATIC      110
#define ID_PORT          111
#define ID_PORTSTATIC    112
#define ID_CONNECT       113
#define ID_DISCONNECT    114
#define ID_STOP          115
#define ID_SETBOOK       116
#define ID_SETBOOK_H     117
#define ID_SETBOOK_M     118
#define ID_SETBOOK_S     119
#define ID_CANCELBOOK    120
// <<<<<<<<<< define


// typedef >>>>>>>>>>
typedef struct st_Member
{
	TCHAR type[MAX_IDENTI];
	TCHAR name[MAX_IDENTI];
	BOOL isPointer;   // non-pointer type: false
	int  maxArrayIdx; // non-array type: 0
} Member;
// <<<<<<<<<< typedef


// function declare >>>>>>>>>>
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void makeMainWindow(HWND hWnd);
int getToken(char *tokenBuf, const BYTE *arBuf);
void tcstombs(char *dest, TCHAR *origin);
int recvData(SOCKET sock, BYTE *buf);
void mbstotcs(TCHAR *dest, char *origin);
int createMemberList(Member **dest, const BYTE *arBuf);
void updateMemberList(const Member *arMem, int com);
void deleteMemberList(Member **p);
int getValueWindow(HWND parents, char *sendBuf);
void updateSelectedList(char *buf);
int deleteSelected(void);
void connectUI(void);
void disconnectUI(void);
void clearSelectedList(void);
void tpsSendUI(HWND hWnd);
void tpsStopUI(HWND hWnd);
void startBookEndUI(void);
void cancelBookEndUI(void);
void debugf(BOOL isClear, const TCHAR *format, ...);
// <<<<<<<<<< function declare

// extern g_var >>>>>>>>>>
extern HINSTANCE g_hInst;
extern LPTSTR g_lpszClass;
extern HWND g_hMainWindow;
extern HWND g_hPath;
extern HWND g_hMemberList;
extern HWND g_hIdx;
extern HWND g_hIndexStatic;
extern HWND g_hSelect;
extern HWND g_hSelectedList;
extern HWND g_hRemove;
extern HWND g_hSend;
extern HWND g_hFirst;
extern HWND g_hIp;
extern HWND g_hPort;
extern HWND g_hConnect;
extern HWND g_hDisConnect;
extern HWND g_hIpStatic;
extern HWND g_hPortStatic;
extern Member *arMem;
extern HWND g_hNotiStatic;
extern HWND g_hStartTime, g_hStopwatch;
extern HWND g_hBookEndTime;
extern HWND g_hSetBookEndTime;
extern HWND g_hCancelBookEndTime;
extern HWND g_hBookEndTime_Hour;
extern HWND g_hBookEndTime_Min;
extern HWND g_hBookEndTime_Sec;
extern HWND g_hStop;
extern HWND g_hDebugStatic;
// <<<<<<<<<< extern g_var