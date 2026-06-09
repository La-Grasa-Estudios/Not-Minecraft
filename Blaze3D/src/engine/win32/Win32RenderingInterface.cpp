#ifdef RENDERER_D3D11

#include "../RenderingInterface.h"
#include "../memory/MemoryAllocator.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb_image.h>

#include <fstream>
#include <vector>
#include <d3d11.h>
#include <comdef.h>
#include <stack>
#include <unordered_map>

#include "rsc/VertexShader.h"
#include "rsc/PixelShader.h"

static const size_t VERTEX_FORMAT_SIZEOF[16] =
{
	4,              // RI_VERTEX_FORMAT_FLOAT
	8,              // RI_VERTEX_FORMAT_FLOAT2
	12,             // RI_VERTEX_FORMAT_FLOAT3
	16,             // RI_VERTEX_FORMAT_FLOAT4
	4,              // RI_VERTEX_FORMAT_UINT8_4
	4,              // RI_VERTEX_FORMAT_UINT16_2
	8               // RI_VERTEX_FORMAT_UINT16_4
};

static const size_t VERTEX_COMPONENT_COUNT[16] =
{
	1,              // RI_VERTEX_FORMAT_FLOAT
	2,              // RI_VERTEX_FORMAT_FLOAT2
	3,              // RI_VERTEX_FORMAT_FLOAT3
	4,              // RI_VERTEX_FORMAT_FLOAT4
	4,              // RI_VERTEX_FORMAT_UINT8_4
	2,              // RI_VERTEX_FORMAT_UINT16_2
	4               // RI_VERTEX_FORMAT_UINT16_4
};

const DXGI_FORMAT VERTEX_FORMAT_D3D_TYPE[16] =
{
	DXGI_FORMAT_R32_FLOAT,          // RI_VERTEX_FORMAT_FLOAT
	DXGI_FORMAT_R32G32_FLOAT,          // RI_VERTEX_FORMAT_FLOAT2
	DXGI_FORMAT_R32G32B32_FLOAT,          // RI_VERTEX_FORMAT_FLOAT3
	DXGI_FORMAT_R32G32B32A32_FLOAT,          // RI_VERTEX_FORMAT_FLOAT4
	DXGI_FORMAT_R8G8B8A8_UNORM,  // RI_VERTEX_FORMAT_UINT8_4
	DXGI_FORMAT_R16G16_SNORM, // RI_VERTEX_FORMAT_UINT16_2
	DXGI_FORMAT_R16G16B16A16_UNORM  // RI_VERTEX_FORMAT_UINT16_4
};

const D3D11_COMPARISON_FUNC DEPTHFUNC_TO_D3D[] =
{
	D3D11_COMPARISON_LESS,
	D3D11_COMPARISON_LESS_EQUAL,
	D3D11_COMPARISON_EQUAL,
	D3D11_COMPARISON_GREATER_EQUAL,
	D3D11_COMPARISON_GREATER
};

const D3D11_CULL_MODE CULLMODE_TO_D3D[] =
{
	D3D11_CULL_NONE,
	D3D11_CULL_FRONT,
	D3D11_CULL_BACK
};

const D3D11_BLEND BLEND_TO_D3D[] =
{
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
};

#ifdef _WIN32

bool g_isFullscreen = false;
bool g_wantsClose = false;
int g_windowedX, g_windowedY, g_windowedWidth, g_windowedHeight;

struct riNativeData
{
	SDL_Window* pWindow;
};

struct Texture2D
{
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* view;

	~Texture2D()
	{
		view->Release();
		texture->Release();
	}
};

HWND hWnd;
ID3D11Device* pDevice;
ID3D11DeviceContext* pContext;
IDXGISwapChain* pSwapChain = NULL;

ID3D11Texture2D* pDepthStencil;
Texture2D* pWhiteTexture;

ID3D11RasterizerState* pFillRasterizerState;

ID3D11SamplerState* pBilinearSampler;
ID3D11SamplerState* pNearestSampler;

ID3D11RenderTargetView* pRenderTargetView;
ID3D11DepthStencilView* pDepthStencilView;

D3D11_TEXTURE2D_DESC RenderTargetDesc;
D3D11_TEXTURE2D_DESC StencilDesc;
DXGI_ADAPTER_DESC Adapter;
D3D11_VIEWPORT Viewport;

ID3D11Buffer* CameraCb;
ID3D11Buffer* LightingCb;
ID3D11Buffer* StreamingBuffer;
ID3D11Buffer* QuadsToTrisIndex;
size_t StreamingSize = 0;

ID3D11PixelShader* PixelShader;
ID3D11VertexShader* VertexShader;

ID3D11InputLayout* InputLayouts[8];

struct Light
{
	int Type;
	glm::vec3 Direction;
	bool Enabled;
	glm::vec3 Diffuse;
	glm::vec4 Ambient;
	glm::vec4 Specular;
};

