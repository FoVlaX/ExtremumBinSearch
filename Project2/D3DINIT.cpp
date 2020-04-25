#include "D3DINIT.h"
using namespace std;

int OBJECT::global_ids[1000] = { 0 }; //объявление глобальлных классовых переменных
int OBJECT::current_id = 0;
OBJECT* OBJECT::set_gg = NULL;
float D3DINIT::ViewDist =70.f;

XMVECTOR D3DINIT::g_Eye = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
XMVECTOR D3DINIT::g_Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
int OBJECT::spd = 25;
int LIGHT::currentidP = 0;
int LIGHT::currentidN = 0;
int LIGHT::idsP[100] = { 0 };
int LIGHT::idsN[100] = { 0 };
D3DINIT::D3DINIT(HWND mwh)
{
	main_window_handle = mwh;
}


D3DINIT::~D3DINIT(){
	CleanUpDevice();
}

bool D3DINIT::Initialization()
{
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
	return true;
}

HRESULT D3DINIT::InitDevice()
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
	sd.BufferDesc.Width = width; //shirina_bufera
	sd.BufferDesc.Height = height; //visota_buffera
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ФОРМАТ ПИКСЕЛЯ В БУФЕРЕ
	sd.BufferDesc.RefreshRate.Numerator = 60; // частота обновления экрана
	sd.BufferDesc.RefreshRate.Denominator = 1;// 
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //НАЗНАЧЕНИЕ БУФЕРА - peredniu? БУФЕР
	sd.OutputWindow = main_window_handle; //привязка к окну
	sd.SampleDesc.Count = 1.0f;
	sd.SampleDesc.Quality = 0.3f;
	sd.Windowed = TRUE; // оконный режим
	//
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_drivertype = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_drivertype, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
			&sd, &g_pSwapChain, &g_pd3device, &g_featurelevel, &g_pImmediateContext);
		if SUCCEEDED(hr) break; //если устройства созданы, то выходим из цикла
	}
	if FAILED(hr)
		return hr;
	//создаем задний буфер в SDK
	//RenderTargetOutput - это передний буфер, RenderTargetView - задний

	ID3D11Texture2D* pBackBuffer = NULL;//объект текстур, т.е. область памяти которуюю можно юзать для рисования как буфер глубин и как текстуру
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
	if FAILED(hr) 
		return hr;
	hr = g_pd3device->CreateRenderTargetView(pBackBuffer, NULL , &g_pRenderTargetView);
	pBackBuffer->Release();
	if FAILED(hr)
		return hr;

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

void D3DINIT::CleanUpDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState(); // чистим за собой
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

void D3DINIT::RenderStart()
{
	float ClearColor[4] = { 0.0f,0.6f,1.f,1.f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor); //очищаем задний буфер
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); //очищаем буфер глубин


	LIGHT::lightAll();

	

	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pConstantBufferLight); //1 - точка входа в констант буффер в шейдере
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pConstantBufferLight); //передаем инфу о свете в пиксельный и вершинный шейдеры
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pConstantBufferPointLight); //1 - точка входа в констант буффер в шейдере
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pConstantBufferPointLight); //передаем инфу о свете в пиксельный и вершинный шейдеры
}

void D3DINIT::SetGameSpeed(int spd) //spd  - кадров в секкунду;
{
	GameSpeed = 1000 / spd;
}

void D3DINIT::SetViewPoint(float x, float y, float z)
{
	viewPoint = XMVectorSet(x, y, z, 0.0f);
}

void D3DINIT::RenderEnd()
{
	g_pSwapChain->Present(0, 0); // копируем задний буфер вв передний собственно на экран
	Sleep(GameSpeed); // задержка
}

HRESULT D3DINIT::InitGeometry() //шейдеры и констынтный буффер
{
	HRESULT hr = S_OK;
	//компиляция вершинного шейдера из файла
	ID3DBlob* pVSBlob = NULL; //вспомогательный оъект просто место в оперативной памяти
	hr = CompileShaderFromFile("shaderVS.fx", "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "VS Don't compile file FX, Please, execute this porgram from directory with FX file", "ERROR", MB_OK);
		return hr;
	}
	
	//Create Vertex SHader
	hr = g_pd3device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	hr = CompileShaderFromFile("depth.fx", "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "VS Don't compile file FX, Please, execute this porgram from directory with FX file", "ERROR", MB_OK);
		return hr;
	}

	//Create Vertex SHader
	hr = g_pd3device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShaderSM);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	//определение шаблона вершин
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0,DXGI_FORMAT_R32G32B32_FLOAT,0,20,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	UINT numElements = ARRAYSIZE(layout);
	//создание шаблона вершин
	hr = g_pd3device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout); //установка шаблона в устройство рисования

	ID3DBlob* pPSBlob = NULL; //вспомогательный оъект просто место в оперативной памяти
	hr = CompileShaderFromFile("shaderPS.fx", "PS", "ps_5_0", &pPSBlob);
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

	hr = CompileShaderFromFile("shaderPS.fx", "PSfL", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "PS Don't compile file FX, Please, execute this porgram from directory with FX file", "ERROR", MB_OK);
		return hr;
	}

	//Create Vertex SHader
	hr = g_pd3device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderLightO);
	if (FAILED(hr))
	{
		pPSBlob->Release();
		return hr;
	}

	hr = CompileShaderFromFile("depth.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, "PS Don't compile file FX, Please, execute this porgram from directory with FX file", "ERROR", MB_OK);
		return hr;
	}

	//Create Vertex SHader
	hr = g_pd3device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderSM);
	if (FAILED(hr))
	{
		pPSBlob->Release();
		return hr;
	}


	//создание буфера вершин
	D3D11_BUFFER_DESC bd;
	

	//Создание константного буфера
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3device->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
	if (FAILED(hr)) 
		return hr;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBufferLight);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3device->CreateBuffer(&bd, NULL, &g_pConstantBufferLight);
	if (FAILED(hr))
		return hr;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBufferPointLight);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3device->CreateBuffer(&bd, NULL, &g_pConstantBufferPointLight);
	//загрузка текстуры из файла


	if (FAILED(hr)) return hr;
	return S_OK;
}

