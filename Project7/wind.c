#include <process.h>
#include <Windows.h>
#include <stdlib.h>
#include "resource.h"
#include <Tchar.h>
BOOL CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("hi");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ITEM_MENU), HWND_DESKTOP, MyMenuProc);
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

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
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
	HDC hdc;
	PAINTSTRUCT ps;
	
	switch (iMessage) {
		case WM_CLOSE:
			if (MessageBox(hWnd, "���α׷��� �����Ͻðڽ��ϱ�?", "����", MB_YESNO) == IDNO) {
				return 0;
			}
			else {
				break;
			}
		case WM_DESTROY:
			PostQuitMessage(0);
		return 0;
	}
	return (DefWindowProc(hWnd, iMessage, wParam, IParam));
}
BOOL CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam){
	//��ȭ ������ �޴��� ��Ʈ�� �׸��� ���� ���� �� �߻��ϴ� �޽���
	//WM_COMMAND �޽����� �پ��� ��Ʈ���� ����ϱ� �빮�� � ������� ���õǾ�������
	//Ȯ���� �� �ֵ��� wParam �׸��� ���� 16��Ʈ�� ��Ʈ�� ID�� ����Ǿ��ִ�.
	TCHAR word[100];
	switch (iMessage==WM_COMMAND) {
		case WM_INITDIALOG:
				return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			
			default:
				break;
			}
	
	}
}
/*
unsigned WINAPI ThreadFunction(void* para) {
	int* num = (int*)para;
	*num = 10;
	return 0;
}
int main()
{
	HANDLE hThread;
	unsigned threadID;
	int num = 0;

	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, (void*)&num, 0, &threadID);
	if (0 == hThread)
	{
		puts("_beginthreadex() error");
		return - 1;
	}
	sleep(1000);
	printf("num:%d\n", num);
	return 0;
}
*/