struct RenderParameters
{
	bool LightingEnabled;
	glm::vec3 Padding1;
	Light Lights[8];
	bool FogEnabled;
	float FogStart;
	float FogEnd;
	float Padding2;
	glm::vec3 FogColor;
};

struct RenderStateMachine
{
	bool FogStateDirty = false;
	bool FogEnabled = false;
	float FogStart = 0.0f;
	float FogEnd = 100.0f;
	glm::vec3 FogColor = {};
	bool AlphaTest = false;
	bool BlendEnabled = false;
	bool TexturingEnabled = false;

	bool LightingStateDirty = false;
	bool LightingEnabled = false;
	Light Lights[8];

	bool ViewportDirty = false;
	int8_t BoundIA = -1;

	riBlendMode BlendSrcMode = BLEND_MODE_ONE;
	riBlendMode BlendDstMode = BLEND_MODE_ONE;

	riCullMode CullMode = RI_CULL_MODE_NONE;

	bool DepthStateDirty = false;
	bool DepthTest = false;
	riComparisonMode DepthMode = RI_COMPARISON_MODE_EQUAL;
	float DepthMin = 0.0f;
	float DepthMax = 1.0f;

	std::stack<glm::mat4> MatrixStack;

	glm::mat4 ProjectionMatrix;
	glm::mat4 ModelViewMatrix;
	size_t CurrentMatrixHash = 0;
	size_t DepthHash = 0;
	size_t RasterHash = 0;
	size_t BlendHash = 0;