HRESULT D3DINIT::CompileShaderFromFile(LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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
			MessageBox(NULL, (const char*)pErrorBlob->GetBufferPointer(), "Error", NULL);
		}
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();
	return S_OK;
}

HRESULT D3DINIT::InitMatrixes()
{
	RECT rc;
	GetClientRect(main_window_handle, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	g_World = XMMatrixIdentity(); //инициализируем матрицу мира
	// инициализируем матрицу вида
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -70.0f, 0.0f); //откуда смотрим
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); //Куда смотрим
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Направление верха
	g_View = XMMatrixLookAtLH(Eye, At, Up); //матрица вида

	//инициализация матрицы проекции
	vLightDirs[0] = { -0.5f,0.5f,-0.5f,0.5f };
	vLightDirs[1] = { -0.5f,0.5f,-0.5f,1.f };
	//это свет
	vLightColors[0] = { 0.8f,1.f,1.f,1.f };
	vLightColors[1] = {0.8f, 1.f, 1.f, 1.f};

	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 500.0f); //проекция на экран

	return S_OK;
}

void D3DINIT::SetView(float angleY, float angleX)
{
	if (OBJECT::set_gg) {
		g_At = XMVectorSet(OBJECT::set_gg->x, OBJECT::set_gg->y, OBJECT::set_gg->z, 0.f); //получение положения объекта за которым закреплена камера
	}
	else {
		g_At = viewPoint;
	}
	if (angleY != 0 || angleX !=0) //если есть изменения в координатах мыши
	{
		if (angleY != 0) { //вращение вокруг оси У т.е. смещение мыши по х
			g_Eye = XMVector3Rotate(g_Eye, XMVectorSet(0.f, 1 * sin(angleY), 0.f, cos(angleY))); //поворот вектора направления на камеру вокруг вектора 0, 1, 0 т.е. оси У посредством задания кватериона вращения
			helpXas = XMVector3Rotate(helpXas, XMVectorSet(0.f, 1 * sin(angleY), 0.f, cos(angleY))); //поворот вспомагательной горизонтальной оси на тот же угол вокруг оси У
		}
		if (!(vertAng + angleX * 2.1 > XM_PIDIV2&& vertAng + angleX * 2.1 < 3 * XM_PIDIV2)) //ограничение поворота по вертикали границы -пи/2 пи/2
		{
			vertAng += angleX * 2; //Поворот вокруг вспомогательной оси также задается кватерион вращения

			g_Eye = XMVector3Rotate(g_Eye, XMVectorSet(XMVectorGetX(helpXas) * sin(angleX), XMVectorGetY(helpXas) * sin(angleX), XMVectorGetZ(helpXas) * sin(angleX), cos(angleX))); //врааащаем
		}
		if (vertAng > 2 * XM_PI) vertAng -= 2 * XM_PI;
		if (vertAng < 0) vertAng += 2 * XM_PI; //если вдруг понадобится убрать ограничение
	
	}

	g_Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); //задаем направление верха
	
	XMVECTOR Eye = D3DINIT::ViewDist*g_Eye+g_At;
	g_View = XMMatrixLookAtLH(Eye, g_At, g_Up); //устанавливаем новой положение камеры а если точнее матрицы вида
}

