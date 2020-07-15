
#include <windows.h>
int CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("hi");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

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
	//��ȭ ������ �޴��� ��Ʈ�� �׸��� ���� ���� �� �߻��ϴ� �޽���
	//WM_COMMAND �޽����� �پ��� ��Ʈ���� ����ϱ� �빮�� � ������� ���õǾ�������
	//Ȯ���� �� �ֵ��� wParam �׸��� ���� 16��Ʈ�� ��Ʈ�� ID�� ����Ǿ��ִ�.
	switch (iMessage) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return (DefWindowProc(hWnd, iMessage, wParam, IParam));
}
int CALLBACK MyMenuProc(HWND an_dig, UINT iMessage, WPARAM wParam, LPARAM IParam){
//��ȭ ������ �޴��� ��Ʈ�� �׸��� ���� ���� �� �߻��ϴ� �޽���
	//WM_COMMAND �޽����� �پ��� ��Ʈ���� ����ϱ� �빮�� � ������� ���õǾ�������
	//Ȯ���� �� �ֵ��� wParam �׸��� ���� 16��Ʈ�� ��Ʈ�� ID�� ����Ǿ��ִ�.
	if (iMessage == WM_COMMAND) {
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			//Ȯ��(IDOK)or ���(IDCANCEL)��ư�� ������ ���
			//Ȯ���� �� �ֵ��� �ι�° ���ڿ� ��ư ID ���� �ִ´�.
			EndDialog(an_dig, LOWORD(wParam));
			return 1;
		}
		return 0;
	}
}