	inline void Combine(std::size_t& seed, float v) {
		std::hash<float> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	size_t GetMatrixHash(const glm::mat4& matrix) {
		size_t seed = 0;
		const float* pSource = (const float*)glm::value_ptr(matrix);

		for (int i = 0; i < 16; ++i) {
			Combine(seed, pSource[i]);
		}
		return seed;
	}

	size_t GetMatrixStackHash()
	{
		return GetMatrixHash(ProjectionMatrix) ^ GetMatrixHash(ModelViewMatrix);
	}

	ID3D11ShaderResourceView* Textures[16];
	bool BoundTextures = false;
};

struct alignas(16) CameraParameters
{
	glm::mat4 ProjMatrix;
	glm::mat4 ModelViewMatrix;
	glm::mat4 NormalMatrix;
};

std::string MBFromW(LPCWSTR pwsz, UINT cp) {
	int cch = WideCharToMultiByte(cp, 0, pwsz, -1, 0, 0, NULL, NULL);

	char* psz = new char[cch];

	WideCharToMultiByte(cp, 0, pwsz, -1, psz, cch, NULL, NULL);

	std::string st(psz);
	delete[] psz;

	return st;
}

static RenderStateMachine g_StateMachine = {};
static std::unordered_map<size_t, ID3D11DepthStencilState*> g_DepthStates;
static std::unordered_map<size_t, ID3D11RasterizerState*> g_RasterStates;
static std::unordered_map<size_t, ID3D11BlendState*> g_BlendStates;

size_t GetDepthStencilStateHash(D3D11_DEPTH_STENCIL_DESC desc)
{
	size_t hash = 0;
	const uint8_t* p = reinterpret_cast<const uint8_t*>(&desc);
	for (size_t i = 0; i < sizeof(desc); ++i) {
		hash = (hash ^ p[i]) * 16777619ULL;
	}
	return hash;
}

size_t GetRasterizerStateHash(D3D11_RASTERIZER_DESC desc)
{
	size_t hash = 0;
	const uint8_t* p = reinterpret_cast<const uint8_t*>(&desc);
	for (size_t i = 0; i < sizeof(desc); ++i) {
		hash = (hash ^ p[i]) * 16777619ULL;
	}
	return hash;
}

size_t GetBlendStateHash(D3D11_BLEND_DESC desc)
{
	size_t hash = 0;
	const uint8_t* p = reinterpret_cast<const uint8_t*>(&desc);
	for (size_t i = 0; i < sizeof(desc); ++i) {
		hash = (hash ^ p[i]) * 16777619ULL;
	}
	return hash;
}

D3D11_BLEND GetAlphaValidBlend(D3D11_BLEND colorBlend)
{
	switch (colorBlend)
	{
	case D3D11_BLEND_SRC_COLOR:       return D3D11_BLEND_SRC_ALPHA;
	case D3D11_BLEND_INV_SRC_COLOR:   return D3D11_BLEND_INV_SRC_ALPHA;
	case D3D11_BLEND_DEST_COLOR:      return D3D11_BLEND_DEST_ALPHA;
	case D3D11_BLEND_INV_DEST_COLOR:  return D3D11_BLEND_INV_DEST_ALPHA;
	default: return colorBlend;
	}
}

void UpdateRenderState(int8_t ia)
{
	if (g_StateMachine.ViewportDirty)
	{
		pContext->RSSetViewports(1, &Viewport);
		g_StateMachine.ViewportDirty = false;
	}

	if (g_StateMachine.BoundIA != ia)
	{
		pContext->IASetInputLayout(InputLayouts[ia]);
		g_StateMachine.BoundIA = ia;
	}

	D3D11_DEPTH_STENCIL_DESC stencilDesc{};
	stencilDesc.DepthEnable = g_StateMachine.DepthTest;
	stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	stencilDesc.DepthFunc = DEPTHFUNC_TO_D3D[g_StateMachine.DepthMode];

	stencilDesc.StencilEnable = FALSE;
	stencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	stencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	auto depthHash = GetDepthStencilStateHash(stencilDesc);

	if (g_StateMachine.DepthHash != depthHash)
	{
		if (auto it = g_DepthStates.find(depthHash); it != g_DepthStates.end())
		{
			pContext->OMSetDepthStencilState(it->second, 0);
		}
		else
		{
			ID3D11DepthStencilState* state;
			pDevice->CreateDepthStencilState(&stencilDesc, &state);
			pContext->OMSetDepthStencilState(state, 0);
			g_DepthStates[depthHash] = state;
		}

		g_StateMachine.DepthHash = depthHash;
	}

	D3D11_RASTERIZER_DESC rasterizerState;
	ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerState.CullMode = CULLMODE_TO_D3D[g_StateMachine.CullMode];
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.DepthClipEnable = false;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.AntialiasedLineEnable = true;

	auto rsHash = GetRasterizerStateHash(rasterizerState);

	if (g_StateMachine.RasterHash != rsHash)
	{
		if (auto it = g_RasterStates.find(rsHash); it != g_RasterStates.end())
		{
			pContext->RSSetState(it->second);
		}
		else
		{
			ID3D11RasterizerState* state;
			pDevice->CreateRasterizerState(&rasterizerState, &state);
			pContext->RSSetState(state);
			g_RasterStates[rsHash] = state;
		}

		g_StateMachine.RasterHash = rsHash;
	}

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

	blendDesc.RenderTarget[0].BlendEnable = g_StateMachine.BlendEnabled;
	blendDesc.RenderTarget[0].SrcBlend = BLEND_TO_D3D[g_StateMachine.BlendSrcMode];
	blendDesc.RenderTarget[0].DestBlend = BLEND_TO_D3D[g_StateMachine.BlendDstMode];
	blendDesc.RenderTarget[0].SrcBlendAlpha = GetAlphaValidBlend(blendDesc.RenderTarget[0].SrcBlend);
	blendDesc.RenderTarget[0].DestBlendAlpha = GetAlphaValidBlend(blendDesc.RenderTarget[0].DestBlend);
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	auto blendHash = GetBlendStateHash(blendDesc);

	if (g_StateMachine.BlendHash != blendHash)
	{
		if (auto it = g_BlendStates.find(blendHash); it != g_BlendStates.end())
		{
			pContext->OMSetBlendState(it->second, nullptr, 0xffffffff);
		}
		else
		{
			ID3D11BlendState* state;
			pDevice->CreateBlendState(&blendDesc, &state);
			pContext->OMSetBlendState(state, nullptr, 0xffffffff);
			g_BlendStates[blendHash] = state;
		}

		g_StateMachine.BlendHash = blendHash;
	}

	auto matrixHash = g_StateMachine.GetMatrixStackHash();

	if (g_StateMachine.CurrentMatrixHash != matrixHash)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		CameraParameters params
		{
			.ProjMatrix = g_StateMachine.ProjectionMatrix,
			.ModelViewMatrix = g_StateMachine.ModelViewMatrix,
			.NormalMatrix = glm::transpose(glm::inverse(g_StateMachine.ModelViewMatrix))
		};

		HRESULT hr = pContext->Map(CameraCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		memcpy(mappedResource.pData, &params, sizeof(params));

		pContext->Unmap(CameraCb, 0);
		pContext->VSSetConstantBuffers(0, 1, &CameraCb);

		g_StateMachine.CurrentMatrixHash = matrixHash;
	}

	if (g_StateMachine.LightingStateDirty || g_StateMachine.FogStateDirty)
	{
		RenderParameters params{};
		memcpy(&params.Lights, &g_StateMachine.Lights, sizeof(params.Lights));
		params.LightingEnabled = g_StateMachine.LightingEnabled;

		params.FogEnabled = g_StateMachine.FogEnabled;
		params.FogStart = g_StateMachine.FogStart;
		params.FogEnd = g_StateMachine.FogEnd;
		params.FogColor = g_StateMachine.FogColor;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT hr = pContext->Map(LightingCb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		memcpy(mappedResource.pData, &params, sizeof(params));

		pContext->Unmap(LightingCb, 0);
		pContext->PSSetConstantBuffers(1, 1, &LightingCb);
		g_StateMachine.LightingStateDirty = false;
		g_StateMachine.FogStateDirty = false;
	}

	if (g_StateMachine.TexturingEnabled && !g_StateMachine.BoundTextures)
	{
		pContext->PSSetShaderResources(0, 16, g_StateMachine.Textures);
		g_StateMachine.BoundTextures = true;
	}

	if (!g_StateMachine.TexturingEnabled && g_StateMachine.BoundTextures)
	{
		ID3D11ShaderResourceView* white[16] =
		{
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
			pWhiteTexture->view,
		};
		pContext->PSSetShaderResources(0, 16, white);
		g_StateMachine.BoundTextures = false;
	}
}

std::vector<char> LoadShaderFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) return {};

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size)) {
		return buffer;
	}
	return {};
}