void D3DINIT::SetCamera()
{
	if (WININIT::mousepress)
	{

		if (show)
		{
			SetCursorPos(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
			show = !show;
		}
	}
	else
	{
		if (!show)
		{
			show = !show;
		}

	}

	POINT p;
	GetCursorPos(&p);

	if (WININIT::mousepress)
	{
		ang = 0.3 * (WINDOW_WIDTH / 2 - p.x) * XM_PI / 180;
		ang2 = 0.3 * (WINDOW_HEIGHT / 2 - p.y) * XM_PI / 180;
	}

	SetView(ang, ang2);

	if (WININIT::mousepress)
	{
		SetCursorPos(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
	}


}

void D3DINIT::SetPlayer(OBJECT* obj)
{
	OBJECT::set_gg = obj;
}

void D3DINIT::Render()
{
	RenderStart();
	SetCamera();
	OBJECT::drawall();
	RenderEnd();
}

OBJECT::OBJECT(char const* vertxt,  char const* texture, HRESULT &hr) //читаем из *.obj файла
{

	FILE* vtxt;

	vtxt = fopen(vertxt, "r");// открываем наши фалйлы
	lightOn = NULL;
	XMFLOAT3* vert = new XMFLOAT3;
	XMFLOAT2* vtext = new XMFLOAT2;
	XMFLOAT3* vnorm = new XMFLOAT3;

	char s[40];

	while (strcmp(s, "o"))
		fscanf(vtxt, "%s", &s);

	char objName[40];
	fscanf(vtxt, "%s", &objName);

	fscanf(vtxt, "%s", &s);
	int i = 1;
	while (!strcmp(s, "v")) // считываем координаты вершин
	{
		vert = (XMFLOAT3*)realloc((void*)vert, sizeof(XMFLOAT3) * i);
		fscanf(vtxt, "%f%f%f", &vert[i - 1].x, &vert[i - 1].y, &vert[i - 1].z);
		i++;
		fscanf(vtxt, "%s", &s);
	}
	i = 1;
	while (!strcmp(s, "vt")) //считываем координаты текстур
	{
		vtext = (XMFLOAT2*)realloc((void*)vtext, sizeof(XMFLOAT2) * i);
		fscanf(vtxt, "%f%f", &vtext[i - 1].x, &vtext[i - 1].y);
		i++;
		fscanf(vtxt, "%s", &s);
	}
	i = 1;
	while (!strcmp(s, "vn")) //считываем координаты нормалей
	{
		vnorm = (XMFLOAT3*)realloc((void*)vnorm, sizeof(XMFLOAT3) * i);
		fscanf(vtxt, "%f%f%f", &vnorm[i - 1].x, &vnorm[i - 1].y, &vnorm[i - 1].z);
		i++;
		fscanf(vtxt, "%s", &s);
	}
	while (strcmp(s,"f")) fscanf(vtxt, "%s", &s);
	i = 3;
	SimpleVertex* vercicles = new SimpleVertex;
	
	while (!feof(vtxt) && !strcmp(s,"f")) //заполняем структуру для вершинного буфера
	{
		vercicles = (SimpleVertex*)realloc((void*)vercicles, sizeof(SimpleVertex) * i);
		int x1, x2, x3,x4;
		int nx1, nx2, nx3, nx4;
		int tx1, tx2, tx3, tx4;
		
		fscanf(vtxt, "%i/%i/%i  %i/%i/%i  %i/%i/%i", &x1, &tx1, &nx1, &x2, &tx2, &nx2, &x3, &tx3, &nx3);
		
		vercicles[i - 3].Pos.x = vert[x1 - 1].x;
		vercicles[i - 3].Pos.y = vert[x1 - 1].y;
		vercicles[i - 3].Pos.z = vert[x1 - 1].z;
		vercicles[i - 2].Pos.x = vert[x2 - 1].x; 
		vercicles[i - 2].Pos.y = vert[x2 - 1].y;
		vercicles[i - 2].Pos.z = vert[x2 - 1].z;
		vercicles[i - 1].Pos.x = vert[x3 - 1].x;
		vercicles[i - 1].Pos.y = vert[x3 - 1].y;
		vercicles[i - 1].Pos.z = vert[x3 - 1].z;

		vercicles[i - 3].Normal.x = vnorm[nx1 - 1].x;
		vercicles[i - 3].Normal.y = vnorm[nx1 - 1].y;
		vercicles[i - 3].Normal.z = vnorm[nx1 - 1].z;
		vercicles[i - 2].Normal.x = vnorm[nx2 - 1].x;
		vercicles[i - 2].Normal.y = vnorm[nx2 - 1].y;
		vercicles[i - 2].Normal.z = vnorm[nx2 - 1].z;
		vercicles[i - 1].Normal.x = vnorm[nx3 - 1].x;
		vercicles[i - 1].Normal.y = vnorm[nx3 - 1].y;
		vercicles[i - 1].Normal.z = vnorm[nx3 - 1].z;

		vercicles[i - 3].Tex.x = vtext[tx1 - 1].x;
		vercicles[i - 3].Tex.y = -vtext[tx1 - 1].y;

		vercicles[i - 2].Tex.x = vtext[tx2 - 1].x;
		vercicles[i - 2].Tex.y = -vtext[tx2 - 1].y;

		vercicles[i - 1].Tex.x = vtext[tx3 - 1].x;
		vercicles[i - 1].Tex.y = -vtext[tx3 - 1].y;//одна поверхность за итерацию
		i += 3;
		

		fscanf(vtxt, "%s", &s);
		if (strcmp(s, "f"))
		{
			if (strcmp(s, "s")) {
				vercicles = (SimpleVertex*)realloc((void*)vercicles, sizeof(SimpleVertex) * i);

				sscanf(s, "%i/%i/%i", &x4, &tx4, &nx4);

				vercicles[i - 3].Pos.x = vert[x1 - 1].x;
				vercicles[i - 3].Pos.y = vert[x1 - 1].y;
				vercicles[i - 3].Pos.z = vert[x1 - 1].z;
				vercicles[i - 2].Pos.x = vert[x3 - 1].x;
				vercicles[i - 2].Pos.y = vert[x3 - 1].y;
				vercicles[i - 2].Pos.z = vert[x3 - 1].z;
				vercicles[i - 1].Pos.x = vert[x4 - 1].x;
				vercicles[i - 1].Pos.y = vert[x4 - 1].y;
				vercicles[i - 1].Pos.z = vert[x4 - 1].z;

				vercicles[i - 3].Normal.x = vnorm[nx1 - 1].x;
				vercicles[i - 3].Normal.y = vnorm[nx1 - 1].y;
				vercicles[i - 3].Normal.z = vnorm[nx1 - 1].z;
				vercicles[i - 2].Normal.x = vnorm[nx3 - 1].x;
				vercicles[i - 2].Normal.y = vnorm[nx3 - 1].y;
				vercicles[i - 2].Normal.z = vnorm[nx3 - 1].z;
				vercicles[i - 1].Normal.x = vnorm[nx4 - 1].x;
				vercicles[i - 1].Normal.y = vnorm[nx4 - 1].y;
				vercicles[i - 1].Normal.z = vnorm[nx4 - 1].z;

				vercicles[i - 3].Tex.x = vtext[tx1 - 1].x;
				vercicles[i - 3].Tex.y = -vtext[tx1 - 1].y;

				vercicles[i - 2].Tex.x = vtext[tx3 - 1].x;
				vercicles[i - 2].Tex.y = -vtext[tx3 - 1].y;

				vercicles[i - 1].Tex.x = vtext[tx4 - 1].x;  // 
				vercicles[i - 1].Tex.y = -vtext[tx4 - 1].y;  // одна поверхность за итерацию
				fscanf(vtxt, "%s", &s);
				i += 3;
			}
			else
			{
				while (!feof(vtxt) && strcmp(s, "f")) {
					fscanf(vtxt, "%s", &s);
					if (!strcmp(s, "off")) break;
				}
			}
		}

	}
	m = i-3; //общее колво вертексов
	/*int n = 0;
	fscanf(vtxt, "%i", &n);*/


	
	/*for (int i = 0; i < n; i++) //собственно заполн¤ем структуру
	{
		fscanf(vtxt, "%f", &vercicles[i].Pos.x);
		fscanf(vtxt, "%f", &vercicles[i].Pos.y);
		fscanf(vtxt, "%f", &vercicles[i].Pos.z);
		fscanf(vtxt, "%f", &vercicles[i].Tex.x);
		fscanf(vtxt, "%f", &vercicles[i].Tex.y);
		fscanf(vtxt, "%f", &vercicles[i].Normal.x);
		fscanf(vtxt, "%f", &vercicles[i].Normal.y);
		fscanf(vtxt, "%f", &vercicles[i].Normal.z);

	}

	fscanf(vtxt, "%i",&m);*/


	WORD* indexes = new WORD[m]; // массив порядка отрисовки в данном случае числа от 0 до м-1
	//ZeroMemory(indexes, sizeof(WORD)*m);
	/*for (int i = 0; i < m; i++)
	{
		fscanf(vtxt, "%i", &indexes[i]);
	}*/
	for (int k = 0; k < m; k++)
	{
		indexes[k] =k; //заполняем его
	}


	D3D11_BUFFER_DESC bd; //cтруктурка описывающа¤ наш буфер
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * m; //размер буфера = размер одной вершины*м;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData; // структура, содержаща¤ данные буфера;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vercicles; // указатель на наши вершины;

								  //—оздаем объект буффера вершин ID3D11Buffer
	hr = g_pd3device->CreateBuffer(&bd, &InitData, &pVertexBuffer);
	if (FAILED(hr))
		return;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * m; //м треугольников 18 вершин 6*3
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; //тип буфера - индексов
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indexes;
	//—оздаем глобальный объект буфера индекссов
	hr = g_pd3device->CreateBuffer(&bd, &InitData, &pIndexBuffer);
	if (FAILED(hr))
		return;


	delete[]vercicles;
	indexes = NULL;
	delete[]indexes;
	delete[]vert;
	delete[]vnorm;
	delete[]vtext;
	id = OBJECT::current_id;
	OBJECT::current_id++;
	OBJECT::global_ids[id] = int(this);
	fclose(vtxt);

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3device, texture, NULL, NULL, &pTextureRV, NULL);
	//
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;         // задаем координаты
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// создаем интерфейс сэмпла текстурировани¤
	hr = g_pd3device->CreateSamplerState(&sampDesc, &pSamplerLinear);

}
void OBJECT::step()
{
	
	/*е
		x += vx;
		y += vy;
		z += vz;
		xang += vxang;
		yang += vyang;
		zang += vzang;
	*/
	XMVECTOR vpmg = XMVectorSet(XMVectorGetX(D3DINIT::g_Eye), 0, XMVectorGetZ(D3DINIT::g_Eye), 0); //нормализуем проекцию вектора камеры на плосксть оХZ
	vpmg = XMVector3Normalize(vpmg); //вот тут нормаизуем
	float ang = acos(XMVectorGetX(vpmg)); // получаем угол по горизонтали
	float k = XMVectorGetY(D3DINIT::g_Up); // направление верха  получаем

	yang = XMVectorGetZ(D3DINIT::g_Eye) < 0 ? ang :  2*XM_PI -  ang; //разворачиваем оъект по направлению камеры по оси Y

	z += XMVectorGetZ(D3DINIT::g_Eye) < 0 ? speed * cos(ang - XM_PIDIV2): speed * cos(2*XM_PI-ang- XM_PIDIV2); //перемещение по направлению камеры в плскости оXZ
	x += XMVectorGetZ(D3DINIT::g_Eye) < 0 ? speed * sin(ang - XM_PIDIV2): speed * sin(2*XM_PI-ang- XM_PIDIV2);
	
}


OBJECT* OBJECT::getadress()
{
	return this;
}

void OBJECT::ReloadModel(char const* vertxt, char const* texture)
{
	HRESULT hr;
	FILE* vtxt;

	vtxt = fopen(vertxt, "r");// открываем наши фалйлы
	lightOn = NULL;
	XMFLOAT3* vert = new XMFLOAT3;
	XMFLOAT2* vtext = new XMFLOAT2;
	XMFLOAT3* vnorm = new XMFLOAT3;

	char s[40];

	while (strcmp(s, "o"))
		fscanf(vtxt, "%s", &s);

	char objName[40];
	fscanf(vtxt, "%s", &objName);

	fscanf(vtxt, "%s", &s);
	int i = 1;
	while (!strcmp(s, "v")) // считываем координаты вершин
	{
		vert = (XMFLOAT3*)realloc((void*)vert, sizeof(XMFLOAT3) * i);
		fscanf(vtxt, "%f%f%f", &vert[i - 1].x, &vert[i - 1].y, &vert[i - 1].z);
		i++;
		fscanf(vtxt, "%s", &s);
	}
	i = 1;
	while (!strcmp(s, "vt")) //считываем координаты текстур
	{
		vtext = (XMFLOAT2*)realloc((void*)vtext, sizeof(XMFLOAT2) * i);
		fscanf(vtxt, "%f%f", &vtext[i - 1].x, &vtext[i - 1].y);
		i++;
		fscanf(vtxt, "%s", &s);
	}
	i = 1;
	while (!strcmp(s, "vn")) //считываем координаты нормалей
	{
		vnorm = (XMFLOAT3*)realloc((void*)vnorm, sizeof(XMFLOAT3) * i);
		fscanf(vtxt, "%f%f%f", &vnorm[i - 1].x, &vnorm[i - 1].y, &vnorm[i - 1].z);
		i++;
		fscanf(vtxt, "%s", &s);
	}
	while (strcmp(s, "f")) fscanf(vtxt, "%s", &s);
	i = 3;
	SimpleVertex* vercicles = new SimpleVertex;

	while (!feof(vtxt) && !strcmp(s, "f")) //заполняем структуру для вершинного буфера
	{
		vercicles = (SimpleVertex*)realloc((void*)vercicles, sizeof(SimpleVertex) * i);
		int x1, x2, x3, x4;
		int nx1, nx2, nx3, nx4;
		int tx1, tx2, tx3, tx4;

		fscanf(vtxt, "%i/%i/%i  %i/%i/%i  %i/%i/%i", &x1, &tx1, &nx1, &x2, &tx2, &nx2, &x3, &tx3, &nx3);

		vercicles[i - 3].Pos.x = vert[x1 - 1].x;
		vercicles[i - 3].Pos.y = vert[x1 - 1].y;
		vercicles[i - 3].Pos.z = vert[x1 - 1].z;
		vercicles[i - 2].Pos.x = vert[x2 - 1].x;
		vercicles[i - 2].Pos.y = vert[x2 - 1].y;
		vercicles[i - 2].Pos.z = vert[x2 - 1].z;
		vercicles[i - 1].Pos.x = vert[x3 - 1].x;
		vercicles[i - 1].Pos.y = vert[x3 - 1].y;
		vercicles[i - 1].Pos.z = vert[x3 - 1].z;

		vercicles[i - 3].Normal.x = vnorm[nx1 - 1].x;
		vercicles[i - 3].Normal.y = vnorm[nx1 - 1].y;
		vercicles[i - 3].Normal.z = vnorm[nx1 - 1].z;
		vercicles[i - 2].Normal.x = vnorm[nx2 - 1].x;
		vercicles[i - 2].Normal.y = vnorm[nx2 - 1].y;
		vercicles[i - 2].Normal.z = vnorm[nx2 - 1].z;
		vercicles[i - 1].Normal.x = vnorm[nx3 - 1].x;
		vercicles[i - 1].Normal.y = vnorm[nx3 - 1].y;
		vercicles[i - 1].Normal.z = vnorm[nx3 - 1].z;

		vercicles[i - 3].Tex.x = vtext[tx1 - 1].x;
		vercicles[i - 3].Tex.y = -vtext[tx1 - 1].y;

		vercicles[i - 2].Tex.x = vtext[tx2 - 1].x;
		vercicles[i - 2].Tex.y = -vtext[tx2 - 1].y;

		vercicles[i - 1].Tex.x = vtext[tx3 - 1].x;
		vercicles[i - 1].Tex.y = -vtext[tx3 - 1].y;//одна поверхность за итерацию
		i += 3;


		fscanf(vtxt, "%s", &s);
		if (strcmp(s, "f"))
		{
			if (strcmp(s, "s")) {
				vercicles = (SimpleVertex*)realloc((void*)vercicles, sizeof(SimpleVertex) * i);

				sscanf(s, "%i/%i/%i", &x4, &tx4, &nx4);

				vercicles[i - 3].Pos.x = vert[x1 - 1].x;
				vercicles[i - 3].Pos.y = vert[x1 - 1].y;
				vercicles[i - 3].Pos.z = vert[x1 - 1].z;
				vercicles[i - 2].Pos.x = vert[x3 - 1].x;
				vercicles[i - 2].Pos.y = vert[x3 - 1].y;
				vercicles[i - 2].Pos.z = vert[x3 - 1].z;
				vercicles[i - 1].Pos.x = vert[x4 - 1].x;
				vercicles[i - 1].Pos.y = vert[x4 - 1].y;
				vercicles[i - 1].Pos.z = vert[x4 - 1].z;

				vercicles[i - 3].Normal.x = vnorm[nx1 - 1].x;
				vercicles[i - 3].Normal.y = vnorm[nx1 - 1].y;
				vercicles[i - 3].Normal.z = vnorm[nx1 - 1].z;
				vercicles[i - 2].Normal.x = vnorm[nx3 - 1].x;
				vercicles[i - 2].Normal.y = vnorm[nx3 - 1].y;
				vercicles[i - 2].Normal.z = vnorm[nx3 - 1].z;
				vercicles[i - 1].Normal.x = vnorm[nx4 - 1].x;
				vercicles[i - 1].Normal.y = vnorm[nx4 - 1].y;
				vercicles[i - 1].Normal.z = vnorm[nx4 - 1].z;

				vercicles[i - 3].Tex.x = vtext[tx1 - 1].x;
				vercicles[i - 3].Tex.y = -vtext[tx1 - 1].y;

				vercicles[i - 2].Tex.x = vtext[tx3 - 1].x;
				vercicles[i - 2].Tex.y = -vtext[tx3 - 1].y;

				vercicles[i - 1].Tex.x = vtext[tx4 - 1].x;  // 
				vercicles[i - 1].Tex.y = -vtext[tx4 - 1].y;  // одна поверхность за итерацию
				fscanf(vtxt, "%s", &s);
				i += 3;
			}
			else
			{
				while (!feof(vtxt) && strcmp(s, "f")) {
					fscanf(vtxt, "%s", &s);
					if (!strcmp(s, "off")) break;
				}
			}
		}

	}
	m = i - 3; //общее колво вертексов
	


	WORD* indexes = new WORD[m]; 
	for (int k = 0; k < m; k++)
	{
		indexes[k] = k; //заполняем его
	}


	D3D11_BUFFER_DESC bd; //cтруктурка описывающа¤ наш буфер
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * m; //размер буфера = размер одной вершины*м;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData; // структура, содержаща¤ данные буфера;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vercicles; // указатель на наши вершины;

								  //—оздаем объект буффера вершин ID3D11Buffer
	hr = g_pd3device->CreateBuffer(&bd, &InitData, &pVertexBuffer);
	if (FAILED(hr))
		return;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * m; //м треугольников 18 вершин 6*3
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; //тип буфера - индексов
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indexes;
	//—оздаем глобальный объект буфера индекссов
	hr = g_pd3device->CreateBuffer(&bd, &InitData, &pIndexBuffer);
	if (FAILED(hr))
		return;


	delete[]vercicles;
	indexes = NULL;
	delete[]indexes;
	delete[]vert;
	delete[]vnorm;
	delete[]vtext;
	
	fclose(vtxt);

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3device, texture, NULL, NULL, &pTextureRV, NULL);
	//
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;         // задаем координаты
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// создаем интерфейс сэмпла текстурировани¤
	hr = g_pd3device->CreateSamplerState(&sampDesc, &pSamplerLinear);
}

