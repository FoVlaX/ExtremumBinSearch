#define WIN32_LEAN_AND_MEAN
#include "D3DINIT.h"
#include "WININIT.h"
#include "server.h"
//#include <iostream>
#include <stdlib.h>
#include <time.h>
enum fgtype {rect,stick,g_o,t_o,g_o_r,z_o};

struct figure {
	std::pair<int, int> coord[4];
	int idsobj[4];
	fgtype type;
};

const int timedel = 60;

bool setfg(figure &fg, int **mas)
{
	if (fg.type == rect)
	{
		fg.coord[0].first = 0;
		fg.coord[0].second = 5;
		fg.coord[1].first = 0;
		fg.coord[1].second = 6;
		fg.coord[2].first = 1;
		fg.coord[2].second = 5;
		fg.coord[3].first = 1;
		fg.coord[3].second = 6;
	}
	if (fg.type == z_o)
	{
		fg.coord[0].first = 0;
		fg.coord[0].second = 5;
		fg.coord[1].first = 0;
		fg.coord[1].second = 6;
		fg.coord[2].first = 1;
		fg.coord[2].second = 6;
		fg.coord[3].first = 1;
		fg.coord[3].second = 7;
	}
	if (fg.type == stick)
	{
		fg.coord[0].first = 0;
		fg.coord[0].second = 5;
		fg.coord[1].first = 1;
		fg.coord[1].second = 5;
		fg.coord[2].first = 2;
		fg.coord[2].second = 5;
		fg.coord[3].first = 3;
		fg.coord[3].second = 5;
	}
	if (fg.type == g_o)
	{
		fg.coord[0].first = 0;
		fg.coord[0].second = 6;
		fg.coord[1].first = 0;
		fg.coord[1].second = 5;
		fg.coord[2].first = 1;
		fg.coord[2].second = 5;
		fg.coord[3].first = 2;
		fg.coord[3].second = 5;
	}
	if (fg.type == g_o_r)
	{
		fg.coord[0].first = 0;
		fg.coord[0].second = 4;
		fg.coord[1].first = 0;
		fg.coord[1].second = 5;
		fg.coord[2].first = 1;
		fg.coord[2].second = 5;
		fg.coord[3].first = 2;
		fg.coord[3].second = 5;
	}
	if (fg.type == t_o)
	{
		fg.coord[0].first = 1;
		fg.coord[0].second = 6;
		fg.coord[1].first = 0;
		fg.coord[1].second = 5;
		fg.coord[2].first = 1;
		fg.coord[2].second = 5;
		fg.coord[3].first = 2;
		fg.coord[3].second = 5;
	}

	for (int i = 0; i < 4; i++) {
		if (mas[fg.coord[i].first][fg.coord[i].second] != 0){
			return false;
		}
	}

	return true;
}

bool init(figure &fg, int **mas)
{
	int choose = rand()%6;
	switch (choose) {
	case 0: fg.type = rect; break;
	case 1: fg.type = g_o; break;
	case 2: fg.type = t_o; break;
	case 3: fg.type = stick; break;
	case 4: fg.type = g_o_r; break;
	case 5: fg.type = z_o; break;
	}
	bool pr;
	pr = setfg(fg,mas);
	if (pr) {
		for (int i = 0; i < 4; i++)
		{
			HRESULT hr;
			mas[fg.coord[i].first][fg.coord[i].second] = (int)new OBJECT("test.obj", "startex.dds", hr);
			OBJECT* o = (OBJECT*)mas[fg.coord[i].first][fg.coord[i].second];
			o->x = 1 + fg.coord[i].second * 2;
			o->y = 46 - fg.coord[i].first * 2;
			o->blend = { 0.9f,0.2f,0.2f,1.f };
			fg.idsobj[i] = mas[fg.coord[i].first][fg.coord[i].second];
		}
	}
	return pr;
}

bool itsmyblock( figure &fg,  int** mas,int x, int y)
{
	bool pr = false;
	for (int i = 0; i < 4; i++)
	{
		if (mas[x][y] == fg.idsobj[i])
		{
			pr = true;
			break;
		}
	}
	return pr;
}

