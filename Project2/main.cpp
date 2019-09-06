#define WIN32_LEAN_AND_MEAN
#define WINDOW_CLASS_NAME "WINCLASS1"
#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <tlhelp32.h>

LRESULT CALLBACK WindowProc(HWND hwnd,
							UINT msg,
							WPARAM wparam,
							LPARAM lparam)
{
	HDC hdc; //дескриптор контекста устройтсва
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			if (wparam == 16)
			{
				MessageBox(NULL, LPCSTR("Ќј ши‘т≈, проаро"), LPCSTR("MessageBox"), 0);
			}
		}break;
		case WM_MOUSEMOVE:
		{

		}
		case WM_DESTROY:
		{ 
			PostQuitMessage(0);
			return (0);
		}break;
	default:break;
	}
	return (DefWindowProc(hwnd,msg,wparam,lparam));
}



int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE hpprevinstance,
	LPSTR lpcmdline,
	int ncmdshow)
{
	MSG msg;

	WNDCLASSEX winclass = { 0 };

	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	winclass.hbrBackground - (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	int y;
	if (!(y = RegisterClassEx(&winclass)))
	{

		return 0;
	}
	HWND hwnd;
	if (!(hwnd = CreateWindowEx(NULL,
		WINDOW_CLASS_NAME,
		"only text",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,
		400, 400,
		NULL,
		NULL,
		hinstance,
		NULL)))
		return 0;



	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}