void OBJECT::setname(const char* nm)
{
	name = nm;
}

void OBJECT::drawall()
{
	OBJECT* o;
	for (int i = 0; i < OBJECT::current_id; i++)
	{
		o = (OBJECT*)OBJECT::global_ids[i];
		o->draw();
	}
	o = NULL;
}

void OBJECT::draw()
{
	// начиная отсюда в отрисовку бъекта
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	g_pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset); //в отрисовкку объекта пойдет это
	//установка буфера индексов
	g_pImmediateContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	//установка способа отрисовки вершин в буфре
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//ДАЛЕЕ ДЕЛАЕМ НЕООБХОДИМЫЕ МАТРИЧНЫЕ ТРАНСФОРМАЦИИ ДЛЯ ОТОБРАЖЕНИЯ ПОЛОЖЕНИЯ ОБЪЕКТА В МИРЕ
	XMMATRIX mSpinY = XMMatrixRotationY(yang); //поворачиваем смещенную от центра пираамиду на угол -т ++ англе
	XMMATRIX mSpinX = XMMatrixRotationX(xang);
	XMMATRIX mSpinZ = XMMatrixRotationZ(zang);// вращаем пирамиду вокруг воей оси
	XMMATRIX mTrans = XMMatrixTranslation(x, y, z); //смщаем пирамиду на -3 по оси х
	XMMATRIX mScale = XMMatrixScaling(xs,ys,zs); //меняем маштаб пирамиды
	g_World = mScale * mSpinY*mSpinX*mSpinZ*mTrans;
	ConstantBuffer cb;
	cb.mWVP = XMMatrixTranspose(g_World*g_View*g_Projection);
	cb.World = XMMatrixTranspose(g_World);
	cb.colorWhenNoLight = blend;
	


	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
	
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer); // 0 - точка входа в констант буффер в шейдере
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &pTextureRV); //передаем в шейдер текстуру в пиксельный
	g_pImmediateContext->PSSetSamplers(0, 1, &pSamplerLinear);

	if (lightOn)
	{
		if (lightOn->tl == pointLight)
		{
			lightOn->Pos.x = x;
			lightOn->Pos.y = y;
			lightOn->Pos.z = z;
		}
		g_pImmediateContext->PSSetShader(g_pPixelShaderLightO, NULL, 0);
	}

	g_pImmediateContext->DrawIndexed(m, 0, 0);
	if (lightOn)
	{
		g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	}
}



