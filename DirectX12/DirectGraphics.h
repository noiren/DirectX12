#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")



class DirectGraphics
{
public:

	DirectGraphics();

	~DirectGraphics(){}

	void EnableDebugLayer();

	bool Init(HWND& hwnd);

	void Release();

	bool ResetCommandAllocator();

	void Render();

private:

	// èâä˙âªån
	bool CreateDevice();
	bool CreateCommand();
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND& hwnd);
	bool CreateDiscriptorHeap();
	void CreateRenderTargetView();
	bool CreateFence();
	bool SetupSwapChain();

private:
	ID3D12Device* m_device;
	IDXGIFactory6* m_dxgiFactory;
	IDXGISwapChain4* m_swapChain;

	ID3D12CommandAllocator* m_cmdAllocator;
	ID3D12GraphicsCommandList* m_cmdList;
	ID3D12CommandQueue* m_cmdQueue;
	ID3D12DescriptorHeap* m_rtvHeaps;
	std::vector<ID3D12Resource*> m_backBuffers;
	ID3D12Fence* m_fence;
	UINT64 m_fenceVal;
};