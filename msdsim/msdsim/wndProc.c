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
*  �ۼ� : ������ ������ (devismylife@ariatech.kr), ������ ���� ������
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

	// Window �ʱ�ȭ
	makeMainWindow(hWnd);

	// ���� �ʱ�ȭ
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	// �귯�� ����
	hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));

	// Ŀ��Ʈ ���� ���� ������ UI�� ����
	disconnectUI();

	return 0;
}

static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	char recvbuf[2048] = "";
	char sendbuf[2048] = "";
	TCHAR tbuf[256] = TEXT("");
	struct sockaddr_in addr; // ���� ��巹�� ����ü
	int recvLen = 0;
	int32_t deadCode = -4;

	switch (LOWORD(wParam))
	{
	case ID_SELECT:
	{
		int n = SendMessage(g_hMemberList, LB_GETCURSEL, 0, 0); // (by ������)
		SetWindowText(g_hNotiStatic, TEXT(" "));

		if (n < 0) // (by ������)
		{
			debugf(TRUE, TEXT("if NOK"));
			MessageBox(hWnd, TEXT("������ ������ �����ϴ�"), PROGRAM_TITLE, MB_OK | MB_ICONWARNING);
		}
		else
		{
			if (sendSelectMember(hWnd))
			{
				SendMessage(g_hIdx, EM_SETSEL, 0, 8);
				SendMessage(g_hIdx, WM_CLEAR, 0, 0);
				deleteMemberList(&arMem);

				if (recvData(sock, recvbuf)) createMemberList(&arMem, recvbuf);
				else // ������ �׸� �������� ��
				{
					volatile int32_t value = getValueWindow(hWnd, sendbuf);

					if (value == -5)
					{
						send(sock, (char *)&value, sizeof(int32_t), 0); // -5���� ��
						send(sock, sendbuf, sizeof(int32_t) + strlen(&sendbuf[sizeof(int32_t)]), 0); // ����+���� ����
					}
					else
					{
						send(sock, (char *)&value, sizeof(int32_t), 0); // TPS ����
					}

					if (value != 0) // 0�̸� ����� �����
					{
						recvData(sock, recvbuf); // ���� �ڵ� ����
						updateSelectedList(&recvbuf[1]); // @�� �����ϰ� ���
					}
					// ���� arMem�� ������ ��, WM_CREATE���� �ۼ��� ���� ���� ���� �����
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
		if (MessageBox(hWnd, TEXT("������ ������ �����ұ��?"), PROGRAM_TITLE, MB_YESNO) == IDYES)
		{
			int32_t itemdata = deleteSelected();

			if (itemdata < 0)
			{
				MessageBox(hWnd, TEXT("������ ������ �������ּ���."), PROGRAM_TITLE, MB_OK | MB_ICONWARNING);
			}
			else
			{
				int32_t ret = -3;
				char buf[2048];
				send(sock, (char *)&ret, sizeof(int32_t), 0); // ���� �ڵ�� -3
				send(sock, (char *)&itemdata, sizeof(int32_t), 0); // ������ ������ ����
				recvData(sock, buf); // ó���� �ִ� ���� �޾Ƽ� ���� (������ ������ ... )
				SendMessage(g_hFirst, WM_LBUTTONDOWN, 0, 0);
				SendMessage(g_hFirst, WM_LBUTTONUP, 0, 0);
			}
		}
		break;

	case ID_FIRST:
	{
		TCHAR path[MAX_PATH] = TEXT("");

		GetWindowText(g_hPath, path, MAX_PATH);
		if (_tcschr(&path[7], TEXT('/'))) // PATH: /G_Ms.. => /�� [6]�� ����
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

				// ���� ���� �� ����
				//sock = socket(AF_INET, SOCK_STREAM, 0);
				sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&rcvTmout, sizeof rcvTmout);
				memset(&addr, 0x00, sizeof(struct sockaddr_in));
				addr.sin_family = AF_INET;
				addr.sin_port = htons(_ttoi(tbuf));
				addr.sin_addr.S_un.S_addr = inet_addr(recvbuf);
				
				// Ÿ�Ӿƿ� �� Non-Blocking ����
				tm.tv_sec = 1; // 0sec
				tm.tv_usec = 500000; // 0.500000sec
				ioctlsocket(sock, FIONBIO, &isBlocking); // non blocking

				// ������ Ŀ��Ʈ
				connect(sock, (struct sockaddr *)&addr, sizeof(addr));
				//if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != SOCKET_ERROR) // blocking
				//{
				//	connectSuccess();
				//}

				// Blocking ����
				isBlocking = 0;
				ioctlsocket(sock, FIONBIO, &isBlocking);

				// fd_set �ʱ�ȭ
				FD_ZERO(&set);
				FD_ZERO(&err);
				FD_SET(sock, &set);
				FD_SET(sock, &err);

				// �ð� ��� �� fd_set Ȯ��
				select(0, NULL, &set, &err, &tm);
				if (FD_ISSET(sock, &set))
				{
					connectSuccess();
				}
				else
				{
					closesocket(sock);
					MessageBox(hWnd, TEXT("������ ������ �� �����ϴ�."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
				}
			}
			else
			{
				MessageBox(hWnd, TEXT("PORT�� ��ȿ���� �ʽ��ϴ�."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
			}
		}
		else
		{
			MessageBox(hWnd, TEXT("IP�� ��ȿ���� �ʽ��ϴ�."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
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

		if (cnt <= 0) //����ڰ� ������ ������ 0���� ������ �����Ѻ������ٰ� ��� (by ������)
		{
			MessageBox(hWnd, TEXT("������ ������ �����ϴ�."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
		}
		else
		{
			send(sock, (char *)&sendTPSprotocol, sizeof sendTPSprotocol, 0);
			sendTPS(hWnd);//������ ������ �� �ְ���.
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
		if (MessageBox(hWnd, TEXT("TPS ������ �ߴ��Ͻðڽ��ϱ�?"), PROGRAM_TITLE, MB_YESNO | MB_ICONWARNING) == IDNO)
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
	// �ε��� ȸ��: 32��Ʈ�� ���� ��
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
			SetWindowText(g_hNotiStatic, TEXT("<Noti> �迭�� �ε����� �Է����� �ʾ� 0�� �ε����� �����˴ϴ�."));
			buf[0] = L'0';
			buf[1] = L'\0';
		}
		arrayIndex = _ttoi(buf);
		if (arrayIndex < 0 || arrayIndex >(data & 0x7FFFFFFF))
		{
			MessageBox(hWnd, TEXT("����� �� ���� �ε����Դϴ�."), PROGRAM_TITLE, MB_OK | MB_ICONERROR);
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
			if (i == 0) return FALSE; // �� ù�ڸ��� .��
			else if (buf[i - 1] == TEXT('.')) return FALSE; // .�� �������� ����
			else if (token < 0 || token > 255) return FALSE; // IPv4�� 0~255�� ���� ���� �� ����
			token = 0;
			dot++;
		}
		else if (buf[i] >= TEXT('0') && buf[i] <= TEXT('9'))
		{
			token = token * 10 + buf[i] - TEXT('0');
		}
		else if (dot > 3)
		{
			return FALSE; // .�� 3�� �̻���
		}
		else
		{
			return FALSE; // ���ڵ�, .�� �ƴ�
		}
	}
	if (dot == 3 && buf[i - 1] != TEXT('.')) return TRUE; // .�� 3����, ������ �ڸ��� .�� �ƴ�
	else return FALSE; // .�� 3���� ���� �ʰų�, ������ �ڸ��� .��
}

static BOOL isInvalidPort(HWND portControl)
{
	TCHAR buf[256] = TEXT(""), *p;
	int port;

	GetWindowText(portControl, buf, 255);
	for (p = buf; *p; p++)
	{
		if (*p < TEXT('0') || *p > TEXT('9')) // ���ڰ� �ƴ� ���ڰ� ��������
		{
			return FALSE;
		}
	}

	port = _ttoi(buf);
	if (port < 0 || port > 65535) // port�� ��ȿ ����: 0~65535
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
	// �ñ׳�(�� �ϳ�)�� ����
	//recvData(sock, NULL); // TPS ���� �� ���� ���� ���� �׽�Ʈ
	
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
	// ���� �ð�
	time_t startTime;
	struct tm startTime_local;

	// ���� �ð�
	time_t nowTime;
	struct tm nowTime_local;

	// ���� �ð�
	time_t beforeTime = 0;

	// ���� �ð� - ���� �ð�
	time_t diffTime = 0;

	// ���� HeartBeat �ð�
	time_t nextHBTime = 0;

	// Windows Title�� ���� ����
	TCHAR buf[128], temp[128];

	// HB ���� ����
	fd_set set, err;
	struct timeval tm;

	// ���� �ð�
	int h, m, s;

	// ���� �ð� ����
	time(&startTime);
	startTime_local = *localtime(&startTime);

	while (isTpsSending)
	{
		time(&nowTime);
		nowTime_local = *localtime(&nowTime);

		// ���� ��¥ ������Ʈ
		if (nowTime_local.tm_hour == 0 && nowTime_local.tm_min == 0 && nowTime_local.tm_sec == 0 // 0�� 0�� 0��
			|| diffTime == 0) // ���� ���� ����
		{
			// ���� ��¥��� "����", ���� ��¥��� "n�� ��"
			if (nowTime_local.tm_mday != startTime_local.tm_mday)
			{
				wsprintf(buf, TEXT("%d�� �� %d�� %d�� %d��"), 
					nowTime_local.tm_mday - startTime_local.tm_mday,
					startTime_local.tm_hour, startTime_local.tm_min, startTime_local.tm_sec);

				SetWindowText(g_hStartTime, buf);
			}
			else
			{
				wsprintf(buf, TEXT("%d�� %d�� %d��"),
					startTime_local.tm_hour, startTime_local.tm_min, startTime_local.tm_sec);

				SetWindowText(g_hStartTime, buf);
			}
		}

		// ���� �ð� ������Ʈ
		if (nowTime != beforeTime)
		{
			diffTime = nowTime - startTime;
			buf[0] = TEXT('\0');
			if (diffTime >= 3600)
			{
				wsprintf(buf, TEXT("%d�ð� "), diffTime / 3600);
				diffTime = diffTime % 3600;
			}
			if (diffTime >= 60)
			{
				wsprintf(temp, TEXT("%d�� "), diffTime / 60);
				diffTime = diffTime % 60;
				lstrcat(buf, temp);
			}
			wsprintf(temp, TEXT("%d��"), diffTime);
			lstrcat(buf, temp);
			SetWindowText(g_hStopwatch, buf);

			// ���� ���� ���̶��, ���� �ð� ������Ʈ
			if (isBookEnd)
			{
				GetWindowText(g_hBookEndTime_Hour, buf, _countof(buf));
				h = _ttoi(buf);
				GetWindowText(g_hBookEndTime_Min, buf, _countof(buf));
				m = _ttoi(buf);
				GetWindowText(g_hBookEndTime_Sec, buf, _countof(buf));
				s = _ttoi(buf);

				if (--s < 0) // -1��, �� -1�ʰ� �Ǹ�
				{
					s += 60; // +60��, -1��
					if (--m < 0) // -1���� �Ǹ�
					{
						m += 60; // +60��, -1�ð�
						if (--h < 0) // -1�ð��� �Ǹ�
						{
							s = m = h = 0; // Ÿ�� �ƿ�
							isTpsSending = FALSE;
							MessageBox(g_hMainWindow, TEXT("TPS ������ ��Ĩ�ϴ�."), PROGRAM_TITLE, MB_OK);
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

		// HB���� �� ���� HB �ð� ������Ʈ
		if (nextHBTime == 0 || nextHBTime < nowTime)
		{
			// HB �۽�
			int32_t hbprotocol = -6;
			send(sock, (char *)&hbprotocol, sizeof hbprotocol, 0);

			// Send Heartbeat (�ִ� 1�� ���)
			tm.tv_sec = 1;
			tm.tv_usec = 0;
			
			FD_ZERO(&set);
			FD_ZERO(&err);
			FD_SET(sock, &set);
			FD_SET(sock, &err);

			select(0, NULL, &set, &err, &tm);
			if (FD_ISSET(sock, &set))
			{
				// HB ���� (�ϴ� -6�̵� ���� ������ ������ ó��. �ʿ� �� -6���� Ȯ���ϴ� ���� �߰�)
				recv(sock, (char *)&hbprotocol, sizeof hbprotocol, 0);
				nextHBTime = nowTime + 5;
			}
			else
			{
				// 1�ʰ� HB�� ���Ź��� ���� - ���� �������� �Ǵ�
			}
		}

		beforeTime = nowTime;
		
		// delay
		Sleep(20);
	}

	// -2 ����

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
		// '0'���� �۰ų�, '9'���� ũ�� ���ڰ� �ƴ�
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