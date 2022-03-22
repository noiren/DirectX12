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
		XMFLOAT3 pos; // xyz���W
		XMFLOAT2 uv;  // uv���W
	};

	struct VertexObj {
		XMFLOAT3 pos;		// ���_���W
		XMFLOAT3 normal;	// �@���x�N�g��
		XMFLOAT2 uv;		// uv���W
	};

	struct MatricesData {
		XMMATRIX world;
		XMMATRIX viewproj;
	};

	struct TexRGBA {
		unsigned char R, G, B, A;
	};

private:

	// �������n
	void LoadObj(std::string objName, std::vector<VertexObj>& object);
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

	//----- Pera�n������
	void CreatePeraResource();
	void CreatePeraViewes();
	void CreatePeraVertex();
	void CreatePeraIndexLayout();

	bool CreatePlaneVertexBuffer();
	bool CreatePlaneConstantBuffer();
	bool CreatePlanePipelineState();

	void RenderShadowMap();

	void setViewPort();

private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGIFactory6> m_dxgiFactory;
	ComPtr<IDXGISwapChain4> m_swapChain;

	ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_cmdList;
	ComPtr<ID3D12CommandQueue> m_cmdQueue;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeaps;	// �����_�[�^�[�Q�b�g�r���[�p�̃f�B�X�N���v�^�q�[�v
	std::vector<ID3D12Resource*> m_backBuffers;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceVal;

	ComPtr<ID3DBlob> m_pVsShader;
	ComPtr<ID3DBlob> m_pPsShader;

	ComPtr<ID3D12Resource> m_vertBuff; // ���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW m_vbView;// ���_�o�b�t�@�r���[

	ComPtr<ID3D12Resource> m_indexBuff; // �C���f�b�N�X�o�b�t�@
	D3D12_INDEX_BUFFER_VIEW m_ibView; //�@�C���f�b�N�X�o�b�t�@�r���[

	ComPtr<ID3D12Resource> m_texBuff;	//	�e�N�X�`���o�b�t�@
	ComPtr<ID3D12DescriptorHeap> m_pDescHeap;

	ComPtr<ID3D12Resource> m_constBuff; // �萔�o�b�t�@

	ComPtr<ID3D12Resource> m_depthBuff; // �[�x�o�b�t�@
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap; // �f�B�X�N���v�^�q�[�v

//-------------------------------------------------------------------------
	ComPtr<ID3D12Resource> m_PlaneVertBuff; // �O���E���h�p���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW m_PlaneVbView; // �O���E���h�p���_�o�b�t�@�r���[

	ComPtr<ID3D12Resource> m_PlaneIndexBuff; // �O���E���h�p�C���f�b�N�X�o�b�t�@
	D3D12_INDEX_BUFFER_VIEW m_PlaneIbView;

	ComPtr<ID3DBlob> m_PlaneVertexShader;
	ComPtr<ID3DBlob> m_PlanePixelShader;

	ComPtr<ID3D12Resource> m_PlaneConstBuff;
	ComPtr<ID3D12PipelineState> m_peraPipeline;

//-------------------------------------------------------------------------
	// TODO: �悭������Ȃ����� ��ŏ���
	ComPtr<ID3D12Resource> m_peraResource; // �������ݐ惊�\�[�X(�e�N�X�`���p���\�[�X)

	ComPtr<ID3D12DescriptorHeap> m_peraRTVHeap; // �����_�[�^�[�Q�b�g�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> m_peraSRVHeap; // �e�N�X�`���p�f�B�X�N���v�^�q�[�v

	ComPtr<ID3D12Resource> m_peraVB; // ���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW m_peraVBV; // ���_�o�b�t�@�r���[

	ComPtr<ID3DBlob> m_rsBlob;
	ComPtr<ID3D12RootSignature> m_peraRS;

//--------------------------------------------------------------------------
	XMMATRIX m_worldMat; // ���[���h�s��
	XMMATRIX m_viewMat; // �r���[�s��
	XMMATRIX m_projMat; // �v���W�F�N�V�����s��

	XMMATRIX m_planeWorldMat;
	XMMATRIX m_planeViewMat;
	XMMATRIX m_planeProjMat;

	MatricesData* m_constMapMatrix; // �}�b�v�����}�b�v�s��

	MatricesData* m_constPlaneMapMatrix; // �}�b�v�����O���E���h�s��

	ComPtr<ID3D12Resource> m_pUploadBuff; // �A�b�v���[�h���\�[�X

	ComPtr<ID3D12PipelineState> m_pPipelineState;

	ComPtr<ID3D12RootSignature> m_pRootSignature;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorrect;

	DirectX::TexMetadata m_metadata;

	std::vector<VertexObj> m_vertex; // �L���[�u�p
	std::vector<VertexObj> m_plane; // �O���E���h�p

	float m_angleX;
	float m_angleY;
	XMFLOAT3 m_pos;
	float m_size;

	float m_planeSize;
	XMFLOAT3 m_planePos;

	XMFLOAT3 m_eye;
	XMFLOAT3 m_target;

	Input* m_directInput;

	DirectX::GraphicsMemory* m_gmemory;
	DirectX::SpriteFont* m_spritefont;
	DirectX::SpriteBatch* m_spritebatch;
};