void riContext::Init()
{
	Width = 1280;
	Height = 720;
	NativeData = sysNew<riNativeData>();
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO))
	{
		auto error = SDL_GetError();
		printf_s("Ingaturroña: Failed to initialize SDL %s\n", error);
		exit(0);
	}

	NativeData->pWindow = SDL_CreateWindow("Blaze3D Window", Width, Height, SDL_WINDOW_RESIZABLE);

	SDL_SysWMinfo wmInfo{};
	SDL_version sdlver;
	SDL_VERSION(&sdlver);
	wmInfo.version = SDL_GetVersion(&sdlver);
	if (SDL_GetWindowWMInfo(NativeData->pWindow, &wmInfo, SDL_SYSWM_CURRENT_VERSION) != 0) {
		printf("Cant get native window handle: %s\n", SDL_GetError());
	}
	hWnd = wmInfo.info.win.window;

	UINT CreateDeviceFlags = 0;

	D3D_FEATURE_LEVEL feature_level;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	DXGI_SWAP_EFFECT swapEffects = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = swapEffects;

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_10_1,
	};

	UINT flags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&pSwapChain,
		&pDevice,
		&feature_level,
		&pContext);
	if (FAILED(hr)) {

		std::string error = "A D3D11-compatible GPU (Feature Level 11.1, Shader Model 5.0) is required to run the engine. HR: ";

		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();

		std::string fcid;

#ifdef UNICODE
		int wlen = lstrlenW(errMsg);
		int len = WideCharToMultiByte(CP_ACP, 0, errMsg, wlen, NULL, 0, NULL, NULL);
		fcid.resize(len);
		WideCharToMultiByte(CP_ACP, 0, errMsg, wlen, &fcid[0], len, NULL, NULL);
#else
		fcid = errMsg;
#endif

		error.append(fcid);

		MessageBoxA(NULL, error.c_str(), "Zircon fatal error", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	ID3D11Texture2D* pBackBuffer;

	if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer)))
	{
		MessageBoxA(NULL, "Failed to create back buffer!", "Zircon fatal error", MB_OK);
		exit(-1);
	}

	if (FAILED(pDevice->CreateRenderTargetView(pBackBuffer, NULL,
		&pRenderTargetView)))
	{
		MessageBoxA(NULL, "Failed to create render target!", "Zircon fatal error", MB_OK);
		exit(-1);
	}

	D3D11_RASTERIZER_DESC rasterizerState;
	ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.DepthClipEnable = false;
	rasterizerState.FrontCounterClockwise = true;

	pDevice->CreateRasterizerState(&rasterizerState, &pFillRasterizerState);

	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_WIREFRAME;

	//pDevice->CreateRasterizerState(&rasterizerState, &pLineRasterizerState);

	pContext->RSSetState(pFillRasterizerState);

	pBackBuffer->Release();

	RECT winRect;
	GetClientRect(hWnd, &winRect);
	ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = (float)winRect.right;
	Viewport.Height = (float)winRect.bottom;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	pContext->RSSetViewports(1, &Viewport);

	ZeroMemory(&RenderTargetDesc, sizeof(D3D11_TEXTURE2D_DESC));
	ZeroMemory(&StencilDesc, sizeof(D3D11_TEXTURE2D_DESC));

	StencilDesc.Width = winRect.right;
	StencilDesc.Height = winRect.bottom;
	StencilDesc.MipLevels = 1;
	StencilDesc.ArraySize = 1;
	StencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	StencilDesc.SampleDesc.Count = 1;
	StencilDesc.SampleDesc.Quality = 0;
	StencilDesc.Usage = D3D11_USAGE_DEFAULT;
	StencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	StencilDesc.CPUAccessFlags = 0;
	StencilDesc.MiscFlags = 0;

	pDevice->CreateTexture2D(&StencilDesc, NULL, &pDepthStencil);
	pDevice->CreateDepthStencilView(pDepthStencil, NULL, &pDepthStencilView);

	pContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

	// auto vs_blob = LoadShaderFile("VertexShader.cso"), ps_blob = LoadShaderFile("PixelShader.cso");

	hr = pDevice->CreateVertexShader(
		VertexShader_cso,
		sizeof(VertexShader_cso),
		NULL,
		&VertexShader);
	assert(SUCCEEDED(hr));

	hr = pDevice->CreatePixelShader(
		PixelShader_cso,
		sizeof(PixelShader_cso),
		NULL,
		&PixelShader);
	assert(SUCCEEDED(hr));

	pContext->VSSetShader(VertexShader, NULL, 0);
	pContext->PSSetShader(PixelShader, NULL, 0);

	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = (sizeof(CameraParameters) + (16 - sizeof(CameraParameters) % 16));
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;

	pDevice->CreateBuffer(&cbDesc, NULL, &CameraCb);
	pContext->VSSetConstantBuffers(0, 1, &LightingCb);

	cbDesc.ByteWidth = (sizeof(RenderParameters) + (16 - sizeof(RenderParameters) % 16));

	pDevice->CreateBuffer(&cbDesc, NULL, &LightingCb);
	pContext->PSSetConstantBuffers(1, 1, &LightingCb);

	D3D11_SAMPLER_DESC samplerDesc{};

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

	pDevice->CreateSamplerState(&samplerDesc, &pNearestSampler);

	pContext->PSSetSamplers(0, 1, &pNearestSampler);

	std::vector<uint16_t> indices;
	uint32_t maxQuads = 32000;
	indices.reserve(maxQuads * 6);

	for (uint32_t i = 0; i < maxQuads; i++)
	{
		uint16_t offset = (uint16_t)(i * 4);
		indices.push_back(offset + 0);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);
		indices.push_back(offset + 0);
		indices.push_back(offset + 2);
		indices.push_back(offset + 3);
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint16_t) * indices.size();
	bd.BindFlags |= D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData{};
	indexData.pSysMem = indices.data();

	if (FAILED(hr = pDevice->CreateBuffer(&bd, &indexData, &QuadsToTrisIndex))) {
		MessageBoxA(NULL, "Failed to create D3D11Buffer", "Fatal Error", MB_OK);
		exit(-1);
	}
	pContext->IASetIndexBuffer(QuadsToTrisIndex, DXGI_FORMAT_R16_UINT, 0);

	uint32_t color = 0xFFFFFFFF;
	pWhiteTexture = (Texture2D*)this->CreateTexture2D(1, 1, 1, RI_FORMAT_R8G8B8A8_UNORM, &color);
}

