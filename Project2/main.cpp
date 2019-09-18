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

#pragma comment(lib,"D3DX11.lib") // вот оно то что надо было подключиtb
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

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
ID3D11Buffer *g_pIndexBuffer = NULL; //Буфер индексов вершин в каком порядке отрисовывать
ID3D11Buffer *g_pConstantBuffer = NULL; // Констатный буфер
ID3D11Texture2D* g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;

XMMATRIX g_World; //матрица мира00
XMMATRIX g_View; //матрциа вида
XMMATRIX g_Projection; //матрица проекции
float pulse = 0.0f;
bool Bpulse = true;
HRESULT InitDevice();//инициализайция директХ
void CleanUpDevice(); //удаление объектов Direct3D
void Render(); //отрисовка 
float ViewAngle = 0;
HRESULT InitMatrixes(); //Инициализация матриц
void SetMatrixes(float angle); // изменение матрицы мира
HRESULT InitGeometry();
HRESULT CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color; //каждая вершна содержит инфу о цвете
};

//Структура константного буфера

struct ConstantBuffer
{
	XMMATRIX mWorld; //матрица мира
	XMMATRIX mView; //марица вида
	XMMATRIX mProjection; //матрица проекции
};

LRESULT CALLBACK WindowProc(HWND hwnd,
							UINT msg,
							WPARAM wparam,
							LPARAM lparam)
{
	PAINTSTRUCT ps;
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
			hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);

		}break;
		case WM_DESTROY:
		{ 
			PostQuitMessage(0);
			
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
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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
		CleanUpDevice();
		return 0;
	}

	if (FAILED(InitMatrixes()))
	{
		CleanUpDevice();
		return 0;
	}

	bool dir = true;
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

	//Создаем буффер глубин
	// Создаем текстуру описание буфера глубин

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	//при помощи заполненной структуры описания созахдаем объект тектуры
	hr = g_pd3device->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr)) 
		return hr;
	//now создаем сам объект буффера глубин сначало как обычно стурктура с описынием
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3device->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr)) 
		return hr;
	//теперь подключим к устройствву рисоваия сразу оба вида (глубин и задний буффер)
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

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
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3device)  g_pd3device->Release();
}

void Render()
{
	float ClearColor[4] = { 0.5f,0.5f,0.6f,1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,D3D11_CLEAR_DEPTH, 1.0f,0);


	for (int i = 0; i < 6; i++)
	{
		SetMatrixes(i*(XM_PI * 2) / 6);

		g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer); // 0 - точка входа в констант буффер в шейдере
		g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
		g_pImmediateContext->DrawIndexed(18, 0, 0);
	}
	g_pSwapChain->Present(0, 0);
}

HRESULT InitGeometry()
{
	HRESULT hr = S_OK;
	//компиляция вершинного шейдера из файла
	ID3DBlob* pVSBlob = NULL; //вспомогательный оъект просто место в оперативной памяти
	hr = CompileShaderFromFile("urok2.fx", "VS", "vs_5_0", &pVSBlob);
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
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
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
	
	SimpleVertex verticles[] =
	{// координаты х у зет                      цвет ргба
		{XMFLOAT3(0.0f,1.4f,0.0f), XMFLOAT4(1.0f,1.0f,0.0f,1.0f) },
		{XMFLOAT3(-1.0f,0.0f,-1.0f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f) },
		{XMFLOAT3(1.0f,0.0f,-1.0f), XMFLOAT4(1.0f,0.0f,0.0f,1.0f) },
		{XMFLOAT3(-1.0f,0.0f,1.0f), XMFLOAT4(0.0f,1.0f,1.0f,1.0f) },
		{XMFLOAT3(1.0f,0.0f,1.0f), XMFLOAT4(1.0f,0.0f,1.0f,1.0f) }
	};


	
	//Описывающая буфер структура
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 5; //размер буфера = размер одной вершины*5;
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
	//Создание буферра индексов
	//Создадим массив с данными
	WORD indices[] =
	{
		0,2,1,
		0,3,4,  //порядок отрисовки примитивов пирамиды
		0,1,3,
		0,4,2,

		1,2,3,
		2,4,3,
	};
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 18; //6 треугольников 18 вершин 6*3
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; //тип буфера - индексов
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	//Создаем глобальный объект буфера индекссов
	hr = g_pd3device->CreateBuffer(&bd,&InitData,&g_pIndexBuffer);
	if (FAILED(hr)) 
		return hr;
	//усtановка буфера вершин
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	//установка буфера индексов
	
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	//установка способа отрисовки вершин в буфре
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Создание константного буфера
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3device->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
	if (FAILED(hr)) return hr;

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

HRESULT InitMatrixes()
{
	RECT rc;
	GetClientRect(main_window_handle, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	g_World = XMMatrixIdentity(); //инициализируем матрицу мира
	// инициализируем матрицу вида
	XMVECTOR Eye = XMVectorSet(0.0f,2.0f,-9.0f, 0.0f); //откуда смотрим
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); //Куда смотрим
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Направление верха
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	//инициализация матрицы проекции

	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
	return S_OK;
}

void SetMatrixes(float angle)
{
	static float t = 0.0f;
	
	if (g_drivertype == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI*0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

	if (ViewAngle > XM_2PI) ViewAngle -= XM_2PI;

	ViewAngle += (float)XM_PI*0.00025;
	XMMATRIX mPos = XMMatrixRotationY(-t + angle);
	XMMATRIX mSpin = XMMatrixRotationY(2 * t);
	XMMATRIX mTrans = XMMatrixTranslation(-3.0f, 0.f, 0.f);
	XMMATRIX mScale = XMMatrixScaling(0.5f+pulse,0.5f+pulse,0.5f+pulse);
	if (Bpulse && pulse <0.3f ) pulse += 0.00004f;
	else Bpulse = false;


	if (!Bpulse && pulse > 0.0f) pulse -= 0.00004f;
	else Bpulse = true;

	
	float x = -12.0*cos(t);
	float y = -12.0*sin(t);
	XMVECTOR Eye = XMVectorSet(0.0f, y , x , 0.0f); //откуда смотрим
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); //Куда смотрим
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Направление верха
	g_View = XMMatrixLookAtLH(Eye, At, Up);
	
	//g_World = XMMatrixRotationY(t);

	// обновляем констатный буффер создадим структуру и кинем ее в наш буфер

	g_World = mScale * mSpin*mTrans*mPos;

	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(g_World);
	cb.mView = XMMatrixTranspose(g_View);
	cb.mProjection = XMMatrixTranspose(g_Projection);
	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);

}
