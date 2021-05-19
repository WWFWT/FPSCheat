#pragma once
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;
using namespace std;

#include "../imgui/imgui.h"
#include "../imgui/examples/imgui_impl_dx11.h"
#include "../imgui/examples/imgui_impl_win32.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define safe_release(p) if (p) { p->Release(); p = nullptr; } 
#define VMT_PRESENT (UINT)IDXGISwapChainVMT::Present
#define PRESENT_STUB_SIZE 5

#ifdef _DEBUG
#include <cstdio>
#define DBG_LOG( format, ... )   printf( format, ##__VA_ARGS__ )
#else
#define DBG_LOG(msg)
#endif // _DEBUG

typedef struct Box2D_ {
	float x;
	float y;
	float w;
	float h;
	float distance;
}Box2D, * PBox2D;

typedef struct Vector3D_ {
	float x;
	float y;
	float z;
}Vector3D, * PVector3D;


struct HandleData
{
	DWORD pid;
	HWND hWnd;
};

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct ConstantBuffer
{
	XMMATRIX mProjection;
};

enum class IDXGISwapChainVMT {
	QueryInterface,
	AddRef,
	Release,
	SetPrivateData,
	SetPrivateDataInterface,
	GetPrivateData,
	GetParent,
	GetDevice,
	Present,
	GetBuffer,
	SetFullscreenState,
	GetFullscreenState,
	GetDesc,
	ResizeBuffers,
	ResizeTarget,
	GetContainingOutput,
	GetFrameStatistics,
	GetLastPresentCount,
};

enum class ID3D11DeviceVMT {
	QueryInterface,
	AddRef,
	Release,
	CreateVideoDecoder,
	CreateVideoProcessor,
	CreateAuthenticatedChannel,
	CreateCryptoSession,
	CreateVideoDecoderOutputView,
	CreateVideoProcessorInputView,
	CreateVideoProcessorOutputView,
	CreateVideoProcessorEnumerator,
	GetVideoDecoderProfileCount,
	GetVideoDecoderProfile,
	CheckVideoDecoderFormat,
	GetVideoDecoderConfigCount,
	GetVideoDecoderConfig,
	GetContentProtectionCaps,
	CheckCryptoKeyExchange,
	SetPrivateData,
	SetPrivateDataInterface,
};

enum class ID3D11DeviceContextVMT {
	QueryInterface,
	AddRef,
	Release,
	GetDevice,
	GetPrivateData,
	SetPrivateData,
	SetPrivateDataInterface,
	VSSetConstantBuffers,
	PSSetShaderResources,
	PSSetShader,
	PSSetSamplers,
	VSSetShader,
	DrawIndexed,
	Draw,
	Map,
	Unmap,
	PSSetConstantBuffers,
	IASetInputLayout,
	IASetVertexBuffers,
	IASetIndexBuffer,
	DrawIndexedInstanced,
	DrawInstanced,
	GSSetConstantBuffers,
	GSSetShader,
	IASetPrimitiveTopology,
	VSSetShaderResources,
	VSSetSamplers,
	Begin,
	End,
	GetData,
	SetPredication,
	GSSetShaderResources,
	GSSetSamplers,
	OMSetRenderTargets,
	OMSetRenderTargetsAndUnorderedAccessViews,
	OMSetBlendState,
	OMSetDepthStencilState,
	SOSetTargets,
	DrawAuto,
	DrawIndexedInstancedIndirect,
	DrawInstancedIndirect,
	Dispatch,
	DispatchIndirect,
	RSSetState,
	RSSetViewports,
	RSSetScissorRects,
	CopySubresourceRegion,
	CopyResource,
	UpdateSubresource,
	CopyStructureCount,
	ClearRenderTargetView,
	ClearUnorderedAccessViewUint,
	ClearUnorderedAccessViewFloat,
	ClearDepthStencilView,
	GenerateMips,
	SetResourceMinLOD,
	GetResourceMinLOD,
	ResolveSubresource,
	ExecuteCommandList,
	HSSetShaderResources,
	HSSetShader,
	HSSetSamplers,
	HSSetConstantBuffers,
	DSSetShaderResources,
	DSSetShader,
	DSSetSamplers,
	DSSetConstantBuffers,
	CSSetShaderResources,
	CSSetUnorderedAccessViews,
	CSSetShader,
	CSSetSamplers,
	CSSetConstantBuffers,
	VSGetConstantBuffers,
	PSGetShaderResources,
	PSGetShader,
	PSGetSamplers,
	VSGetShader,
	PSGetConstantBuffers,
	IAGetInputLayout,
	IAGetVertexBuffers,
	IAGetIndexBuffer,
	GSGetConstantBuffers,
	GSGetShader,
	IAGetPrimitiveTopology,
	VSGetShaderResources,
	VSGetSamplers,
	GetPredication,
	GSGetShaderResources,
	GSGetSamplers,
	OMGetRenderTargets,
	OMGetRenderTargetsAndUnorderedAccessViews,
	OMGetBlendState,
	OMGetDepthStencilState,
	SOGetTargets,
	RSGetState,
	RSGetViewports,
	RSGetScissorRects,
	HSGetShaderResources,
	HSGetShader,
	HSGetSamplers,
	HSGetConstantBuffers,
	DSGetShaderResources,
	DSGetShader,
	DSGetSamplers,
	DSGetConstantBuffers,
	CSGetShaderResources,
	CSGetUnorderedAccessViews,
	CSGetShader,
	CSGetSamplers,
	CSGetConstantBuffers,
	ClearState,
	Flush,
	GetType,
	GetContextFlags,
	FinishCommandList,
};



class Util
{
public:
	static bool Hook(void* pSrc, void* pDst, size_t size);
	static bool WriteMem(void* pDst, char* pBytes, size_t size);
	static HWND FindMainWindow(DWORD dwPID);
	static BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam);
	static uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
	static bool WorldToScreen(int WinXMid, int WinYMid, uintptr_t MatrixAddr, Vector3D pos, PBox2D box, float EnemyHigth);
	static void AutoAim(float x, float y, float ScreenCenterX, float ScreenCenterY, int AimSpeed);
	static float GetDistance(float Target_X, float Target_Y, float ScreenCenterX, float ScreenCenterY);
};



