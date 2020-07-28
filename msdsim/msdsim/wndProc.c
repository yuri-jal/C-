/***********************************************************************

			  (c) Copyright 2020 ariatech, Inc.
						  All rights reserved.

   This program contains confidential and proprietary information  of
   ntelia, and any reproduction, disclosure, or use in whole or in part
   is expressly  prohibited, except as may be specifically authorized
   by prior written agreement or permission of ntelia.

************************************************************************

*======================================================================*
*  FILE : wndProc.c
*  작성 : 조대현 연구원 (devismylife@ariatech.kr), 김유리 인턴 연구원
*======================================================================*/

#include "msdsim_c.h"

static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnCtlColorStatic(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnCtlColorBtn(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void toggleEnableIndexTextBox(int index);
static BOOL sendSelectMember(HWND hWnd);
static void indexEditorCheck(void);
static BOOL isInvalidIp(HWND ipControl);
static BOOL isInvalidPort(HWND portControl);
static void connectSuccess(void);
static void disconnectSuccess(void);
static void sendTPS(HWND hWnd);
static void stopTPS(HWND hWnd);
static DWORD __stdcall sendTPSProc(void *arg);
static void timerEditorCheck(WORD id);
static void startBookEnd();
static void cancelBookEnd();

static SOCKET sock;
static HBRUSH hWhiteBrush;
static BOOL isConnect;
static HANDLE hTpsSendThread;
static BOOL isTpsSending = FALSE;
static BOOL isBookEnd = FALSE;
Member *arMem;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_CREATE:
		return OnCreate(hWnd, wParam, lParam);

	case WM_COMMAND:
		return OnCommand(hWnd, wParam, lParam);

	case WM_CTLCOLORSTATIC:
		return OnCtlColorStatic(hWnd, wParam, lParam);

	case WM_CTLCOLORBTN:
		//return OnCtlColorBtn(hWnd, wParam, lParam);
		return 0;

	case WM_CLOSE:
		if (OnClose(hWnd, wParam, lParam)) break;
		else return 0;

	case WM_DESTROY:
		return OnDestroy(hWnd, wParam, lParam);
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WSADATA wsaData;

	// Window 초기화
	makeMainWindow(hWnd);

	// 윈속 초기화
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	// 브러시 생성
	hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));

	// 커넥트 되지 않은 상태의 UI로 세팅
	disconnectUI();

	return 0;
}