OBJECT::~OBJECT()
{
	for (int i = id; i < OBJECT::current_id - 1; i++)
	{
		OBJECT::global_ids[i] = OBJECT::global_ids[i + 1];
		OBJECT* a = (OBJECT*)OBJECT::global_ids[i];
		a->id = i;

	}
	OBJECT::current_id--;
	OBJECT::global_ids[OBJECT::current_id] = 0;
}

void LIGHT::lightAll()
{

	g_pImmediateContext->PSSetShader(g_pPixelShaderSM, NULL, 0); //устанавливаем пиксельный шейдер
	g_pImmediateContext->VSSetShader(g_pVertexShaderSM, NULL, 0); //устанавливаем вершинный шейдер

	LIGHT* o;

	ConstantBufferLight cbl;
	ConstantBufferPointLight cbpl;
	for (int i = 0; i < currentidP; i++)
	{
		o = (LIGHT*)idsP[i];
		o->setLight(cbl,cbpl);
	}
	ID3D11ShaderResourceView* depthMapTexture;     
	for (int i = 0; i < currentidN; i++)
	{
		o = (LIGHT*)idsN[i];
		o->setLight(cbl, cbpl);
	}
	cbl.vOutputColor = { (float)currentidN,(float)currentidP,0,0 };

	g_pImmediateContext->UpdateSubresource(g_pConstantBufferLight, 0, NULL, &cbl, 0, 0); //обновление констатных буфферов для дальнейшей передачи их в шейдеры
	g_pImmediateContext->UpdateSubresource(g_pConstantBufferPointLight, 0, NULL, &cbpl, 0, 0);
	o = NULL;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
	//настройка вьюпорта
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)WINDOW_WIDTH;
	vp.Height = (FLOAT)WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	//подключаем вьюпорт к контексту устроства
	g_pImmediateContext->RSSetViewports(1, &vp);

	// Передаем в шейдер оба sampler state

	ID3D11SamplerState* m_sampleStateClap;


	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Создаем sampler state для того чтобы 
