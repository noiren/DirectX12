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
		XMFLOAT3 pos; // xyz���W
		XMFLOAT2 uv;  // uv���W
	};

	struct VertexObj {
		XMFLOAT3 pos;		// ���_���W
		XMFLOAT3 normal;	// �@���x�N�g��
		XMFLOAT2 uv;		// uv���W
	};

	struct TexRGBA {
		unsigned char R, G, B, A;
	};

private:

	// �������n
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
	ID3D12DescriptorHeap* m_rtvHeaps;	// �����_�[�^�[�Q�b�g�r���[�p�̃f�B�X�N���v�^�q�[�v
	std::vector<ID3D12Resource*> m_backBuffers;
	ID3D12Fence* m_fence;
	UINT64 m_fenceVal;

	ID3DBlob* m_pVsShader;
	ID3DBlob* m_pPsShader;

	ID3D12Resource* m_vertBuff; // ���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW m_vbView;// ���_�o�b�t�@�r���[

	ID3D12Resource* m_indexBuff; // �C���f�b�N�X�o�b�t�@
	D3D12_INDEX_BUFFER_VIEW m_ibView; //�@�C���f�b�N�X�o�b�t�@�r���[

	ID3D12Resource* m_texBuff;	//	�e�N�X�`���o�b�t�@
	ID3D12DescriptorHeap* m_pDescHeap;

	ID3D12Resource* m_constBuff; // �萔�o�b�t�@

	XMMATRIX m_worldMat; // ���[���h�s��
	XMMATRIX m_viewMat; // �r���[�s��
	XMMATRIX m_projMat; // �v���W�F�N�V�����s��

	XMMATRIX* m_constMapMatrix; // �}�b�v�����}�b�v�s��

	ID3D12Resource* m_pUploadBuff; // �A�b�v���[�h���\�[�X

	ID3D12PipelineState* m_pPipelineState;

	ID3D12RootSignature* m_pRootSignature;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorrect;

	DirectX::TexMetadata m_metadata;
	std::vector<VertexObj> m_vertex;

	float m_angle;
};