bool canrot(figure& fg, int **mas)
{
	bool pr = true;
	for (int i = 0; i < 4; i++)
	{
		int y = fg.coord[i].first;
		int x = fg.coord[i].second;
		if (x < 0 || x >= 9 || y < 0 || y >22)
		{
			pr = false;
			break;
		}
		else
		{
			if (mas[y][x] != 0 && !itsmyblock(fg, mas, y, x))
			{
				pr = false;
				break;
			}
		}
	}
	return pr;
}

bool movedown(figure &fg, int **mas)
{
	bool proov = true;
	bool what = true;
	
	for (int i = 0; i < 4; i++)
	{
		if (fg.coord[i].first + 1 < 23) {
			
			if (mas[fg.coord[i].first + 1][fg.coord[i].second] != 0 )
			{
				what = itsmyblock(fg, mas, fg.coord[i].first + 1, fg.coord[i].second);
				if (!what)
				{
					proov = false;
					break;
				}
			}
		}
		else
		{
			proov = false;
			break;
		}
		
	}
	if (proov)
	{
		for (int i = 0; i < 4; i++)
		{
			mas[fg.coord[i].first][fg.coord[i].second] = 0;
		}
		fg.coord[0].first++;
		fg.coord[1].first++;
		fg.coord[2].first++;
		fg.coord[3].first++;
		for (int i = 0; i < 4; i++)
		{
			mas[fg.coord[i].first][fg.coord[i].second] = fg.idsobj[i];
			OBJECT* o = (OBJECT*)mas[fg.coord[i].first][fg.coord[i].second];
			o->x = 1 + fg.coord[i].second * 2;
			o->y = 46 - fg.coord[i].first * 2;
		}
	}
	return proov;
}


bool moveright(figure& fg, int** mas)
{
	bool proov = true;
	bool what = true;

	for (int i = 0; i < 4; i++)
	{
		if (fg.coord[i].second + 1 < 9) {

			if (mas[fg.coord[i].first][fg.coord[i].second+1] != 0)
			{
				what = itsmyblock(fg, mas, fg.coord[i].first, fg.coord[i].second+1);
				if (!what)
				{
					proov = false;
					break;
				}
			}
		}
		else{
			proov = false;
			break;
		}

	}
	if (proov)
	{
		for (int i = 0; i < 4; i++)
		{
			mas[fg.coord[i].first][fg.coord[i].second] = 0;
		}
		fg.coord[0].second++;
		fg.coord[1].second++;
		fg.coord[2].second++;
		fg.coord[3].second++;
		for (int i = 0; i < 4; i++)
		{
			mas[fg.coord[i].first][fg.coord[i].second] = fg.idsobj[i];
			OBJECT* o = (OBJECT*)mas[fg.coord[i].first][fg.coord[i].second];
			o->x = 1 + fg.coord[i].second * 2;
			o->y = 46 - fg.coord[i].first * 2;
		}
	}
	return proov;
}


bool moveleft(figure& fg, int** mas)
{
	bool proov = true;
	bool what = true;

	for (int i = 0; i < 4; i++)
	{
		if (fg.coord[i].second - 1 >=0) {

			if (mas[fg.coord[i].first][fg.coord[i].second - 1] != 0)
			{
				what = itsmyblock(fg, mas, fg.coord[i].first, fg.coord[i].second - 1 );
				if (!what)
				{
					proov = false;
					break;
				}
			}
		}
		else
		{
			proov = false;
			break;
		}

	}
	if (proov)
	{
		for (int i = 0; i < 4; i++)
		{
			mas[fg.coord[i].first][fg.coord[i].second] = 0;
		}
		fg.coord[0].second--;
		fg.coord[1].second--;
		fg.coord[2].second--;
		fg.coord[3].second--;
		for (int i = 0; i < 4; i++)
		{
			mas[fg.coord[i].first][fg.coord[i].second] = fg.idsobj[i];
			OBJECT* o = (OBJECT*)mas[fg.coord[i].first][fg.coord[i].second];
			o->x = 1 + fg.coord[i].second * 2;
			o->y = 46 - fg.coord[i].first * 2;
		}
	}
	return proov;
}