// установить режим адресации текстуры как CLAMP
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;



	if (!FAILED(g_pd3device->CreateSamplerState(&samplerDesc, &m_sampleStateClap))) {

	}

	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0); //устанавливаем пиксельный шейдер
	g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0); //устанавливаем вершинный шейдер
	g_pImmediateContext->PSSetSamplers(1, 1, &m_sampleStateClap);
	g_pImmediateContext->PSSetShaderResources(1, 1, &cbl.tex[0]);
	g_pImmediateContext->VSSetSamplers(1, 1, &m_sampleStateClap);
	g_pImmediateContext->VSSetShaderResources(1, 1, &cbl.tex[0]);

}

LIGHT::LIGHT(XMFLOAT4 PosP,  XMFLOAT4 Color, typelight typeL, XMFLOAT4 lookAt)
{
	tl = typeL;

	if (tl == pointLight)
	{
		Pos = PosP;
		CurColor = Color;
		id = currentidP++;
	
		idsP[id] = (int)this;
	}
	if (tl == normalLight)
	{
		LookAt = lookAt;
		Pos = PosP;
		CurColor = Color;
		id = currentidN++;
		GenerateViewMatrix();
		GenerateProjectionMatrix(1.f, 500.f);
		idsN[id] = (int)this;
		InitShadow();
	}
	
}

