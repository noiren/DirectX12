#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <DirectXMath.h>
#include <DirectXTex.h>

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace DirectX;
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

	void Rotate();

	struct Vertex {
		XMFLOAT3 pos; // xyz座標
		XMFLOAT2 uv;  // uv座標
	};

	struct VertexObj {
		XMFLOAT3 pos;		// 頂点座標
		XMFLOAT3 normal;	// 法線ベクトル
		XMFLOAT2 uv;		// uv座標
	};

	struct TexRGBA {
		unsigned char R, G, B, A;
	};

private:

	// 初期化系
	void LoadObj();

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
	bool CreateShaderConstResourceView();
	bool CreateRootSignature();
	bool CreateShader();
	bool CreateInputLayout();

	void setViewPort();

private:
	ID3D12Device* m_device;
	IDXGIFactory6* m_dxgiFactory;
	IDXGISwapChain4* m_swapChain;

	ID3D12CommandAllocator* m_cmdAllocator;
	ID3D12GraphicsCommandList* m_cmdList;
	ID3D12CommandQueue* m_cmdQueue;
	ID3D12DescriptorHeap* m_rtvHeaps;	// レンダーターゲットビュー用のディスクリプタヒープ
	std::vector<ID3D12Resource*> m_backBuffers;
	ID3D12Fence* m_fence;
	UINT64 m_fenceVal;

	ID3DBlob* m_pVsShader;
	ID3DBlob* m_pPsShader;

	ID3D12Resource* m_vertBuff; // 頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW m_vbView;// 頂点バッファビュー

	ID3D12Resource* m_indexBuff; // インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW m_ibView; //　インデックスバッファビュー

	ID3D12Resource* m_texBuff;	//	テクスチャバッファ
	ID3D12DescriptorHeap* m_pDescHeap;

	ID3D12Resource* m_constBuff; // 定数バッファ

	XMMATRIX m_worldMat; // ワールド行列
	XMMATRIX m_viewMat; // ビュー行列
	XMMATRIX m_projMat; // プロジェクション行列

	XMMATRIX* m_constMapMatrix; // マップしたマップ行列

	ID3D12Resource* m_pUploadBuff; // アップロードリソース

	ID3D12PipelineState* m_pPipelineState;

	ID3D12RootSignature* m_pRootSignature;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorrect;

	DirectX::TexMetadata m_metadata;
	std::vector<VertexObj> m_vertex;

	float m_angle;
};