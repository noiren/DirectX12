#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <SpriteFont.h>
#include <ResourceUploadBatch.h>
#include "wrl.h"
#include "DirectInput.h"

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"DirectXTK12.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX;
using namespace Microsoft::WRL;
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

	void RenderText();

	void SetRotate();

	struct Vertex {
		XMFLOAT3 pos; // xyz座標
		XMFLOAT2 uv;  // uv座標
	};

	struct VertexObj {
		XMFLOAT3 pos;		// 頂点座標
		XMFLOAT3 normal;	// 法線ベクトル
		XMFLOAT2 uv;		// uv座標
	};

	struct MatricesData {
		XMMATRIX world;
		XMMATRIX viewproj;
	};

	struct TexRGBA {
		unsigned char R, G, B, A;
	};

private:

	// 初期化系
	void LoadObj();
	bool CreateSpriteBatch();
	void CreateDirectInput(HWND& hwnd);
	bool CreateDevice();
	bool CreateCommand();
	bool CreateCommandQueue();
	bool CreateSwapChain(HWND& hwnd);
	bool CreateDiscriptorHeap();
	void CreateRenderTargetView();
	bool CreateFence();
	bool SetupSwapChain();

	bool CreateVertexBuffer();
	bool CreateTextureBuffer();
	bool CreateConstantBuffer();
	bool CreateDepthBuffer();
	bool CreateShaderConstResourceView();
	bool CreateDepthBufferView();
	bool CreateRootSignature();
	bool CreateShader();
	bool CreateInputLayout();

	void setViewPort();

private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGIFactory6> m_dxgiFactory;
	ComPtr<IDXGISwapChain4> m_swapChain;

	ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_cmdList;
	ComPtr<ID3D12CommandQueue> m_cmdQueue;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeaps;	// レンダーターゲットビュー用のディスクリプタヒープ
	std::vector<ID3D12Resource*> m_backBuffers;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceVal;

	ComPtr<ID3DBlob> m_pVsShader;
	ComPtr<ID3DBlob> m_pPsShader;

	ComPtr<ID3D12Resource> m_vertBuff; // 頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW m_vbView;// 頂点バッファビュー

	ComPtr<ID3D12Resource> m_indexBuff; // インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW m_ibView; //　インデックスバッファビュー

	ComPtr<ID3D12Resource> m_texBuff;	//	テクスチャバッファ
	ComPtr<ID3D12DescriptorHeap> m_pDescHeap;

	ComPtr<ID3D12Resource> m_constBuff; // 定数バッファ

	ComPtr<ID3D12Resource> m_depthBuff; // 深度バッファ
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap; // ディスクリプタヒープ

	XMMATRIX m_worldMat; // ワールド行列
	XMMATRIX m_viewMat; // ビュー行列
	XMMATRIX m_projMat; // プロジェクション行列

	MatricesData* m_constMapMatrix; // マップしたマップ行列

	ComPtr<ID3D12Resource> m_pUploadBuff; // アップロードリソース

	ComPtr<ID3D12PipelineState> m_pPipelineState;

	ComPtr<ID3D12RootSignature> m_pRootSignature;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorrect;

	DirectX::TexMetadata m_metadata;
	std::vector<VertexObj> m_vertex;

	float m_angleX;
	float m_angleY;
	XMFLOAT3 m_pos;
	float m_size;

	XMFLOAT3 m_eye;
	XMFLOAT3 m_target;

	Input* m_directInput;

	DirectX::GraphicsMemory* m_gmemory;
	DirectX::SpriteFont* m_spritefont;
	DirectX::SpriteBatch* m_spritebatch;
};