void LIGHT::setLight(ConstantBufferLight& cbl, ConstantBufferPointLight& cbpl)
{
	if (tl == pointLight)
	{
		cbpl.vPos[id] = Pos;
		cbpl.vLightPointColor[id] = CurColor;
	}
	if (tl == normalLight)
	{
		cbl.vLightDir[id] = {  Pos.x - LookAt.x,  Pos.y - LookAt.y, Pos.z - LookAt.z ,1.f };
		cbl.vLightColor[id] = CurColor;
		cbl.VWPL[id] = XMMatrixTranspose(g_World*m_viewMatrix*m_projectionMatrix);
		cbl.tex[id] = RenderShadowMap();
	}
}

LIGHT::~LIGHT()
{
	if (tl == pointLight){
		for (int i = id; i < LIGHT::currentidP - 1; i++)
		{
			LIGHT::idsP[i] = LIGHT::idsP[i + 1];
			LIGHT* a = (LIGHT*)LIGHT::idsP[i];
			a->id = i;
		}
		LIGHT::currentidP--;
		LIGHT::idsP[LIGHT::currentidP] = 0;
	}

	if (tl == normalLight) {
		for (int i = id; i < LIGHT::currentidN - 1; i++)
		{
			LIGHT::idsN[i] = LIGHT::idsN[i + 1];
			LIGHT* a = (LIGHT*)LIGHT::idsN[i];
			a->id = i;

		}
		LIGHT::currentidN--;
		LIGHT::idsN[LIGHT::currentidN] = 0;
	}

}