void riContext::Shutdown()
{
	SDL_DestroyWindow(NativeData->pWindow);
	sysFree(NativeData);
}

extern void SDL_EventCB(SDL_Event&);

void CreateDepthStencil(uint32_t width, uint32_t height)
{
	// 1. Cleanup existing resources if they exist
	//if (pDepthStencilView) pDepthStencilView->Release();
	if (pDepthStencil) pDepthStencil->Release();

	// 2. Describe the Texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24-bit depth, 8-bit stencil
	descDepth.SampleDesc.Count = 1;                   // Must match SwapChain multisampling
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	// 3. Create the Texture Resource
	HRESULT hr = pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
	if (FAILED(hr)) {
		// Handle Error
		return;
	}

	// 4. Describe the Depth Stencil View (DSV)
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	// 5. Create the View
	hr = pDevice->CreateDepthStencilView(pDepthStencil, &descDSV, &pDepthStencilView);
	if (FAILED(hr)) {
		// Handle Error
	}
}

void riContext::SwapBuffers()
{
	pSwapChain->Present(0, 0);

	int newWidth, newHeight;
	SDL_GetWindowSize(NativeData->pWindow, &newWidth, &newHeight);

	// Safety for minimized windows
	if (newWidth < 1) newWidth = 1;
	if (newHeight < 1) newHeight = 1;

	if (newWidth != Width || newHeight != Height)
	{
		Width = newWidth;
		Height = newHeight;

		// 2. Clear existing references to the back buffer
		pContext->OMSetRenderTargets(0, nullptr, nullptr);
		if (pRenderTargetView) pRenderTargetView->Release();
		if (pDepthStencilView) pDepthStencilView->Release();
		// If you have a DepthStencil Texture, Release() it here too

		// 3. Resize the SwapChain
		// Passing 0 for width/height tells D3D to use the window size automatically
		HRESULT hr = pSwapChain->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(hr)) {
			// Handle fatal error
		}

		// 4. Recreate the Render Target View
		ID3D11Texture2D* pBackBuffer = nullptr;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
		pBackBuffer->Release();

		// 5. Recreate Depth Stencil (If applicable)
		// You must recreate your Depth Buffer texture here because its 
		// dimensions must match the new SwapChain dimensions.
		CreateDepthStencil(Width, Height); // Helper function you likely have

		// 6. Update Viewport
		D3D11_VIEWPORT vp = { 0, 0, (float)Width, (float)Height, 0.0f, 1.0f };
		pContext->RSSetViewports(1, &vp);
	}

	// Always re-bind the targets for the start of the next frame
	pContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		{
			g_wantsClose = true;
		}
		SDL_EventCB(e);
	}

	static bool f11pressed = false;
	auto window = NativeData->pWindow;
}

