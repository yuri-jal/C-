#include <process.h>
#include <Windows.h>
#include <stdlib.h>
#include "resource.h"
#include <Tchar.h>
BOOL CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("hi");
HWND hListProc, hWndMain;
DWORD WINAPI ThreadFunc(LPVOID temp) {
	HDC hdc;
	BYTE Blue = 0;
	HBRUSH hBrush, hOldBrush;
	hdc = GetDC(hWndMain);

	for (;;)
	{
		Blue += 5;
		Sleep(20);
		hBrush = CreateSolidBrush(RGB(0, 0, Blue));
		hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		Rectangle(hdc,10,10,400,200);
		SelectObject(hdc, hOldBrush);
		DeleteObject(hBrush);
	}
	ReleaseDC(hWndMain, hdc);
	return 0;
}
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
	DWORD ThreadID;
	HANDLE hThread;
	switch (iMessage)
	{
		case WM_CREATE:
			hWndMain = hWnd;
			hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &ThreadID);
			return TRUE;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	/*
	if (iMessage == WM_CLOSE) {
		int Check=MessageBox(hWnd, L"종료?", L"종료확인",MB_ICONQUESTION|MB_OKCANCEL);
	}
	else if (iMessage == WM_DESTROY) {
		PostQuitMessage(0);
	}
	*/
	return (DefWindowProc(hWnd, iMessage, wParam, IParam));

}
BOOL CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam){
	//대화 상자의 메뉴나 컨트롤 항목을 선택 했을 때 발생하는 메시지
	//WM_COMMAND 메시지는 다양한 컨트롤을 사용하기 대문에 어떤 컨드롤이 선택되었는지를
	//확인할 수 있도록 wParam 항목의 하위 16비트에 컨트롤 ID가 저장되어있다.
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
			break;
		case WM_CLOSE:
			MessageBox(NULL, TEXT("종료"), TEXT("종료"), MB_OK);
			return FALSE;
	}
	return (DefWindowProc(an_dig, iMessage, wParam, IParam));
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