bool CheckBlocks(int **mas,int step) {
	
	bool proov = false;
	for (int i = 22; i >= 0; i--)
	{
		bool tr = true;
		for (int j = 0; j < 9; j++)
		{
			if (mas[i][j] == 0)
			{
				tr = false;
				break;
			}
		}
		if (tr)
		{
			proov = true;
			if (step >= timedel)
			{
				for (int j = 0; j < 9; j++)
				{
					OBJECT* o = (OBJECT*)mas[i][j];
					mas[i][j] = 0;
					delete o;
					
				}
				for (int k = i; k >= 0; k--) {
					if (k != 0) {
						for (int j = 0; j < 9; j++) {
							mas[k][j] = mas[k - 1][j];
							OBJECT* o = (OBJECT*)mas[k][j];
							if (o) {
								o->y = 46 - k * 2;
							}
						}
					}
					else {
						for (int j = 0; j < 9; j++) {
							mas[k][j] = 0;
						}
					}
				}
				i++;
			}
			else
			{
				for (int j = 0; j < 9; j++)
				{
					OBJECT* o = (OBJECT*)mas[i][j];
					if (step % 10 > 5) {
						o->blend = { 0.9f,0.9f,0.9f,1.f };
					}
					else {
						o->blend = { 0.9f,0.0f,0.0f,1.f };
					}
					
				}
			}
		}
	}

	return proov;

}

void shift_fg(figure& fg, int shift) {
	for (int i = 0; i < 4; i++) {
		fg.coord[i].second += shift;
	}
}

void rotate(figure &fg, int** mas,int k)
{
	if (fg.type != rect)
	{
		figure newfg = fg;
		std::pair<int, int> c = fg.coord[2];
		for (int i = 0; i < 4; i++)
		{
			newfg.coord[i].second = k * (fg.coord[i].first - c.first) + c.second;
			newfg.coord[i].first = (-k) * (fg.coord[i].second - c.second) + c.first;
		}

		bool pr = true;

		pr = canrot(newfg, mas);

		if (!pr) {
			shift_fg(newfg, 1);
			pr = canrot(newfg, mas);
			if (!pr) {
				shift_fg(newfg, -2);
				pr = canrot(newfg, mas);
				if (!pr) {
					shift_fg(newfg, 3);
					pr = canrot(newfg, mas);
					if (!pr) {
						shift_fg(newfg, -4);
						pr = canrot(newfg, mas);
					}
				}
			}
		}

		if (pr){
			for (int i = 0; i < 4; i++)
			{
				mas[fg.coord[i].first][fg.coord[i].second] = 0;
			}
			fg = newfg;
			for (int i = 0; i < 4; i++)
			{
				mas[fg.coord[i].first][fg.coord[i].second] = fg.idsobj[i];
				OBJECT* o = (OBJECT*)mas[fg.coord[i].first][fg.coord[i].second];
				o->x = 1 + fg.coord[i].second * 2;
				o->y = 46 - fg.coord[i].first * 2;
			}
		}
	}
	
}

void DeleteAllObj(int **mas){
	for (int i = 22; i >= 0; i--){
		for (int j = 0; j < 9; j++){
			if (mas[i][j] != 0) {
				OBJECT* d = (OBJECT*)mas[i][j];
				delete d;
				mas[i][j] = 0;
			}
		}
	}
}

