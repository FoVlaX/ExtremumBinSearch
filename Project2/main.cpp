#define WIN32_LEAN_AND_MEAN
#define WINDOW_CLASS_NAME "WINCLASS1"
#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <xnamath.h>
#include <D3Dcompiler.h>
#include <D3DX11async.h>
#define WINDOW_HEIGHT 400 //размеры окна
#define WINDOW_WIDTH 400
#define BBP 16 //глубина цвета
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,"D3DX11.lib") // вот оно то что надо было подключиtb
#pragma comment(lib, "d3dcompiler.lib")
float time = 0.0f;
const int vCount = 60;
HWND main_window_handle = NULL;  // идентификатор окна
HINSTANCE main_instance = NULL;  // идентификатор приложения
D3D_DRIVER_TYPE g_drivertype = D3D_DRIVER_TYPE_NULL; //
D3D_FEATURE_LEVEL g_featurelevel = D3D_FEATURE_LEVEL_11_0; // версия  директХ поддерживаемая видеокартой
ID3D11Device *g_pd3device = NULL; // создание ресурсов (текстуры шейдеры, буферы 3-ч мерных объектов  и т.д.)
ID3D11DeviceContext *g_pImmediateContext = NULL; // вывод графической информации
IDXGISwapChain *g_pSwapChain = NULL;  // работа с буфером рисования и вывод нарисованного на экран, должен содержать два буфера задний и передний (экран ) для корректной отрисвки без мельтешений
ID3D11RenderTargetView *g_pRenderTargetView = NULL; // собственно задний буфер
ID3D11VertexShader *g_pVertexShader = NULL; //вершинный шейдер
ID3D11PixelShader *g_pPixelShader = NULL; // пиксельынй шейдер
ID3D11InputLayout *g_pVertexLayout = NULL; // описание формата вершин
ID3D11Buffer *g_pVertexBuffer = NULL; // Буфер вершин


HRESULT InitDevice();//инициализайция директХ
void CleanUpDevice(); //удаление объектов Direct3D
void Render(); //отрисовка 
HRESULT InitGeometry();
HRESULT CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

struct SimpleVertex
{
	XMFLOAT3 Pos;
};

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
				MessageBox(NULL, LPCSTR("НА шиФтЕ, проаро"), LPCSTR("MessageBox"), 0);
			}
		}break;
		case WM_PAINT:
		{

		}break;
		case WM_DESTROY:
		{ 
			PostQuitMessage(0);
			return 0;
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
		"DirectX",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL,
		NULL,
		hinstance,
		NULL)))
		return 0;

	main_window_handle = hwnd;
	main_instance = hinstance;
	if (FAILED(InitDevice()))
	{
		CleanUpDevice();
		return 0;
	}

	if (FAILED(InitGeometry()))
	{
		InitGeometry();
		return 0;
	}
	
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}

	}
	CleanUpDevice();

	return 0;
}

HRESULT InitDevice()
{
	HRESULT hr = S_OK;
	RECT rc;
	GetClientRect(main_window_handle, &rc);
	UINT width = rc.right - rc.left; //получаем ширину
	UINT height = rc.bottom - rc.top; // и высоту окна

	UINT createDeviceFlags = 0;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes); //создаем массив для проверки на хардварную обработку
	//создаем список поддерживаемых версий директХ

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	//Создание устройств директХ. Заполним структуру
	// которая описывает свойства переднегобуфера и привязывает его к нашему окну
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1; //one back buffer
	sd.BufferDesc.Width = width; //shirina bufera
	sd.BufferDesc.Height = height; //visota buffera
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ФОРМАТ ПИКСЕЛЯ В БУФЕРЕ
	sd.BufferDesc.RefreshRate.Numerator = 75; // частота обновления экрана
	sd.BufferDesc.RefreshRate.Denominator = 1;// 
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //НАЗНАЧЕНИЕ БУФЕРА - peredniu? БУФЕР
	sd.OutputWindow = main_window_handle; //привязка к окну
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE; // оконный режим
	//
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_drivertype = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_drivertype, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
			&sd, &g_pSwapChain, &g_pd3device, &g_featurelevel, &g_pImmediateContext);
		if SUCCEEDED(hr) break; //если устройства созданы то выходим из цикла
	}
	if FAILED(hr) return hr;
	//создаем задний буфер в SDK
	//RenderTargetOutput - это передний буфер, RenderTargetView - задний

	ID3D11Texture2D* pBackBuffer = NULL;//объект текстур, т.е. область памяти которуюю можно юзать для расования как буфер глубин и как текстуру
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
	if FAILED(hr) return hr;
	hr = g_pd3device->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if FAILED(hr) return hr;
	//подключаем объект заднего буфера к контексту устройства
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
	//настройка вьюпорта
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	//подключаем вьюпорт к контексту устроства
	g_pImmediateContext->RSSetViewports(1, &vp);
	return S_OK;
}

void CleanUpDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3device)  g_pd3device->Release();
}

void Render()
{
	float ClearColor[4] = { 0.5f,0.5f,0.5f,1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);

	//Подключаем к устройству рисования шейдеры
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->Draw(vCount+2, 0);

	g_pSwapChain->Present(0, 0);
}

HRESULT InitGeometry()
{
	HRESULT hr = S_OK;
	//компиляция вершинного шейдера из файла
	ID3DBlob* pVSBlob = NULL; //вспомогательный оъект просто место в оперативной памяти
	hr = CompileShaderFromFile("urok2.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,"VS Don't compile file FX, Please, execute this porgram from directory with FX file","ERROR",MB_OK);
		return hr;
	}

	//Create Vertex SHader
	hr = g_pd3device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}
	//определение шаблона вершин
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	UINT numElements = ARRAYSIZE(layout);
	//создание шаблона вершин
	hr = g_pd3device->CreateInputLayout(layout,numElements,pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),&g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr)) return hr;
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	ID3DBlob* pPSBlob = NULL; //вспомогательный оъект просто место в оперативной памяти
	hr = CompileShaderFromFile("urok2.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "PS Don't compile file FX, Please, execute this porgram from directory with FX file", "ERROR", MB_OK);
		return hr;
	}

	//Create Vertex SHader
	hr = g_pd3device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
	if (FAILED(hr))
	{
		pPSBlob->Release();
		return hr;
	}

	//создание буфера вершин
	
	SimpleVertex verticles[vCount+2];
	float angle = 0;
	float step = 4 * 3.14 / vCount;
	int i = 0;
	verticles[i].Pos.x = 0.0f + 0.7f*cos(angle); verticles[i].Pos.y = 0.0f + 0.7f*sin(angle);
	angle += step;
	verticles[i].Pos.z = 0.5f;
	for ( i = 1; i < vCount+1; i++)
	{
		if ( i%2!=1)
		{
			verticles[i].Pos.x = 0.0f + (0.7f+time)*cos(angle); verticles[i].Pos.y = 0.0f + (0.7f+time)*sin(angle);
			angle += step;
			verticles[i].Pos.z = 0.5f;
		}
		else
		{
			verticles[i].Pos.x = 0.0f; verticles[i].Pos.y = 0.0f;
			verticles[i].Pos.z = 0.5f;
		}
	}
	i = vCount + 1;
	angle = 0;
	verticles[i].Pos.x = 0.0f + (0.7f+time)*cos(angle); verticles[i].Pos.y = 0.0f + 0.7f*sin(angle);
	verticles[i].Pos.z = 0.5f;
	
	//Описывающая буфер структура
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * (vCount+2); //размер буфера = размер одной вершины*3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData; // структура, содержащая данные буфера;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = verticles; // указатель на наши вершины;
	//Создаем объект буффера вершин ID3D11Buffer
	hr = g_pd3device->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}
	//усtановка буфера вершин
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	//установка способа отрисовки вершин в буфре
	g_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	return S_OK;
}
HRESULT CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint,LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
		{
			OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
		}
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();
	return S_OK;
}