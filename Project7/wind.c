#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "comctl32.lib")
#pragma warning(4 : 4996)

#include "resource.h"
#include <locale.h>
#include <process.h>
#include <Windows.h>
#include <stdlib.h>
#include <stdint.h>
#include <Tchar.h>
//BOOL CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("hi");
HWND hListProc, hWndMain;
HWND g_Idx;
static int OnCreate(HWND hWnd, WPARAM wParam, LPARAM IParam);
static HBRUSH hWhiteBrush;
static int OnCommand(HWND hWnd, WPARAM wParam, LPARAM IParam);
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
	TCHAR tbuf[256] = TEXT("");
	struct sockaddr_in addr; // ���� ��巹�� ����ü
	int recvLen = 0;
	int32_t deadCode = -4;

	switch (LOWORD(wParam))
	{
		//case ID_SELECT:
		SendMessage(g_hIdx, EM_LIMITTEXT, (WPARAM)8, 0); // �ִ� 8���ڸ� �Է� ����
		EnableWindow(g_hIdx, FALSE); // �迭 ������ ������ ���� Ȱ��ȭ
	}
}
/*
BOOL CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam) {
	//��ȭ ������ �޴��� ��Ʈ�� �׸��� ���� ���� �� �߻��ϴ� �޽���
	//WM_COMMAND �޽����� �پ��� ��Ʈ���� ����ϱ� �빮�� � ������� ���õǾ�������
	//Ȯ���� �� �ֵ��� wParam �׸��� ���� 16��Ʈ�� ��Ʈ�� ID�� ����Ǿ��ִ�.
	TCHAR word[100];
	switch (iMessage == WM_COMMAND) {
	case WM_INITDIALOG:
		return 1;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		default:
			break;
		}
		break;
	case WM_CLOSE:
		MessageBox(NULL, TEXT("����"), TEXT("����"), MB_OK);
		return FALSE;
	}
	return (DefWindowProc(an_dig, iMessage, wParam, IParam));
}
*/