OBJECT* CreatObjGameOver(){
	HRESULT hr;
	OBJECT *GameOver = new OBJECT("gameover.obj", "startex.dds", hr);
	GameOver->z = -4;
	GameOver->x = 10;
	GameOver->y = 20;
	GameOver->blend = { 1.f,0.1f,0.8f,1.f };
	GameOver->xs = 4;
	GameOver->ys = 4;
	GameOver->zs = 4;
	GameOver->yang = 3*XM_PIDIV2;
	return GameOver;
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE hpprevinstance,
	LPSTR lpcmdline,
	int ncmdshow)
{
	
	HWND hwnd;
	WININIT win(WINDOW_WIDTH, WINDOW_HEIGHT,"The Tetris", hinstance, hwnd);

	if (hwnd == NULL)
	{
		return 0;
	}

	D3DINIT d3d(hwnd);
	main_instance = hinstance;

	if (!d3d.Initialization()) {
		return 0;
	}

	bool dir = true;
	HRESULT hr = S_OK;

	LIGHT light({ -0.5f,1.4f,-0.6f,1.f }, { 1.f,1.f,1.f,1.f }, normalLight, {0.f,0.f,0.f,0.f});
	int Wall[51];
	int y = 40;
	int x = -1;

	for (int i = 0; i < 20; i++)
	{
		Wall[i] = (int)new OBJECT("test.obj", "startex.dds", hr);
		OBJECT* o = (OBJECT*)Wall[i];
		o->y = y;
		o->x = x;
		o->blend = { 0.5f,0.9f,0.4f,0.5f };
		y -= 2;
	}
	y = 40;
	x = 19;
	for (int i = 20; i < 40; i++)
	{
		Wall[i] = (int)new OBJECT("test.obj", "startex.dds", hr);
		OBJECT* o = (OBJECT*)Wall[i];
		o->y = y;
		o->x = x;
		o->blend = { 0.5f,0.9f,0.4f,0.5f };
		y -= 2;
	}
	y = 0;
	x = -1;
	
	OBJECT *floor = new OBJECT("test.obj", "startex.dds", hr);

	floor->xs = 40;
	floor->zs = 40;
	floor->ys = 0.5;
	floor->blend = { 1.f,1.f,1.f,1.f };
	floor->y = -1.5;
	floor->x = 10;
	for (int i = 40; i < 51; i++)
	{
		Wall[i] = (int)new OBJECT("test.obj", "startex.dds", hr);
		OBJECT* o = (OBJECT*)Wall[i];
		o->y = y;
		o->x = x;
		o->blend = { 0.5f,0.9f,0.4f,1.5f };
		x += 2;
		o = NULL;
	}

	int** field = new int* [23];
	for (int i = 0; i < 23; i++)
	{
		field[i] = new int[9];
		for (int j = 0; j < 9; j++)
		{
			field[i][j] = 0;
		}
	}
	
	figure fg;
	init(fg, field);
	d3d.SetViewPoint(10,20,4);
	d3d.SetGameSpeed(120);	
	if (FAILED(hr))
	{
		return 0;
	}

	float ang = 0;
	float ang2 = 0;
	float angle = 0;
	bool show = true;

	int step = 0;

	


	bool GMOVER = false;

	OBJECT* GameOver = NULL;
	bool PulseUp = true;
	int spd = 40;
	bool DeleteBlocks = false;
	srand(time(NULL));
	MSG msg;

	do{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{	
			d3d.Render();
			if (!GMOVER) {

				bool reason = step > OBJECT::spd;
				if (DeleteBlocks) reason = step > timedel;
				if ( reason )
				{
					if (DeleteBlocks) DeleteBlocks = false;
					step = 0;
					if (GetAsyncKeyState(VK_LEFT))
					{
						moveleft(fg, field);
					}
					if (GetAsyncKeyState(VK_UP))
					{
						rotate(fg, field, 1);
					}
					if (GetAsyncKeyState(VK_RIGHT))
					{
						moveright(fg, field);
					}
					if (!movedown(fg, field)) {
						DeleteBlocks = CheckBlocks(field, 0);
						if (!DeleteBlocks) {
							if (!init(fg, field)) {
								GMOVER = true;
							}
						}
					}

				}
				else {
					if (DeleteBlocks) {
						DeleteBlocks = CheckBlocks(field, step);
						if (step == timedel) {
							if (!init(fg, field)) {
								GMOVER = true;
							}
						}
					}
				}
				step++;
			}
			else{
				if (!GameOver){
					GameOver = CreatObjGameOver();
				}
				else {
					if (PulseUp && GameOver->xs < 7) {
						GameOver->xs *= 1.01;
						GameOver->ys *= 1.01;
						GameOver->zs *= 1.01;
					}
					else {
						if (PulseUp && GameOver->xs >= 7)  PulseUp = false;
					}


					if (!PulseUp && GameOver->xs > 4) {
						GameOver->xs /= 1.01;
						GameOver->ys /= 1.01;
						GameOver->zs /= 1.01;
					}
					else {
						if (!PulseUp && GameOver->xs <=4)  PulseUp = true;

					}
					if (GetAsyncKeyState(VK_CONTROL))
					{
						DeleteAllObj(field);
						delete GameOver;
						GameOver = NULL;
						init(fg, field);
						GMOVER = false;
					}
				}
			}
		}

	} while (msg.message != WM_QUIT);//если получен сигнал  закрытия окна выйти из цикла

	return 0;
}

