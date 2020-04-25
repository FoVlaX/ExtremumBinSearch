#include "WININIT.h"

bool WININIT::mousepress = false;

 LRESULT CALLBACK WindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)  //обработчик прерываний
{
	PAINTSTRUCT ps;
	HDC hdc; //дескриптор контекста устройтсва
	
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	{
		WININIT::mousepress = true;
		ShowCursor(FALSE);
	}break;
	case WM_LBUTTONUP:
	{
		WININIT::mousepress = false;
		ShowCursor(TRUE);
	}break;
	case WM_KEYUP:
	{
		
	}break;
	case WM_KEYDOWN:
	{
		
	}break;
	case WM_MOUSEWHEEL:
	{
		
	}break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);

	}break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}break;
	default:break;
	}
	return (DefWindowProc(hwnd, msg, wparam, lparam));
}
 


WININIT::WININIT(int WIDTH, int HEIGHT,const char *caption, HINSTANCE hinstance, HWND &hwnd/*идентификатор окно понадобиться для привязки устройста директХ к нему*/)
{

	WNDCLASSEX winclass = { 0 }; //класс окна

	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	int y;
	if (!(y = RegisterClassEx(&winclass))) //регистрируем класс окна
	{
		hwnd = NULL;
	}
	if (!(hwnd = CreateWindowEx(NULL,
		WINDOW_CLASS_NAME,
		caption,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,
		WIDTH, HEIGHT,
		NULL,
		NULL,
		hinstance,
		NULL)))
	{
		hwnd = NULL;
	}
	//создаем окно
}


WININIT::~WININIT()
{
}
