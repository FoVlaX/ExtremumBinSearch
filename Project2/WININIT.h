#pragma once
#include <Windows.h>
#include <windowsx.h>


#define WINDOW_CLASS_NAME "WINCLASS1"




class WININIT
{
	
public:
	static bool mousepress;
	WININIT(int WIDTH,int HEIGHT, const char* caption, HINSTANCE hinstance, HWND &hwnd); //окно
	~WININIT();
};