static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	char recvbuf[2048] = "";
	char sendbuf[2048] = "";
	TCHAR tbuf[256] = TEXT("");
	struct sockaddr_in addr; // 소켓 어드레스 구조체
	int recvLen = 0;
	int32_t deadCode = -4;

	switch (LOWORD(wParam))
	{
	case ID_SELECT:
	{
		int n = SendMessage(g_hMemberList, LB_GETCURSEL, 0, 0); // (by 김유리)
		SetWindowText(g_hNotiStatic, TEXT(" "));

		if (n < 0) // (by 김유리)
		{
			debugf(TRUE, TEXT("if NOK"));
			MessageBox(hWnd, TEXT("선택한 변수가 없습니다"), PROGRAM_TITLE, MB_OK | MB_ICONWARNING);
		}
		else
		{
			if (sendSelectMember(hWnd))
			{
				SendMessage(g_hIdx, EM_SETSEL, 0, 8);
				SendMessage(g_hIdx, WM_CLEAR, 0, 0);
				deleteMemberList(&arMem);

				if (recvData(sock, recvbuf)) createMemberList(&arMem, recvbuf);
				else // 마지막 항목에 도달했을 때
				{
					volatile int32_t value = getValueWindow(hWnd, sendbuf);

					if (value == -5)
					{
						send(sock, (char *)&value, sizeof(int32_t), 0); // -5전송 후
						send(sock, sendbuf, sizeof(int32_t) + strlen(&sendbuf[sizeof(int32_t)]), 0); // 길이+내용 전송
					}
					else
					{
						send(sock, (char *)&value, sizeof(int32_t), 0); // TPS 전송
					}

					if (value != 0) // 0이면 등록을 취소함
					{
						recvData(sock, recvbuf); // 대입 코드 수신
						updateSelectedList(&recvbuf[1]); // @는 제외하고 등록
					}
					// 이후 arMem을 해제한 뒤, WM_CREATE에서 작성한 최초 수신 로직 재시작
					deleteMemberList(&arMem);
					recvData(sock, recvbuf);
					createMemberList(&arMem, recvbuf);
				}
			}
		}
		break;
	}

	case ID_MEMBER_LIST:
		switch (HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			toggleEnableIndexTextBox(SendMessage(g_hMemberList, LB_GETCURSEL, 0, 0));
			break;

		case LBN_DBLCLK:
			SendMessage(g_hSelect, WM_LBUTTONDOWN, (WPARAM)0, (LPARAM)0);
			SendMessage(g_hSelect, WM_LBUTTONUP, (WPARAM)0, (LPARAM)0);
			break;
		}
		break;

	case ID_IDX:
		switch (HIWORD(wParam))
		{
		case EN_CHANGE:
			indexEditorCheck();
			break;
		}
		break;

	case ID_DELETE:
		if (MessageBox(hWnd, TEXT("선택한 라인을 삭제할까요?"), PROGRAM_TITLE, MB_YESNO) == IDYES)
		{
			int32_t itemdata = deleteSelected();

			if (itemdata < 0)
			{
				MessageBox(hWnd, TEXT("삭제할 라인을 선택해주세요."), PROGRAM_TITLE, MB_OK | MB_ICONWARNING);
			}
			else
			{
				int32_t ret = -3;
				char buf[2048];
				send(sock, (char *)&ret, sizeof(int32_t), 0); // 삭제 코드는 -3
				send(sock, (char *)&itemdata, sizeof(int32_t), 0); // 아이템 데이터 전송
				recvData(sock, buf); // 처음에 주는 값은 받아서 버림 (모종의 사유가 ... )
				SendMessage(g_hFirst, WM_LBUTTONDOWN, 0, 0);
				SendMessage(g_hFirst, WM_LBUTTONUP, 0, 0);
			}
		}
		break;

	case ID_FIRST:
	{
		TCHAR path[MAX_PATH] = TEXT("");

		GetWindowText(g_hPath, path, MAX_PATH);
		if (_tcschr(&path[7], TEXT('/'))) // PATH: /G_Ms.. => /이 [6]에 있음
		{
			*(int *)(char *)sendbuf = -1;
			send(sock, sendbuf, sizeof(int), 0);
			deleteMemberList(&arMem);
			recvData(sock, recvbuf);
			createMemberList(&arMem, recvbuf);
		}
		break;
	}

	case ID_CONNECT:
		if (isInvalidIp(g_hIp))
		{
			if (isInvalidPort(g_hPort))
			{
				unsigned long isBlocking = 1;
				fd_set set, err;
				struct timeval tm;
				DWORD rcvTmout = 500;

				// Get IP
				GetWindowText(g_hIp, tbuf, 255);
				tcstombs(recvbuf, tbuf);

				// Get PORT
				GetWindowText(g_hPort, tbuf, 255);

				// 소켓 생성 및 설정
				//sock = socket(AF_INET, SOCK_STREAM, 0);
				sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&rcvTmout, sizeof rcvTmout);
				memset(&addr, 0x00, sizeof(struct sockaddr_in));
				addr.sin_family = AF_INET;
				addr.sin_port = htons(_ttoi(tbuf));
				addr.sin_addr.S_un.S_addr = inet_addr(recvbuf);
				
				// 타임아웃 및 Non-Blocking 설정
				tm.tv_sec = 1; // 0sec
				tm.tv_usec = 500000; // 0.500000sec
				ioctlsocket(sock, FIONBIO, &isBlocking); // non blocking

				// 서버에 커넥트
				connect(sock, (struct sockaddr *)&addr, sizeof(addr));
				//if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != SOCKET_ERROR) // blocking
				//{
				//	connectSuccess();
				//}

				// Blocking 설정
				isBlocking = 0;
				ioctlsocket(sock, FIONBIO, &isBlocking);

				// fd_set 초기화
				FD_ZERO(&set);
				FD_ZERO(&err);
				FD_SET(sock, &set);
				FD_SET(sock, &err);

				// 시간 경과 후 fd_set 확인
				select(0, NULL, &set, &err, &tm);
				if (FD_ISSET(sock, &set))
				{
					connectSuccess();
				}
				else
				{
					closesocket(sock);
					MessageBox(hWnd, TEXT("서버에 연결할 수 없습니다."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
				}
			}
			else
			{
				MessageBox(hWnd, TEXT("PORT가 유효하지 않습니다."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			MessageBox(hWnd, TEXT("IP가 유효하지 않습니다."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
		}
		break;

	case ID_DISCONNECT:
		send(sock, (char *)&deadCode, sizeof(int32_t), 0);
		closesocket(sock);
		cancelBookEnd();
		stopTPS(hWnd);
		disconnectSuccess();
		break;

	case ID_SEND:
	{
		int cnt = SendMessage(g_hSelectedList, LB_GETCOUNT, 0, 0);
		int32_t sendTPSprotocol = -2;

		if (cnt <= 0) //사용자가 선택한 변수가 0보다 작으면 선택한변수없다고 출력 (by 김유리)
		{
			MessageBox(hWnd, TEXT("선택한 변수가 없습니다."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
		}
		else
		{
			send(sock, (char *)&sendTPSprotocol, sizeof sendTPSprotocol, 0);
			sendTPS(hWnd);//있으면 전송할 수 있게함.
		}
		break;
	}

	case ID_STOP:
	{
		int32_t stopTPSprotocol = -2;
		send(sock, (char *)&stopTPSprotocol, sizeof stopTPSprotocol, 0);
		stopTPS(hWnd);
		break;
	}

	case ID_SETBOOK_H:
	case ID_SETBOOK_M:
	case ID_SETBOOK_S:
		switch (HIWORD(wParam))
		{
		case EN_UPDATE:
			timerEditorCheck(LOWORD(wParam));
			break;
		}
		break;

	case ID_SETBOOK:
		startBookEnd();
		break;

	case ID_CANCELBOOK:
		cancelBookEnd();
		break;
	}

	return 0;
}
static int OnCtlColorStatic(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND hTargetWnd = (HWND)lParam;
	HDC hTargetDC = (HDC)wParam;

	if ((HWND)lParam == g_hNotiStatic)
	{
		SetBkMode(hTargetDC, TRANSPARENT);
		SetTextColor(hTargetDC, RGB(255, 0, 0));
		return (HRESULT)hWhiteBrush;
	}
	else
	{
		SetBkMode(hTargetDC, TRANSPARENT);
		return (HRESULT)hWhiteBrush;
	}
}

static int OnCtlColorBtn(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if ((HWND)lParam == g_hSetBookEndTime)
	{
		SetBkMode((HDC)wParam, TRANSPARENT);
		return (HRESULT)hWhiteBrush;
	}

	return 0;
}

static int OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (GetClassLong(hWnd, GCL_STYLE) & CS_NOCLOSE)
	{
		if (MessageBox(hWnd, TEXT("TPS 전송을 중단하시겠습니까?"), PROGRAM_TITLE, MB_YESNO | MB_ICONWARNING) == IDNO)
		{
			return 0;
		}
	}
	return 1; // Program exit
}

static int OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int32_t deadCode = -4;

	if (isConnect)
	{
		SendMessage(hWnd, WM_COMMAND, (WPARAM)(MAKEWORD(ID_DISCONNECT, 0)), (LPARAM)0);
	}
	WSACleanup();
	if (hWhiteBrush) DeleteObject(hWhiteBrush);
	PostQuitMessage(0);
	return 0;
}

static void toggleEnableIndexTextBox(int index)
{
	LPARAM data = SendMessage(g_hMemberList, LB_GETITEMDATA, index, (LPARAM)0);

	if (data & 0x7FFFFFFF)
	{
		indexEditorCheck();
		EnableWindow(g_hIdx, TRUE);
		EnableWindow(g_hIndexStatic, TRUE);
	}
	else
	{
		EnableWindow(g_hIdx, FALSE);
		EnableWindow(g_hIndexStatic, FALSE);
	}
}

static BOOL sendSelectMember(HWND hWnd)
{
	// 인덱스 회신: 32비트의 정수 값
	TCHAR buf[256];
	int32_t memberIndex, arrayIndex;
	LPARAM data;
	
	memberIndex = SendMessage(g_hMemberList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	data = SendMessage(g_hMemberList, LB_GETITEMDATA, (WPARAM)memberIndex, (LPARAM)0);
	if (data & 0x7FFFFFFF)
	{
		GetWindowText(g_hIdx, buf, 256);
		
		if (_tcslen(buf) < 1)
		{
			SetWindowText(g_hNotiStatic, TEXT("<Noti> 배열의 인덱스를 입력하지 않아 0번 인덱스로 지정됩니다."));
			buf[0] = L'0';
			buf[1] = L'\0';
		}
		arrayIndex = _ttoi(buf);
		if (arrayIndex < 0 || arrayIndex >(data & 0x7FFFFFFF))
		{
			MessageBox(hWnd, TEXT("사용할 수 없는 인덱스입니다."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	send(sock, (char *)&memberIndex, sizeof(memberIndex), 0);
	
	if (data & 0x7FFFFFFF)
	{
		send(sock, (char *)&arrayIndex, sizeof(arrayIndex), 0);
	}

	return TRUE;
}

static void indexEditorCheck(void)
{
	TCHAR buf[256], *p;
	int memberIndex, arrayIndex;
	LPARAM data;

	memberIndex = SendMessage(g_hMemberList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	data = SendMessage(g_hMemberList, LB_GETITEMDATA, (WPARAM)memberIndex, (LPARAM)0);
	if (data & 0x7FFFFFFF)
	{
		GetWindowText(g_hIdx, buf, 256);
		for (p = buf; *p; p++)
		{
			if (*p < TEXT('0') || *p > TEXT('9'))
			{
				*p = TEXT('\0');
				SetWindowText(g_hIdx, buf);
			}
		}
	}

	arrayIndex = _ttoi(buf);
	if (arrayIndex >= (data & 0x7FFFFFFFF))
	{
		SetWindowText(g_hIdx, _itot((data & 0x7FFFFFFF) - 1, buf, 10));
	}
}

static BOOL isInvalidIp(HWND ipControl)
{
	TCHAR buf[256] = TEXT("");
	int dot = 0, token = 0, i;

	GetWindowText(ipControl, buf, 255);
	for (i = 0; buf[i]; i++)
	{
		if (buf[i] == TEXT('.'))
		{
			if (i == 0) return FALSE; // 맨 첫자리가 .임
			else if (buf[i - 1] == TEXT('.')) return FALSE; // .이 연속으로 나옴
			else if (token < 0 || token > 255) return FALSE; // IPv4는 0~255의 값만 취할 수 있음
			token = 0;
			dot++;
		}
		else if (buf[i] >= TEXT('0') && buf[i] <= TEXT('9'))
		{
			token = token * 10 + buf[i] - TEXT('0');
		}
		else if (dot > 3)
		{
			return FALSE; // .이 3개 이상임
		}
		else
		{
			return FALSE; // 숫자도, .도 아님
		}
	}
	if (dot == 3 && buf[i - 1] != TEXT('.')) return TRUE; // .이 3개고, 마지막 자리가 .이 아님
	else return FALSE; // .이 3개가 되지 않거나, 마지막 자리가 .임
}

static BOOL isInvalidPort(HWND portControl)
{
	TCHAR buf[256] = TEXT(""), *p;
	int port;

	GetWindowText(portControl, buf, 255);
	for (p = buf; *p; p++)
	{
		if (*p < TEXT('0') || *p > TEXT('9')) // 숫자가 아닌 글자가 섞여있음
		{
			return FALSE;
		}
	}

	port = _ttoi(buf);
	if (port < 0 || port > 65535) // port의 유효 범위: 0~65535
	{
		return FALSE;
	}

	return TRUE;
}

static void connectSuccess(void)
{
	char recvbuf[2048] = "";

	connectUI();
	recvData(sock, recvbuf);
	createMemberList(&arMem, recvbuf);
	isConnect = TRUE;
}

static void disconnectSuccess(void)
{
	disconnectUI();
	deleteMemberList(&arMem);
	isConnect = FALSE;
}

static void sendTPS(HWND hWnd)
{
	DWORD threadId;

	tpsSendUI(hWnd);
	cancelBookEnd();
	// 시그널(값 하나)만 전송
	//recvData(sock, NULL); // TPS 전송 시 서버 연결 종료 테스트
	
	hTpsSendThread = CreateThread(NULL, 0, sendTPSProc, NULL, 0, &threadId);
	isTpsSending = TRUE;
}

static void stopTPS(HWND hWnd)
{
	tpsStopUI(hWnd);
	isTpsSending = FALSE;
}

static DWORD __stdcall sendTPSProc(void *arg)
{
	// 시작 시간
	time_t startTime;
	struct tm startTime_local;

	// 현재 시간
	time_t nowTime;
	struct tm nowTime_local;

	// 직전 시간
	time_t beforeTime = 0;

	// 현재 시간 - 시작 시간
	time_t diffTime = 0;

	// 다음 HeartBeat 시간
	time_t nextHBTime = 0;

	// Windows Title을 담을 버퍼
	TCHAR buf[128], temp[128];

	// HB 관련 변수
	fd_set set, err;
	struct timeval tm;

	// 남은 시간
	int h, m, s;

	// 시작 시간 세팅
	time(&startTime);
	startTime_local = *localtime(&startTime);

	while (isTpsSending)
	{
		time(&nowTime);
		nowTime_local = *localtime(&nowTime);

		// 시작 날짜 업데이트
		if (nowTime_local.tm_hour == 0 && nowTime_local.tm_min == 0 && nowTime_local.tm_sec == 0 // 0시 0분 0초
			|| diffTime == 0) // 전송 시작 직후
		{
			// 오늘 날짜라면 "오늘", 지난 날짜라면 "n일 전"
			if (nowTime_local.tm_mday != startTime_local.tm_mday)
			{
				wsprintf(buf, TEXT("%d일 전 %d시 %d분 %d초"), 
					nowTime_local.tm_mday - startTime_local.tm_mday,
					startTime_local.tm_hour, startTime_local.tm_min, startTime_local.tm_sec);

				SetWindowText(g_hStartTime, buf);
			}
			else
			{
				wsprintf(buf, TEXT("%d시 %d분 %d초"),
					startTime_local.tm_hour, startTime_local.tm_min, startTime_local.tm_sec);

				SetWindowText(g_hStartTime, buf);
			}
		}

		// 진행 시간 업데이트
		if (nowTime != beforeTime)
		{
			diffTime = nowTime - startTime;
			buf[0] = TEXT('\0');
			if (diffTime >= 3600)
			{
				wsprintf(buf, TEXT("%d시간 "), diffTime / 3600);
				diffTime = diffTime % 3600;
			}
			if (diffTime >= 60)
			{
				wsprintf(temp, TEXT("%d분 "), diffTime / 60);
				diffTime = diffTime % 60;
				lstrcat(buf, temp);
			}
			wsprintf(temp, TEXT("%d초"), diffTime);
			lstrcat(buf, temp);
			SetWindowText(g_hStopwatch, buf);

			// 예약 종료 중이라면, 남은 시간 업데이트
			if (isBookEnd)
			{
				GetWindowText(g_hBookEndTime_Hour, buf, _countof(buf));
				h = _ttoi(buf);
				GetWindowText(g_hBookEndTime_Min, buf, _countof(buf));
				m = _ttoi(buf);
				GetWindowText(g_hBookEndTime_Sec, buf, _countof(buf));
				s = _ttoi(buf);

				if (--s < 0) // -1초, 단 -1초가 되면
				{
					s += 60; // +60초, -1분
					if (--m < 0) // -1분이 되면
					{
						m += 60; // +60분, -1시간
						if (--h < 0) // -1시간이 되면
						{
							s = m = h = 0; // 타임 아웃
							isTpsSending = FALSE;
							MessageBox(g_hMainWindow, TEXT("TPS 전송을 마칩니다."), PROGRAM_TITLE, MB_OK);
							SendMessage(g_hStop, WM_LBUTTONDOWN, 0, 0);
							SendMessage(g_hStop, WM_LBUTTONUP, 0, 0);
						}
					}
				}

				SetWindowText(g_hBookEndTime_Hour, _itot(h, buf, 10));
				SetWindowText(g_hBookEndTime_Min, _itot(m, buf, 10));
				SetWindowText(g_hBookEndTime_Sec, _itot(s, buf, 10));
			}
		}

		// HB전송 후 다음 HB 시간 업데이트
		if (nextHBTime == 0 || nextHBTime < nowTime)
		{
			// HB 송신
			int32_t hbprotocol = -6;
			send(sock, (char *)&hbprotocol, sizeof hbprotocol, 0);

			// Send Heartbeat (최대 1초 블록)
			tm.tv_sec = 1;
			tm.tv_usec = 0;
			
			FD_ZERO(&set);
			FD_ZERO(&err);
			FD_SET(sock, &set);
			FD_SET(sock, &err);

			select(0, NULL, &set, &err, &tm);
			if (FD_ISSET(sock, &set))
			{
				// HB 수신 (일단 -6이든 뭐든 수신한 것으로 처리. 필요 시 -6인지 확인하는 구문 추가)
				recv(sock, (char *)&hbprotocol, sizeof hbprotocol, 0);
				nextHBTime = nowTime + 5;
			}
			else
			{
				// 1초간 HB를 수신받지 못함 - 연결 끊김으로 판단
			}
		}

		beforeTime = nowTime;
		
		// delay
		Sleep(20);
	}

	// -2 전송

	return 0;
}

static void timerEditorCheck(WORD id)
{
	int maxValue, inputValue;
	HWND hTarget;
	TCHAR buf[5] = TEXT(""), *p;

	switch (id)
	{
	case ID_SETBOOK_H:
		maxValue = 99;
		hTarget = g_hBookEndTime_Hour;
		break;

	case ID_SETBOOK_M:
		maxValue = 59;
		hTarget = g_hBookEndTime_Min;
		break;

	case ID_SETBOOK_S:
		maxValue = 59;
		hTarget = g_hBookEndTime_Sec;
		break;

	default:
		return;
	}

	// Get Title
	GetWindowText(hTarget, buf, _countof(buf));
	for (p = buf; *p; p++)
	{
		// '0'보다 작거나, '9'보다 크면 숫자가 아님
		if (*p < TEXT('0') || *p > TEXT('9'))
		{
			*p = TEXT('\0');
			SetWindowText(hTarget, buf);
		}
		p++;
	}

	// Get String
	inputValue = _ttoi(buf);
	if (inputValue > maxValue)
	{
		SetWindowText(hTarget, _itot(maxValue, buf, 10));
	}
}

static void startBookEnd()
{
	isBookEnd = TRUE;
	startBookEndUI();
}

static void cancelBookEnd()
{
	isBookEnd = FALSE;
	cancelBookEndUI();
}