bool riContext::ShouldClose()
{
	return g_wantsClose;
}

#endif

riHandle riContext::GetSurfaceHandle()
{
	return 0;
}

void riContext::SetVertexAttributeTable(riVertexAttributeTable table)
{
	// Leftover from before OpenGX, too late to change it
	m_ActiveVTA = table;
	for (int i = 0; i < 8; i++)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> desc;

		size_t sz = 0;
		if (table.Position[i].Enabled)
		{
			desc.emplace_back("POSITION", 0, VERTEX_FORMAT_D3D_TYPE[table.Position[i].Format], 0, table.Position[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0);
			sz += VERTEX_FORMAT_SIZEOF[table.Position[i].Format];
		}
		else
		{
			desc.emplace_back("POSITION", 0, DXGI_FORMAT_R16_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
		}
		if (table.Normal[i].Enabled)
		{
			desc.emplace_back("NORMAL", 0, DXGI_FORMAT_R8G8B8A8_SNORM, 0, table.Normal[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0);
			sz += VERTEX_FORMAT_SIZEOF[table.Normal[i].Format];
		}
		else
		{
			desc.emplace_back("NORMAL", 0, DXGI_FORMAT_R16_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
		}
		if (table.Color[i].Enabled)
		{
			desc.emplace_back("COLOR", 0, VERTEX_FORMAT_D3D_TYPE[table.Color[i].Format], 0, table.Color[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0);
			sz += VERTEX_FORMAT_SIZEOF[table.Color[i].Format];
		}
		else
		{
			desc.emplace_back("COLOR", 0, DXGI_FORMAT_R16_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
		}
		if (table.TexCoord[i].Enabled)
		{
			desc.emplace_back("TEXCOORD", 0, VERTEX_FORMAT_D3D_TYPE[table.TexCoord[i].Format], 0, table.TexCoord[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0);
			sz += VERTEX_FORMAT_SIZEOF[table.TexCoord[i].Format];
		}
		else
		{
			desc.emplace_back("TEXCOORD", 0, DXGI_FORMAT_R16_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
		}

		// auto vs_blob = LoadShaderFile("VertexShader.cso");

		pDevice->CreateInputLayout(
			desc.data(),
			desc.size(),
			VertexShader_cso,
			sizeof(VertexShader_cso),
			&InputLayouts[i]
		);

		m_ActiveVTA.Strides[i] = sz;
	}
}

riHandle riContext::LoadTexture(const char* path)
{
	std::string p = path;

	if (!p.ends_with("rtex"))
	{
		int w, h, c;
		auto data = stbi_load(path, &w, &h, &c, 4);
		if (data)
		{
			auto handle = CreateTexture2D(w, h, 1, RI_FORMAT_R8G8B8A8_UNORM, data);
			SetTextureFilter(handle, RI_FILTER_LINEAR);
			stbi_image_free(data);
			return handle;
		}
		else
		{
			return 0;
		}
	}

	int w, h, c;
	std::ifstream file(p, std::ios::binary);

	// Should make this load a dummy error texture like missing CS:S
	if (!file.is_open())
	{
		exit(0);
		return 0;
	}

	file.read((char*)&w, 4);
	file.read((char*)&h, 4);
	file.read((char*)&c, 4);

	if (c < 3)
	{
		exit(0);
	}

	size_t sz = w * h * c;
	std::vector<uint8_t> texData(sz);
	file.read((char*)texData.data(), sz);

	return CreateTexture2D(w, h, 1, RI_FORMAT_R8G8B8A8_UNORM, texData.data());
}

riHandle riContext::CreateTexture2D(uint32_t width, uint32_t height, uint32_t mipLevels, riFormat format, const void* pData)
{
	ID3D11Texture2D* texture = 0;
	Texture2D* texObj = new Texture2D();

	D3D11_TEXTURE2D_DESC desc{};

	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = pData;
	data.SysMemPitch = width * 4;

	pDevice->CreateTexture2D(&desc, &data, &texture);

	texObj->texture = texture;
	pDevice->CreateShaderResourceView(texture, NULL, &texObj->view);

	return static_cast<riHandle>(texObj);
}

void riContext::UpdateTexturePart(riHandle textureHandle, uint32_t xoff, uint32_t yoff, uint32_t width, uint32_t height, const void* data)
{
	// 1. Cast the handle to your internal Texture2D wrapper
	Texture2D* texInternal = (Texture2D*)textureHandle;
	if (!texInternal || !texInternal->texture) return;

	// 2. Define the destination region (Box)
	// In D3D11, a Box defines the 3D region (Left, Top, Front, Right, Bottom, Back)
	D3D11_BOX destBox;
	destBox.left = xoff;
	destBox.top = yoff;
	destBox.front = 0;
	destBox.right = xoff + width;
	destBox.bottom = yoff + height;
	destBox.back = 1;

	uint32_t bytesPerPixel = 4;
	uint32_t rowPitch = width * bytesPerPixel;
	uint32_t depthPitch = 0; // Not used for 2D textures

	pContext->UpdateSubresource(
		texInternal->texture,
		0,              
		&destBox,       
		data,           
		rowPitch,
		depthPitch      
	);
}

void riContext::SetTextureFilter(riHandle texture, riFilter filter)
{
	
}

void riContext::SetSync(bool sync)
{
	
}

void riContext::DestroyTexture2D(riHandle pTexture)
{
	if (!pTexture)
		return;
	
	Texture2D* texture = (Texture2D*)pTexture;
	delete texture;
}

struct BufferHandle
{
	size_t a;
	size_t b;
	ID3D11Buffer* buff;
};

riHandle riContext::CreateBuffer(uint32_t size, void* data)
{
	ID3D11Buffer* buffer;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;
	bd.BindFlags |= D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA rsc{};
	rsc.pSysMem = data;

	HRESULT hr;
	if (FAILED(hr = pDevice->CreateBuffer(&bd, &rsc, &buffer))) {
		MessageBoxA(NULL, "Failed to create D3D11Buffer", "Fatal Error", MB_OK);
		exit(-1);
	}

	BufferHandle* handle = new BufferHandle();
	handle->a = 0;
	handle->b = 0;
	handle->buff = buffer;

	return handle;
}

void riContext::DestroyBuffer(riHandle pBuffer)
{
	if (!pBuffer)
		return;

	BufferHandle* p = (BufferHandle*)pBuffer;
	ID3D11Buffer* buffer = p->buff;
	buffer->Release();
}

void riDevice::SetViewport(float orgX, float orgY, float width, float height)
{
	g_StateMachine.ViewportDirty = true;
	Viewport.TopLeftX = orgX;
	Viewport.TopLeftY = orgY;
	Viewport.Width = width;
	Viewport.Height = height;
}

bool ClientStates[4] = { false, false, false, false };

bool IsD3D11Buffer(void* ptr) {
	if (!ptr) return false;
	BufferHandle* p = (BufferHandle*)ptr;
	return !(p->a || p->b);
}

void riDevice::Draw(riHandle buffer, uint8_t vtfIndex, riPrimitiveType type, uint32_t count)
{
	UpdateRenderState(vtfIndex);
	UINT stride = m_Context->m_ActiveVTA.Strides[vtfIndex];
	size_t dataSize = count * stride;

	ID3D11Buffer* finalVB = nullptr;
	UINT finalOffset = 0;

	if (!IsD3D11Buffer(buffer))
	{
		static size_t CurrentOffset;

		if (!StreamingBuffer || dataSize > StreamingSize) {
			if (StreamingBuffer) StreamingBuffer->Release();

			StreamingSize = glm::max(dataSize, 64 * 1024 * 1024ULL);

			D3D11_BUFFER_DESC bd = {};
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = (UINT)StreamingSize;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			pDevice->CreateBuffer(&bd, NULL, &StreamingBuffer);
			CurrentOffset = 0;
		}

		D3D11_MAP mapType = D3D11_MAP_WRITE_NO_OVERWRITE;

		if (CurrentOffset + dataSize > StreamingSize) {
			mapType = D3D11_MAP_WRITE_DISCARD;
			CurrentOffset = 0;
		}

		D3D11_MAPPED_SUBRESOURCE mapped;
		if (SUCCEEDED(pContext->Map(StreamingBuffer, 0, mapType, 0, &mapped))) {
			memcpy((uint8_t*)mapped.pData + CurrentOffset, buffer, dataSize);
			pContext->Unmap(StreamingBuffer, 0);

			finalVB = StreamingBuffer;
			finalOffset = (UINT)CurrentOffset;
			CurrentOffset += (dataSize + 15) & ~15;
		}
	}
	else {
		BufferHandle* p = (BufferHandle*)buffer;
		finalVB = p->buff;
		finalOffset = 0;
	}

	pContext->IASetVertexBuffers(0, 1, &finalVB, &stride, &finalOffset);

	static D3D11_PRIMITIVE_TOPOLOGY boundtopology = (D3D11_PRIMITIVE_TOPOLOGY)0;

	D3D11_PRIMITIVE_TOPOLOGY topology;
	switch (type) {
	case RI_PRIMITIVE_TYPE_QUAD:
	case RI_PRIMITIVE_TYPE_TRIANGLES: topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
	case RI_PRIMITIVE_TYPE_LINE_LIST: topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;     break;
	default:                         topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;    break;
	}
	if (topology != boundtopology)
	{
		pContext->IASetPrimitiveTopology(topology);
		boundtopology = topology;
	}
	if (type == RI_PRIMITIVE_TYPE_QUAD) {
		pContext->DrawIndexed((count / 4) * 6, 0, 0);
	}
	else {
		pContext->Draw(count, 0);
	}
}

void riDevice::DrawIndexed(riHandle buffer, riHandle indexBuffer, uint8_t vtfIndex, riPrimitiveType type, uint32_t count)
{
	UpdateRenderState(vtfIndex);
}

void riDevice::SetProjectionMatrix(float* mtx)
{
	g_StateMachine.ProjectionMatrix = *reinterpret_cast<glm::mat4*>(mtx);
}

void riDevice::SetModelViewMatrix(float* mtx)
{
	g_StateMachine.ModelViewMatrix = *reinterpret_cast<glm::mat4*>(mtx);
}

void riDevice::PushMatrixStack()
{
	g_StateMachine.MatrixStack.push(g_StateMachine.ProjectionMatrix);
	g_StateMachine.MatrixStack.push(g_StateMachine.ModelViewMatrix);
}

void riDevice::PopMatrixStack()
{
	g_StateMachine.ModelViewMatrix = g_StateMachine.MatrixStack.top();
	g_StateMachine.MatrixStack.pop();
	g_StateMachine.ProjectionMatrix = g_StateMachine.MatrixStack.top();
	g_StateMachine.MatrixStack.pop();
}

void riDevice::ClearSurface(riHandle handle, float r, float g, float b, float a, bool depth, bool color)
{
	glm::vec4 clearColor = glm::vec4(r, g, b, a);

	if (color)
		pContext->ClearRenderTargetView(pRenderTargetView, (float*)&clearColor);

	if (depth)
		pContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void riDevice::SetDepthTest(bool enable, riComparisonMode mode)
{
	g_StateMachine.DepthStateDirty = true;
	g_StateMachine.DepthTest = enable;
	g_StateMachine.DepthMode = mode;
}

void riDevice::SetCullMode(riCullMode mode)
{
	g_StateMachine.CullMode = mode;
}

void riDevice::SetDepthRange(float zNear, float zFar)
{
	Viewport.MinDepth = zNear;
	Viewport.MaxDepth = zFar;
	g_StateMachine.ViewportDirty = true;
}

void riDevice::SetTexturing(bool enable)
{
	g_StateMachine.TexturingEnabled = enable;
}

void riDevice::SetFogStart(float start)
{
	g_StateMachine.FogStateDirty = true;
	g_StateMachine.FogStart = start;
}
void riDevice::SetFogEnd(float end)
{
	g_StateMachine.FogStateDirty = true;
	g_StateMachine.FogEnd = end;
}
void riDevice::SetFogColor(float r, float g, float b)
{
	g_StateMachine.FogStateDirty = true;
	g_StateMachine.FogColor = { r, g, b };
}
void riDevice::SetFog(bool enabled)
{
	g_StateMachine.FogStateDirty = true;
	g_StateMachine.FogEnabled = enabled;
}

void riDevice::SetLighting(bool enabled)
{
	g_StateMachine.LightingStateDirty = true;
	g_StateMachine.LightingEnabled = enabled;
}

void riDevice::SetSmoothShade(bool enabled)
{
	
}

void riDevice::SetAlphaTest(bool enabled)
{
	g_StateMachine.AlphaTest = enabled;
}

void riDevice::SetBlending(bool enabled)
{
	g_StateMachine.BlendEnabled = enabled;
}

void riDevice::SetBlendMode(riBlendMode src, riBlendMode dst)
{
	g_StateMachine.BlendSrcMode = src;
	g_StateMachine.BlendDstMode = dst;
}

void riDevice::SetLight(riLight* pLight, bool enabled, uint32_t idx)
{
	g_StateMachine.LightingStateDirty = true;
	g_StateMachine.Lights[idx].Enabled = enabled;
	if (enabled)
	{
		if (pLight && pLight->LightType == RI_LIGHT_TYPE_DIRECTIONAL)
		{
			auto ambient = pLight->Data.Directional.Ambient;
			auto diffuse = pLight->Data.Directional.Diffuse;
			auto specular = pLight->Data.Directional.Specular;
			auto dir = g_StateMachine.ModelViewMatrix * glm::vec4(pLight->Data.Directional.Direction, 0.0f);
			g_StateMachine.Lights[idx].Ambient = glm::vec4(ambient, 1.0f);
			g_StateMachine.Lights[idx].Diffuse = glm::vec4(diffuse, 1.0f);
			g_StateMachine.Lights[idx].Specular = glm::vec4(specular, 1.0f);
			g_StateMachine.Lights[idx].Direction = glm::normalize(dir);
		}
	}
}

void riDevice::SetTexture(riHandle texture, uint32_t slot)
{
	if (!texture)
	{
		g_StateMachine.Textures[slot] = pWhiteTexture->view;
		return;
	}
	Texture2D* tex = (Texture2D*)texture;
	pContext->PSSetShaderResources(0, 1, &tex->view);
	g_StateMachine.Textures[slot] = tex->view;
}

void riDevice::GetWindowSize(float* d)
{
	d[0] = m_Context->Width;
	d[1] = m_Context->Height;
}

void riDevice::Init(riContext* context)
{
	m_Context = context;
}

#endif