void LIGHT::GenerateViewMatrix()
{
	XMVECTOR camPos = XMVectorSet(Pos.x, Pos.y, Pos.z, 0.0f);
	XMVECTOR camLookAt = XMVectorSet(LookAt.x,LookAt.y,LookAt.z, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_viewMatrix = XMMatrixLookAtLH(camPos, camLookAt, Up);
}

void LIGHT::GenerateProjectionMatrix(float screenNear, float screenDepth)
{
	float fieldOfView =XM_PIDIV2;
	float screenAspect = 1.0f;
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
}

bool LIGHT::InitShadow()
{
	// Сначала мы создаем текстуру в которую будем выводить shadow map

	// создаем текстуру размером SHADOWMAP_WIDTH х SHADOWMAP_HEIGHT
	// данная текстура будет использоваться как render target (установили
	// флаг - D3D11_BIND_RENDER_TARGET)
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = SHADOWMAP_WIDTH;
	textureDesc.Height = SHADOWMAP_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	if (FAILED(g_pd3device->CreateTexture2D(&textureDesc, NULL, &m_RTTexture)))
		return false;

	// Создаем render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	if (FAILED(g_pd3device->CreateRenderTargetView(m_RTTexture, &renderTargetViewDesc, &m_RTV)))
		return false;

	// создаем shader resource view c ранее созданной текстуры
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	if (FAILED(g_pd3device->CreateShaderResourceView(m_RTTexture, &shaderResourceViewDesc, &m_SRV)))
		return false;

	// Создаем Depth Stencil View

	// Создаем текстуру глубины
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = SHADOWMAP_WIDTH;
	depthBufferDesc.Height = SHADOWMAP_HEIGHT;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	if (FAILED(g_pd3device->CreateTexture2D(&depthBufferDesc, NULL, &m_DSTexture)))
		return false;

	// создаем depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = depthBufferDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	if (FAILED(g_pd3device->CreateDepthStencilView(m_DSTexture, &depthStencilViewDesc, &m_DSV)))
		return false;

	// Задаем вьюпорт по размеру нашей текстуры
	m_viewport.Width = (float)SHADOWMAP_WIDTH;
	m_viewport.Height = (float)SHADOWMAP_HEIGHT;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;




	return true;
}

ID3D11ShaderResourceView* LIGHT::RenderShadowMap()
{
	g_pImmediateContext->OMSetRenderTargets(1, &m_RTV, m_DSV);
	// Устанавливаем вьюпорт
	g_pImmediateContext->RSSetViewports(1, &m_viewport);
	float color[4] = { 0.f, 0.f, 0.f, 1.f };
	g_pImmediateContext->ClearRenderTargetView(m_RTV, color);
	g_pImmediateContext->ClearDepthStencilView(m_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	XMMATRIX copyV = g_View;
	XMMATRIX copyP = g_Projection;

	g_View = m_viewMatrix;
	g_Projection = m_projectionMatrix;
	OBJECT::drawall();
	//g_View = copyV;
	//g_Projection = copyP;

	return m_SRV;
}

XMMATRIX LIGHT::GetViewMatrix()
{
	return m_viewMatrix;
}

XMMATRIX LIGHT::GetProjectionMatrix()
{
	return m_projectionMatrix;
}

void LIGHT::SetShadowMapSize(const int& x, const int& y)
{
	SHADOWMAP_WIDTH = x;
	SHADOWMAP_HEIGHT = y;
}

void LIGHT::GetShadowMapSize(int& x, int& y)
{
	x = SHADOWMAP_WIDTH;
	y = SHADOWMAP_HEIGHT;
}
