#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "comctl32.lib")
#pragma warning(4 : 4996)
#define PORT 9999
#define IPADDR "172.16.53.118"
#include "resource.h"
#include <locale.h>
#include <process.h>
#include <winsock2.h>
#include <Windows.h>
#include <stdlib.h>
#include <stdint.h>
#include <Tchar.h>
#define MAX_IDENTI       (32 + 1)
//BOOL CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
typedef struct st_Member
{
	TCHAR type[MAX_IDENTI];
	TCHAR name[MAX_IDENTI];
	BOOL isPointer;   // non-pointer type: false
	int  maxArrayIdx; // non-array type: 0
} Member;
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("hi");
HWND hListProc, hWndMain;
HWND g_Idx;
HWND g_hIp;
HWND g_hSelectedList;
HWND g_hPort;
static BOOL isRunning;
HWND g_hPath;
HWND g_hDisConnect;
HWND g_hNotiStatic;
HWND g_hMemberList;
Member* arMem;
static HWND owner, tpsEdit, okBtn, cancleBtn;
static HWND strBtn, decBtn;
static RECT ownerArea;
static RECT clientArea; // = { 0, 0, ���ϴ���ǥ, ���ϴ���ǥ }
void updateSelectedList(char* buf);
static int  ret;
static char* a_sendBuf; // getValueWindow's argument
void mbstotcs(TCHAR* dest, char* origin);
static HBRUSH hWhiteBrush;
static SOCKET sock;
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM IParam);
static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM IParam);
void updateMemberList(const Member* arMem, int com);
int createMemberList(Member** dest, const BYTE* arBuf);
void updateMemberList(const Member* arMem, int com);
//static BOOL isInvalidIp(HWND ipControl);
//static BOOL isInvalidPort(HWND portControl);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ITEM_MENU), HWND_DESKTOP, WndProc);
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);
	setlocale(LC_ALL, "ko_KR.UTF-8"); //���
	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, 300, 50, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM IParam)
{
	switch (iMessage)
	{
		case WM_CREATE:
			return OnCreate(hWnd, wParam, IParam);
		case WM_COMMAND:
			return OnCommand(hWnd, wParam, IParam);
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

	}

	return (DefWindowProc(hWnd, iMessage, wParam, IParam));

}
static BOOL sendSelectMember(HWND hWnd)
{
	// �ε��� ȸ��: 32��Ʈ�� ���� ��
	TCHAR buf[256];
	int32_t memberIndex, arrayIndex;
	LPARAM data;

	memberIndex = SendMessage(g_hMemberList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	data = SendMessage(g_hMemberList, LB_GETITEMDATA, (WPARAM)memberIndex, (LPARAM)0);
	if (data & 0x7FFFFFFF)//�迭�� �ε���
	{
		GetWindowText(g_hIdx, buf, 256);

		if (_tcslen(buf) < 1)
		{
			//MessageBox(hWnd, TEXT("�ε����� �Է����ּ���."), TEXT("msd �ùķ�����"), MB_OK);
			//return FALSE;
			SetWindowText(g_hNotiStatic, TEXT("<Noti> �迭�� �ε����� �Է����� �ʾ� 0�� �ε����� �����˴ϴ�."));
			buf[0] = L'0';
			buf[1] = L'\0';
		}
		arrayIndex = _ttoi(buf);
		if (arrayIndex < 0 || arrayIndex >(data & 0x7FFFFFFF))
		{
			MessageBox(hWnd, TEXT("����� �� ���� �ε����Դϴ�."), TEXT("msd �ùķ�����"), MB_OK);
			return FALSE;
		}
	}

	send(sock, (char*)&memberIndex, sizeof(memberIndex), 0);

	if (data & 0x7FFFFFFF)
	{
		send(sock, (char*)&arrayIndex, sizeof(arrayIndex), 0);
	}

	return TRUE;
}
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM IParam) {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 0), &wsadata);
	// �귯�� ����
	hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
	return 0;
}
static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM IParam) {
	char recvbuf[2048] = "";
	char sendbuf[2048] = "";
	int c_socket;
	TCHAR tbuf[256] = TEXT("");
	struct sockaddr_in addr; // ���� ��巹�� ����ü
	int recvLen = 0;
	int32_t deadCode = -4;
	//SendMessage(g_hIdx, EM_LIMITTEXT, (WPARAM)8, 0); // �ִ� 8���ڸ� �Է� ����
	//EnableWindow(g_hIdx, FALSE); // �迭 ������ ������ ���� Ȱ��ȭ
	switch (LOWORD(wParam))
	{
	case ID_SELECT:
		SetWindowText(g_hNotiStatic, TEXT(" "));
		if (sendSelectMember(hWnd))
		{
			SendMessage(g_hIdx, EM_SETSEL, 0, 8);
			SendMessage(g_hIdx, WM_CLEAR, 0, 0);
			//deleteMemberList(&arMem);
			if (recvData(sock, recvbuf)) createMemberList(&arMem, recvbuf);
			else // ������ �׸� �������� ��
			{
				int32_t value = getValueWindow(hWnd, sendbuf);

				if (value == -5)
				{
					//MessageBox(hWnd, TEXT("Proc: -5��"), TEXT("DEBUG"), MB_OK);
					send(sock, (char*)&value, sizeof(int32_t), 0); // -5���� ��
					send(sock, sendbuf, sizeof(int32_t) + strlen(&sendbuf[sizeof(int32_t)]), 0); // ����+���� ����
				}
				else
				{
					//MessageBox(hWnd, TEXT("Proc: 0�̻���"), TEXT("DEBUG"), MB_OK);
					send(sock, (char*)&value, sizeof(int32_t), 0); // TPS ����
				}

				if (value != 0) // 0�̸� ����� �����
				{
					recvData(sock, recvbuf); // ���� �ڵ� ����
					updateSelectedList(&recvbuf[1]); // @�� �����ϰ� ���
				}
				// ���� arMem�� ������ ��, WM_CREATE���� �ۼ��� ���� ���� ���� �����
				//deleteMemberList(&arMem);
				recvData(sock, recvbuf);
				createMemberList(&arMem, recvbuf);
			}
		}
		return 0;
	case ID_CONNECT:
		//if (isInvalidIp(g_hIp))
		//{
			//if (isInvalidPort(g_hPort))
			//{
				// Get IP
				GetWindowText(g_hIp, tbuf, 255);
				//tcstombs(recvbuf, tbuf);

				// Get PORT
				GetWindowText(g_hPort, tbuf, 255);
				// ���� ���� �� ����
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				//2.���� ���� �ʱ�ȭs
				memset(&addr, 0x00, sizeof(struct sockaddr_in));
				addr.sin_addr.s_addr = inet_addr(PORT); //������ IP ���� (127.0.0.1)
				addr.sin_family = AF_INET;
				addr.sin_port = htons(IPADDR);
				// ������ Ŀ��Ʈ
				if (connect(c_socket, (struct sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR) {
					//connectSuccess();
				}
				else {
					MessageBox(hWnd, TEXT("������ ������ �� �����ϴ�."), TEXT("��Ʈ��ũ ����"), MB_OK);
				}
			//}
		//}
	}
}
int recvData(SOCKET sock, BYTE* arBuf)
{
	// Protocol: Length@com@path[@type@name@isPointer@maxArrayIdx] * com
	// Length = 4byte, integer
	// otherwise = string
	int length = 0, recvLen = 0;
	DWORD begin, now;
	MSG msg;

	//char debugBuf[1024];

	// get length
	recv(sock, (char*)&length, sizeof(int), 0);
	//wsprintfA(debugBuf, "length: %d", length);
	//MessageBoxA(NULL, debugBuf, "��� �ùķ����� �����", MB_OK);

	// get data
	begin = GetTickCount();
	for (; length > 0; length -= recvLen)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		recvLen = recv(sock, &arBuf[recvLen], length, 0);
		now = GetTickCount();

		if (now - begin > 3000)
		{
			MessageBox(NULL, TEXT("������ ������ ���������ϴ�"), TEXT("��� �ùķ�����"), MB_OK);
			SendMessage(g_hDisConnect, WM_LBUTTONDOWN, 0, 0);
			SendMessage(g_hDisConnect, WM_LBUTTONUP, 0, 0);
			return 0;
		}
	}

	//sprintf(debugBuf, "Length: %d, arBuf: %s", recvLen, arBuf);
	//MessageBoxA(NULL, debugBuf, "SIBAL", MB_OK);

	return recvLen;
}
void updateMemberList(const Member* arMem, int com)
{
	int i;
	TCHAR line[256], buf[32], temp[32];
	LPARAM data;

	// clear list
	SendMessage(g_hMemberList, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	// list update
	for (i = 0; i < com; i++)
	{
		wsprintf(buf, TEXT("%s%s%s"),
			arMem[i].maxArrayIdx == 0 ? TEXT("") : TEXT("["),
			arMem[i].maxArrayIdx == 0 ? TEXT("") : _itot(arMem[i].maxArrayIdx, temp, 10),
			arMem[i].maxArrayIdx == 0 ? TEXT("") : TEXT("]")
		);
		wsprintf(line, TEXT("%-32s%s%s%s"),
			arMem[i].type,
			arMem[i].isPointer ? "*" : "",
			arMem[i].name,
			buf
		);
		SendMessage(g_hMemberList, LB_INSERTSTRING, (WPARAM)i, (LPARAM)line);
		// ���� 1��Ʈ: isPointer(TRUE/FALSE), ���� 31��Ʈ: �迭 ũ��
		data = (arMem[i].isPointer ? 1 << 31 : 0) | arMem[i].maxArrayIdx;
		SendMessage(g_hMemberList, LB_SETITEMDATA, (WPARAM)i, (LPARAM)data);
	}
}
int createMemberList(Member** dest, const BYTE* arBuf)
{
	char tok[256];
	TCHAR tbuf[256], item[256];
	int ret, com, i;

	// 0. arBuf[0] == '@' => PASS
	arBuf++;

	// 1. get com & memory allocation
	ret = getToken(tok, arBuf);
	arBuf += ret + 1;
	com = atoi(tok);
	*dest = malloc(sizeof(Member) * com);

	// 2. get path & set path
	ret = getToken(tok, arBuf);
	arBuf += ret + 1;
	mbstotcs(tbuf, tok);
	wsprintf(item, TEXT("PATH: %s"), tbuf);
	SetWindowText(g_hPath, item);

	// 3. loop => set Member Struct & add List
	for (i = 0; i < com; i++)
	{
		// 3-1. type get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		mbstotcs(tbuf, tok);
		lstrcpy((*dest)[i].type, tbuf);

		// 3-2. name get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		mbstotcs(tbuf, tok);
		lstrcpy((*dest)[i].name, tbuf);

		// 3-3. isPointer get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		(*dest)[i].isPointer = !!(*tok - '0');

		// 3-4. maxArrayIdx get
		ret = getToken(tok, arBuf);
		arBuf += ret + 1;
		(*dest)[i].maxArrayIdx = atoi(tok);

		wsprintf(item, TEXT("%-32s%s"), (*dest)[i].type, (*dest)[i].name);
		SendMessage(g_hMemberList, LB_ADDSTRING, (WPARAM)0, (LPARAM)item);
	}
	updateMemberList(*dest, com);

	return com;
}
static HWND makeDefaultWindow(WNDCLASS* pWndClass, LPCTSTR lpszClass, LPCTSTR title, HINSTANCE hInst)
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
	pWndClass->lpfnWndProc = (WNDPROC)WndProc;
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

	return hWnd;
}
int getValueWindow(HWND parents, char* sendBuf)
{
	HWND hGetValWnd;
	WNDCLASS WndClass;
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
void mbstotcs(TCHAR* dest, char* origin)
{
#pragma warning(push)
#pragma warning(4:4133)
	* dest = TEXT('\0');
	if (sizeof(char) == sizeof(TCHAR))
	{
		strcpy(dest, origin);
	}
	else
	{
		//mbstowcs(dest, origin, strlen(origin) * sizeof(TCHAR));
		wsprintfW(dest, L"%S", origin);
	}
#pragma warning(pop)
}
int getToken(char* tokenBuf, const BYTE* arBuf)
{
	const char* p;

	p = strchr(arBuf, '@');
	if (p)
	{
		strncpy(tokenBuf, arBuf, p - arBuf);
		tokenBuf[p - arBuf] = '\0';
	}
	else
	{
		strcpy(tokenBuf, arBuf);
	}

	return p - arBuf;
}
void updateSelectedList(char* buf)
{
	TCHAR tbuf[256] = TEXT("");
	int cnt = 0, i;

	// ���� ���ڿ��� = �������� �ڸ�
	mbstotcs(tbuf, buf);
	_tcstok(tbuf, TEXT("="));

	// ���� �׸�� �ߺ��Ǵ��� Ȯ��
	i = SendMessage(g_hSelectedList, LB_FINDSTRING, -1, (LPARAM)tbuf);
	if (i != LB_ERR)
	{
		SendMessage(g_hSelectedList, LB_DELETESTRING, i, 0);
	}

	// ����, ���� (�ű� �׸��̸� �ܼ� ����)
	mbstotcs(tbuf, buf);
	cnt = SendMessage(g_hSelectedList, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
	SendMessage(g_hSelectedList, LB_INSERTSTRING, (WPARAM)cnt, (LPARAM)tbuf);
}

/*
static BOOL isInvalidIp(HWND ipControl)//IP �ּ� �Է¹��� ���� ��Ȯ�ϰ� �Է��ߴ��� �Ǵ�
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

static BOOL isInvalidPort(HWND portControl) //PORT��ȣ�� �Է¹��� ���� ��Ȯ���� �Ǵ�
{
	TCHAR buf[256] = TEXT(""), * p;
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
*/