#define TINYOBJLOADER_IMPLEMENTATION
#include "DirectGraphics.h"
#include "tiny_obj_loader.h"
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <math.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

D3D12_RESOURCE_BARRIER BarrierDesc = {};
using namespace DirectX;
namespace {
	const float WINDOW_WIDTH = 1280.f;
	const float WINDOW_HEIGHT = 720.f;

	constexpr uint32_t shadow_difinition = 1024;
}

template <typename T> std::wstring toWstr(T tep)
{
	std::wstringstream ss;
	ss << tep;
	return ss.str();
};

std::vector<DirectGraphics::TexRGBA> textureData(256 * 256);

size_t indexSize = 0;

size_t planeIndexSize = 0;

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heapForSpriteFont;


DirectGraphics::VertexObj vertices[] =
{
	{{-1.f,-1.f,-1.f},{-1.f,-1.f,0.f},{0.0f,1.0f}}, // ????
	{{-1.f,1.f,-1.f},{-1.f,-1.f,0.f},{0.0f,0.0f}}, // ????
	{{1.f,-1.f,-1.f},{-1.f,-1.f,0.f},{1.0f,1.0f}}, // ????
	{{1.f,1.f,-1.f},{-1.f,-1.f,0.f},{1.0f,0.0f}}, // ????
};

DirectGraphics::Vertex pv[4] = {
	{{-1,-1,0.1},{0,1}}, // ????
	{{-1,1,0.1},{0,0}},  // ????
	{{1,-1,0.1},{1,1}},  // ?E??
	{{1,1,0.1},{1,0}}    // ?E??
};

DirectGraphics::DirectGraphics(): 
	m_device(nullptr),
	m_dxgiFactory(nullptr),
	m_swapChain(nullptr),
	m_cmdAllocator(nullptr),
	m_cmdList(nullptr),
	m_cmdQueue(nullptr),
	m_rtvHeaps(nullptr),
	m_backBuffers(),
	m_fence(nullptr),
	m_fenceVal(0),
	m_pVsShader(nullptr),
	m_pPsShader(nullptr),
	m_vertBuff(nullptr),
	m_indexBuff(nullptr),
	m_texBuff(nullptr),
	m_constBuff(nullptr),
	m_depthBuff(nullptr),
	m_dsvHeap(nullptr),
	m_worldMat(),
	m_viewMat(),
	m_projMat(),
	m_constMapMatrix(nullptr),
	m_pDescHeap(nullptr),
	m_pUploadBuff(nullptr),
	m_vbView(),
	m_ibView(),
	m_pPipelineState(nullptr),
	m_pRootSignature(nullptr),
	m_viewport(),
	m_scissorrect(),
	m_metadata(),
	m_vertex(),
	m_angleX(),
	m_angleY(),
	m_size(1.f),
	m_pos(),
	m_eye(),
	m_target(),
	m_directInput(nullptr),
	m_gmemory(nullptr),
	m_spritefont(nullptr),
	m_spritebatch(nullptr)
{
	// ?e?N?X?`???̃f?[?^????
	// ?????̓m?C?Y?摜?ɂȂ??͂??ł??B
	for (auto& rgba : textureData)
	{
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;
	}
};


bool DirectGraphics::Init(HWND& hwnd)
{
	m_planeSize = 1;

	m_planePos = XMFLOAT3(0.f, 0.f, 0.f);

	LoadObj("cube.obj",m_vertex);

	LoadObj("shadowPlane.obj", m_plane);

	CreateDirectInput(hwnd);

	if (CreateDevice() == false)
	{
		return false;
	}

	if (CreateCommand() == false)
	{
		return false;
	}

	if (CreateCommandQueue() == false)
	{
		return false;
	}

	if (CreateSwapChain(hwnd) == false)
	{
		return false;
	}

	if (CreateDiscriptorHeap() == false)
	{
		return false;
	}

	CreateRenderTargetView();

	if (CreateFence() == false)
	{
		return false;
	}

	if (CreateVertexBuffer() == false)
	{
		return false;
	}

	if (CreatePlaneVertexBuffer() == false)
	{
		return false;
	}

	if (CreateTextureBuffer() == false)
	{
		return false;
	}

	if (CreateConstantBuffer() == false)
	{
		return false;
	}

	if (CreatePlaneConstantBuffer() == false)
	{
		return false;
	}

	if (CreateDepthBuffer() == false)
	{
		return false;
	}

	if (CreateShaderConstResourceView() == false)
	{
		return false;
	}

	if (CreateDepthBufferView() == false)
	{
		return false;
	}

	if (CreateRootSignature() == false)
	{
		return false;
	}

	if (CreateShader() == false)
	{
		return false;
	}

	if (CreateInputLayout() == false)
	{
		return false;
	}

	if (CreatePlanePipelineState() == false)
	{
		return false;
	}
	
	if (CreateShadowPipelineState() == false)
	{
		return false;
	}

	setViewPort();

	CreateSpriteBatch();
	return true;
}




void DirectGraphics::Release()
{
	// ?????ȊO??ComPtr?ɂĊǗ?
	delete(m_directInput);
	delete(m_gmemory);
	delete(m_spritefont);
	delete(m_spritebatch);
}


void DirectGraphics::Render()
{
	
	if (SetupSwapChain() == false)
	{
		return;
	}
	SetRotate();
	
	m_swapChain->Present(0, 0);
	//m_gmemory->Commit(m_cmdQueue);
}

bool DirectGraphics::SetupSwapChain()
{
	// ?o?b?N?o?b?t?@?ɕ`?挋?ʂ?????????

	auto bbIdx = m_swapChain->GetCurrentBackBufferIndex(); // 0??1???A???Ă???

	if (bbIdx > 2)
	{
		return false;
	}

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // ?J??
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	   // ???Ɏw???Ȃ?
	BarrierDesc.Transition.pResource = m_backBuffers[bbIdx];   // ?o?b?N?o?b?t?@?[???\?[?X
	BarrierDesc.Transition.Subresource = 0;

	BarrierDesc.Transition.StateBefore
		= D3D12_RESOURCE_STATE_PRESENT;			// ???O??PRESENT????

	BarrierDesc.Transition.StateAfter
		= D3D12_RESOURCE_STATE_RENDER_TARGET;	// ?????烌???_?[?^?[?Q?b?g????

	m_cmdList->ResourceBarrier(1, &BarrierDesc);

	m_cmdList->SetPipelineState(m_shadowPipeline.Get());

	m_cmdList->SetGraphicsRootSignature(m_pRootSignature.Get()); // ???[?g?V?O?l?`???̐ݒ?

	m_cmdList->SetDescriptorHeaps(1, m_pDescHeap.GetAddressOf()); // ?f?B?X?N???v?^?q?[?v?̐ݒ?

	auto heapHandle = m_pDescHeap->GetGPUDescriptorHandleForHeapStart();

	m_cmdList->SetGraphicsRootDescriptorTable(
		0,
		heapHandle
	);

	auto handle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_cmdList->OMSetRenderTargets(0, nullptr, false, &handle);

	m_cmdList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ???C?g?[?x?e?N?X?`??
	m_cmdList->DrawIndexedInstanced(indexSize, 1, 0, 0, 0);

	m_cmdList->SetPipelineState(m_pPipelineState.Get()); // ?p?C?v???C???X?e?[?g?̐ݒ?

	auto rtvH = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	rtvH.ptr += bbIdx * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	m_cmdList->OMSetRenderTargets(1, &rtvH, true, &dsvH);

	float clearColor[] = { 1.0f,1.0f,0.0f,1.0f }; // ???F

	m_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr); // ???ʃN???A

	m_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_cmdList->RSSetViewports(1, &m_viewport);

	m_cmdList->RSSetScissorRects(1, &m_scissorrect);

	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// ?v???~?e?B?u?g?|???W?̐ݒ?

	m_cmdList->IASetVertexBuffers(0, 1, &m_vbView);

	m_cmdList->IASetIndexBuffer(&m_ibView);

	m_cmdList->DrawIndexedInstanced(indexSize, 1, 0, 0, 0);

	m_cmdList->SetPipelineState(m_peraPipeline.Get()); // ?p?C?v???C???X?e?[?g?̐ݒ?

	m_cmdList->IASetVertexBuffers(0, 1, &m_PlaneVbView);// ???_?o?b?t?@?r???[?̐ݒ?

	m_cmdList->IASetIndexBuffer(&m_PlaneIbView);// ?C???f?b?N?X?o?b?t?@?r???[?̐ݒ?(1???ɐݒ??ł????C???f?b?N?X?o?b?t?@?[??1????)

	m_cmdList->DrawIndexedInstanced(planeIndexSize, 1, 0, 0, 0);

	RenderText();

	// ?R?}???h???X?g?̎??s

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_cmdList->ResourceBarrier(1, &BarrierDesc);

	m_cmdList->Close(); // ???s?O?ɂ͕K???N???[?Y???邱?ƁI

	ID3D12CommandList* cmdLists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(1, cmdLists);

	m_cmdQueue->Signal(m_fence.Get(), ++m_fenceVal);

	if (m_fence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);

		m_fence->SetEventOnCompletion(m_fenceVal, event);

		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}

	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);

	return true;
}

void DirectGraphics::RenderText()
{
	std::wstring modelXYZ = L"modelPos :(" + toWstr(m_pos.x * 64.f) + L"," + toWstr(m_pos.y * 60.f) + L", 0)";
	std::wstring cameraEye = L"cameraEyePos :(" + toWstr(m_eye.x) + L"," + toWstr(m_eye.y) + L"," + toWstr(m_eye.z) + L")";
	std::wstring cameraTarget = L"cameraTargetPos :(" + toWstr(m_target.x) + L"," + toWstr(m_target.y) + L"," + toWstr(m_target.z) + L")";

	m_cmdList->SetDescriptorHeaps(1, m_heapForSpriteFont.GetAddressOf());
	m_spritebatch->Begin(m_cmdList.Get());
	m_spritefont->DrawString(m_spritebatch, modelXYZ.c_str(), DirectX::XMFLOAT2(0, 0), DirectX::Colors::Black);
	m_spritefont->DrawString(m_spritebatch, cameraEye.c_str(), DirectX::XMFLOAT2(0, 70), DirectX::Colors::Black);
	m_spritefont->DrawString(m_spritebatch, cameraTarget.c_str(), DirectX::XMFLOAT2(0, 140), DirectX::Colors::Black);
	m_spritebatch->End();

}

void DirectGraphics::SetRotate()
{
	// X?????????]
	if (m_directInput->CheckKey(static_cast<UINT>(DIK_8)))
	{
		m_angleX += 0.1f;
	}
	if (m_directInput->CheckKey(DIK_2))
	{
		m_angleX -= 0.1f;
	}

	// Y?????????]
	if (m_directInput->CheckKey(DIK_4))
	{
		m_angleY += 0.1f;
	}
	if (m_directInput->CheckKey(DIK_6))
	{
		m_angleY -= 0.1f;
	}


	// X?????s?ړ?
	if (m_directInput->CheckKey(DIK_LEFT))
	{
		m_pos.x -= 0.01f;
	}
	if (m_directInput->CheckKey(DIK_RIGHT))
	{
		m_pos.x += 0.01f;
	}

	// Y?????s?ړ?
	if (m_directInput->CheckKey(DIK_UP))
	{
		m_pos.y += 0.01f;
	}
	if (m_directInput->CheckKey(DIK_DOWN))
	{
		m_pos.y -= 0.01f;
	}

	if (m_directInput->CheckKey(DIK_D))
	{
		m_size -= 0.01f;
	}
	if (m_directInput->CheckKey(DIK_F))
	{
		m_size += 0.01f;
	}

	if (m_directInput->CheckKey(DIK_J))
	{
		m_planeSize -= 0.01f;
	}
	if (m_directInput->CheckKey(DIK_K))
	{
		m_planeSize += 0.01f;
	}

	m_worldMat = XMMatrixScaling(m_size, m_size, m_size) * XMMatrixRotationX(m_angleX * (XM_PI / 180)) * XMMatrixRotationY(m_angleY * (XM_PI / 180)) * XMMatrixTranslation(m_pos.x, m_pos.y, 0);

	auto eyePos = XMLoadFloat3(&m_eye);
	auto targetPos = XMLoadFloat3(&m_target);
	auto upVec = XMLoadFloat3(&m_up);
	auto light = XMFLOAT4(-1, 1, -1, 0);
	XMVECTOR lightVec = XMLoadFloat4(&light);

	auto lightPos = targetPos + XMVector3Normalize(lightVec) * XMVector3Length(XMVectorSubtract(targetPos, eyePos)).m128_f32[0];

	m_constMapMatrix->world = m_worldMat;
	m_constMapMatrix->viewproj = m_viewMat * m_projMat;
	m_constMapMatrix->lightCamera = XMMatrixLookAtLH(lightPos, targetPos, upVec) * XMMatrixOrthographicLH(40, 40, 1.0f, 100.f);

	m_planeWorldMat = XMMatrixScaling(3.5f, 3.5f, 3.5f) * XMMatrixRotationX(45 * (XM_PI / 180)) * XMMatrixRotationY(135 * (XM_PI / 180)) * XMMatrixTranslation(0.f, -3.15f, 0.f);
	m_constPlaneMapMatrix->world = m_planeWorldMat;
	m_constPlaneMapMatrix->viewproj = m_planeViewMat * m_planeProjMat;
}



void DirectGraphics::LoadObj(std::string objName, std::vector<VertexObj>& object)
{
	// ?e?X?g?R?[?h ?z???g?ɓǂݍ??߂??̂??m?F????

	std::string inputfile = objName;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string error;

	bool ret = tinyobj::LoadObj(
		&attrib,
		&shapes,
		&materials,
		&warn,
		&error,
		inputfile.c_str());

	if (!ret)
		exit(1);

	/*
		TODO:
		????attrib?̒??̒??_?͑S???????ɂȂ??Ă????̂ł?????XMFLOAT3??vector?ɓ????Ȃ????B
		normal??uv???????悤?ɂ??āA???ꂼ??index?????Q?Ƃł????悤?ɂ????B
	*/

	std::vector<XMFLOAT3> vec_vertex;
	std::vector<XMFLOAT3> vec_normal;
	std::vector<XMFLOAT2> vec_uv;

	// ???񒸓_??3?ŌŒ??Ƃ???

	const int VERTEX_NUM = 3;
	const int NORMAL_NUM = 3;
	const int UV_NUM = 2;

	int now_vertex_num = 0;
	std::vector<float> vec_temp;
	
	// ???_???W?擾
	XMFLOAT3 tempVertexFloat;
	for (auto vertex : attrib.vertices)
	{
		++now_vertex_num;

		switch (now_vertex_num) {
			case 1:
				tempVertexFloat.x = vertex;
				break;
			case 2:
				tempVertexFloat.y = vertex;
				break;
			case 3:
				tempVertexFloat.z = vertex;
				vec_vertex.push_back(tempVertexFloat);
				now_vertex_num = 0;
				break;
			default:
				break;
		}
	}

	// ?@???x?N?g???擾
	now_vertex_num = 0;
	XMFLOAT3 tempNormalFloat;
	for (auto normal : attrib.normals)
	{
		++now_vertex_num;

		switch (now_vertex_num) {
		case 1:
			tempNormalFloat.x = normal;
			break;
		case 2:
			tempNormalFloat.y = normal;
			break;
		case 3:
			tempNormalFloat.z = normal;
			vec_normal.push_back(tempNormalFloat);
			now_vertex_num = 0;
			break;
		default:
			break;
		}
	}

	// UV???W?擾
	now_vertex_num = 0;
	XMFLOAT2 tempUVFloat;
	for (auto texcoord : attrib.texcoords)
	{
		++now_vertex_num;

		switch (now_vertex_num) {
		case 1:
			tempUVFloat.x = texcoord;
			break;
		case 2:
			tempUVFloat.y = texcoord;
			vec_uv.push_back(tempUVFloat);
			now_vertex_num = 0;
			break;
		default:
			break;
		}
	}

	for (auto index : shapes[0].mesh.indices)
	{
		XMFLOAT3 vertexData = vec_vertex[index.vertex_index];
		XMFLOAT3 normalData = vec_normal[index.normal_index];
		XMFLOAT2 uvData		= vec_uv[index.texcoord_index];

		VertexObj obj = { vertexData,normalData,uvData };

		object.push_back(obj);
	}

	if (!ret)
		exit(1);
}

void DirectGraphics::CreateDirectInput(HWND& hwnd)
{
	m_directInput = new Input(hwnd);
}

bool DirectGraphics::CreateSpriteBatch()
{
	m_gmemory = new DirectX::GraphicsMemory(m_device.Get());

	DirectX::ResourceUploadBatch resUploadBatch(m_device.Get());
	resUploadBatch.Begin();
	DirectX::RenderTargetState rtState(
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_D32_FLOAT
	);
	DirectX::SpriteBatchPipelineStateDescription pd(rtState);

	m_spritebatch = new DirectX::SpriteBatch(m_device.Get(), resUploadBatch,pd);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ret;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(ret.ReleaseAndGetAddressOf()));
	if (result != S_OK)
	{
		return false;
	}

	m_heapForSpriteFont = ret;

	m_spritefont = new DirectX::SpriteFont(m_device.Get(), resUploadBatch, L"font/fonttest.spritefont", m_heapForSpriteFont->GetCPUDescriptorHandleForHeapStart(), m_heapForSpriteFont->GetGPUDescriptorHandleForHeapStart());
	auto future = resUploadBatch.End(m_cmdQueue.Get());

	//???ɏ????Ƃ???_fenceValue??0???????Ƃ??܂?
	m_cmdQueue->Signal(m_fence.Get(), ++m_fenceVal);
	//???̖??ߒ????ł?_fenceValue??1?ŁA
	//GetCompletedValue?͂܂?0?ł??B
	if (m_fence->GetCompletedValue() < m_fenceVal) {
		//?????܂??I?????ĂȂ??Ȃ??A?C?x???g?҂????s??
		//?????̂??߂̃C?x???g?H???Ƃ??̂??߂?_fenceValue
		auto event = CreateEvent(nullptr, false, false, nullptr);
		//?t?F???X?ɑ΂??āACompletedValue??_fenceValue??
		//?Ȃ??????w???̃C?x???g?𔭐????????Ƃ??????߁?
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		//???܂??C?x???g???????Ȃ?
		//???C?x???g???????????܂ő҂?
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	future.wait();

	m_spritebatch->SetViewport(m_viewport);

	return true;
}

bool DirectGraphics::CreateDevice()
{
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf()));

	std::vector<IDXGIAdapter*> adapters;

	IDXGIAdapter* tmpAdapter = nullptr;

	// ???p?\?ȃA?_?v?^?[?̗񋓂??s??
	for (int i = 0; m_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}

	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		// ?T???????A?_?v?^?[?̖??O???m?F
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	if (tmpAdapter == nullptr)
	{
		return false;
	}

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())) == S_OK)
		{
			featureLevel = lv;
			return true;
		}
	}
	return false;
}

bool DirectGraphics::CreateCommand()
{
	if (m_device != nullptr)
	{
		if (m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAllocator.ReleaseAndGetAddressOf())) == S_OK &&
			m_device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,m_cmdAllocator.Get(),nullptr,IID_PPV_ARGS(m_cmdList.ReleaseAndGetAddressOf())) == S_OK)
		{
			return true;
		}
	}
	return false;
}

bool DirectGraphics::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	cmdQueueDesc.NodeMask = 0;

	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(m_cmdQueue.ReleaseAndGetAddressOf())) == S_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DirectGraphics::CreateSwapChain(HWND& hwnd)
{
	// ?X???b?v?`?F?[???֘A?̐ݒ????s??
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = WINDOW_WIDTH;
	swapchainDesc.Height = WINDOW_HEIGHT;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;

	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	if (m_dxgiFactory->CreateSwapChainForHwnd(
		m_cmdQueue.Get(),
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)m_swapChain.ReleaseAndGetAddressOf()
	) == S_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DirectGraphics::CreateDiscriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // ?r???[?̃^?C?v???ݒ? ?????̓????_?[?^?[?Q?b?g?r???[
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;// ?f?B?X?N???v?^?͕\?E???̓???
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_rtvHeaps.ReleaseAndGetAddressOf())) == S_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DirectGraphics::CreateRenderTargetView()
{
	DXGI_SWAP_CHAIN_DESC swcDesc = {};

	auto result = m_swapChain->GetDesc(&swcDesc);

	m_backBuffers.clear();// ?ꉞ??????

	m_backBuffers.reserve(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	// SRGB?p?̃????_?[?^?[?Q?b?g?r???[?̐ݒ?
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // ?K???}?␳?A??
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		m_backBuffers.emplace_back(nullptr);
		m_swapChain->GetBuffer(static_cast<UINT>(idx),IID_PPV_ARGS(&m_backBuffers[idx]));

		m_device->CreateRenderTargetView(m_backBuffers[idx], &rtvDesc, handle);

		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);// ?????̕????????炵?Ď擾
	}
}

bool DirectGraphics::ResetCommandAllocator()
{
	if (m_cmdAllocator->Reset() == S_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void DirectGraphics::EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));

	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

bool DirectGraphics::CreateFence()
{
	if (m_fence == nullptr)
	{
		m_fenceVal = 0;

		auto result = m_device->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()));

		if (result == S_OK)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool DirectGraphics::CreateVertexBuffer()
{
	// ???_?̑傫?????̋󂫂????????ɍ???

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_vertex[0]) * m_vertex.size());

	if (FAILED(m_device->CreateCommittedResource(
		&heapProp, // UPLOAD?q?[?v?Ƃ??Ďg?p????
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // ?T?C?Y?ɉ????ēK?؂Ȑݒ??ɂ??Ă?????

		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vertBuff.ReleaseAndGetAddressOf()))))
	{
		return false;
	}

	DirectGraphics::VertexObj* vertMap = nullptr;

	if (FAILED(m_vertBuff->Map(0, nullptr, (void**)&vertMap)))// ?m?ۂ????o?b?t?@?̉??z?A?h???X???擾????
	{
		return false;
	}

	std::copy(std::begin(m_vertex), std::end(m_vertex), vertMap);// ???ۂɂ??̃A?h???X???ҏW??????OK!(?????͂??̃A?h???X?ɒ??_?????𗬂??????ł???)

	m_vertBuff->Unmap(0, nullptr);

	// ?o?b?t?@???쐬?????炻???ɍ??킹???r???[?̍쐬(??????vertexBufferview)
	m_vbView = {};

	m_vbView.BufferLocation = m_vertBuff->GetGPUVirtualAddress(); // ?o?b?t?@?̉??z?A?h???X
	m_vbView.SizeInBytes = sizeof(m_vertex[0]) * m_vertex.size();						// ?S?o?C?g??
	m_vbView.StrideInBytes = sizeof(m_vertex[0]);					// ?꒸?_?̃o?C?g??


	// ?????????C???f?b?N?X?r???[
	unsigned short indices[] = {
		0,1,2,
		3,4,5,
		6,7,8,
		9,10,11,
		12,13,14,
		15,16,17,
		18,19,20,
		21,22,23,
		24,25,26,
		27,28,29,
		30,31,32,
		33,34,35,
	};

	indexSize = sizeof(indices);

	CD3DX12_HEAP_PROPERTIES indexHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC indexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));

	if (FAILED(m_device->CreateCommittedResource(
		&indexHeapProp, // UPLOAD?q?[?v?Ƃ??Ďg?p????
		D3D12_HEAP_FLAG_NONE,
		&indexResourceDesc, // ?T?C?Y?ɉ????ēK?؂Ȑݒ??ɂ??Ă?????

		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_indexBuff.ReleaseAndGetAddressOf()))))
	{
		return false;
	}

	unsigned short* mappedIdx = nullptr;
	if (FAILED(m_indexBuff->Map(0, nullptr, (void**)&mappedIdx)))
	{
		return false;
	}

	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	m_indexBuff->Unmap(0, nullptr);

	m_ibView = {};

	m_ibView.BufferLocation = m_indexBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = sizeof(indices);


	return true;
}

bool DirectGraphics::CreatePlaneVertexBuffer()
{
	// ???_?̑傫?????̋󂫂????????ɍ???

	CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_plane[0]) * m_plane.size());

	if (FAILED(m_device->CreateCommittedResource(
		&heapProp, // UPLOAD?q?[?v?Ƃ??Ďg?p????
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // ?T?C?Y?ɉ????ēK?؂Ȑݒ??ɂ??Ă?????

		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_PlaneVertBuff.ReleaseAndGetAddressOf()))))
	{
		return false;
	}

	DirectGraphics::VertexObj* vertMap = nullptr;

	if (FAILED(m_PlaneVertBuff->Map(0, nullptr, (void**)&vertMap)))// ?m?ۂ????o?b?t?@?̉??z?A?h???X???擾????
	{
		return false;
	}

	std::copy(std::begin(m_plane), std::end(m_plane), vertMap);// ???ۂɂ??̃A?h???X???ҏW??????OK!(?????͂??̃A?h???X?ɒ??_?????𗬂??????ł???)

	m_PlaneVertBuff->Unmap(0, nullptr);

	// ?o?b?t?@???쐬?????炻???ɍ??킹???r???[?̍쐬(??????vertexBufferview)
	m_PlaneVbView = {};

	m_PlaneVbView.BufferLocation = m_PlaneVertBuff->GetGPUVirtualAddress(); // ?o?b?t?@?̉??z?A?h???X
	m_PlaneVbView.SizeInBytes = sizeof(m_plane[0]) * m_plane.size();						// ?S?o?C?g??
	m_PlaneVbView.StrideInBytes = sizeof(m_plane[0]);					// ?꒸?_?̃o?C?g??


	// ?????????C???f?b?N?X?r???[
	unsigned short indices[] = {
		0,1,2,
		3,4,5,
	};

	planeIndexSize = sizeof(indices);

	CD3DX12_HEAP_PROPERTIES indexHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC indexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));

	if (FAILED(m_device->CreateCommittedResource(
		&indexHeapProp, // UPLOAD?q?[?v?Ƃ??Ďg?p????
		D3D12_HEAP_FLAG_NONE,
		&indexResourceDesc, // ?T?C?Y?ɉ????ēK?؂Ȑݒ??ɂ??Ă?????

		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_PlaneIndexBuff.ReleaseAndGetAddressOf()))))
	{
		return false;
	}

	unsigned short* mappedIdx = nullptr;
	if (FAILED(m_PlaneIndexBuff->Map(0, nullptr, (void**)&mappedIdx)))
	{
		return false;
	}

	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	m_PlaneIndexBuff->Unmap(0, nullptr);

	m_PlaneIbView = {};

	m_PlaneIbView.BufferLocation = m_PlaneIndexBuff->GetGPUVirtualAddress();
	m_PlaneIbView.Format = DXGI_FORMAT_R16_UINT;
	m_PlaneIbView.SizeInBytes = sizeof(indices);


	return true;

}

bool DirectGraphics::CreateConstantBuffer()
{
	XMMATRIX matrix = m_worldMat = XMMatrixRotationY(XM_PIDIV4);

	m_eye.x = 0;
	m_eye.y = 0;
	m_eye.z = -5;

	m_target = XMFLOAT3(0, 0, 0);

	m_up = XMFLOAT3(0, 1, 0);		// ???x?N?g??

	// ?r???[?s??
	m_viewMat = XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up));
	//matrix *= m_viewMat;

	// ?v???W?F?N?V?????s??
	m_projMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		1280.f / 720.f,
		1.0f,
		10.0f
	);
	//matrix *= m_projMat;

	CD3DX12_HEAP_PROPERTIES constHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC constResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatricesData) + 0xff) & ~0xff); // ?????I??256?̔{???ɂ?????

	// ?萔?o?b?t?@?̍쐬
	auto result = 
		m_device->CreateCommittedResource(
		&constHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&constResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_constBuff.ReleaseAndGetAddressOf()));

	if (result != S_OK)
	{
		return false;
	}

	result = m_constBuff->Map(0, nullptr, (void**)&m_constMapMatrix);
	m_constMapMatrix->world = m_worldMat;
	m_constMapMatrix->viewproj = m_viewMat * m_projMat;

	return true;
}

bool DirectGraphics::CreatePlaneConstantBuffer()
{
	XMMATRIX matrix = m_planeWorldMat = XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixRotationX(45 * (XM_PI / 180)) * XMMatrixRotationY(45 * (XM_PI / 180)) * XMMatrixTranslation(200.f, 200.f, 0);


	m_eye.x = 0;
	m_eye.y = 0;
	m_eye.z = -5;

	m_target = XMFLOAT3(0, 0, 0);

	XMFLOAT3 up(0, 1, 0);		// ???x?N?g??

	// ?r???[?s??
	m_planeViewMat = XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_target), XMLoadFloat3(&up));
	//matrix *= m_viewMat;

	// ?v???W?F?N?V?????s??
	m_planeProjMat = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		1280.f / 720.f,
		1.0f,
		10.0f
	);
	//matrix *= m_projMat;

	CD3DX12_HEAP_PROPERTIES constHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC constResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatricesData) + 0xff) & ~0xff); // ?????I??256?̔{???ɂ?????

	// ?萔?o?b?t?@?̍쐬
	auto result =
		m_device->CreateCommittedResource(
			&constHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&constResDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_PlaneConstBuff.ReleaseAndGetAddressOf()));

	if (result != S_OK)
	{
		return false;
	}

	result = m_PlaneConstBuff->Map(0, nullptr, (void**)&m_constPlaneMapMatrix);
	m_constPlaneMapMatrix->world = m_planeWorldMat;
	m_constPlaneMapMatrix->viewproj = m_planeViewMat * m_planeProjMat;
	return true;
}



bool DirectGraphics::CreateTextureBuffer()
{
	// ?e?N?X?`???̃??[?h

	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	ScratchImage scratchImg = {};

	result = LoadFromWICFile(
		L"inu.png",
		WIC_FLAGS_NONE,
		&m_metadata,
		scratchImg
	);

	if (result != S_OK)
	{
		return false;
	}

	auto img = scratchImg.GetImage(0, 0, 0);

	{
		// ???ԃo?b?t?@?Ƃ??ẴA?b?v???[?h?q?[?v?ݒ?
		D3D12_HEAP_PROPERTIES uploadHeapProp = {};
		uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

		uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		uploadHeapProp.CreationNodeMask = 0;
		uploadHeapProp.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Format = DXGI_FORMAT_UNKNOWN;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

		resDesc.Width = img->slicePitch;
		resDesc.Height = 1;
		resDesc.DepthOrArraySize = 1;
		resDesc.MipLevels = 1;

		resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;

		// ???ԃo?b?t?@?̍쐬

		result = m_device->CreateCommittedResource(
			&uploadHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, // CPU???珑?????݉\?????AGPU???猩???Ɠǂݎ??????p
			nullptr,
			IID_PPV_ARGS(m_pUploadBuff.ReleaseAndGetAddressOf())
		);

	}

	{
		// ?e?N?X?`???̂??߂̃q?[?v?ݒ?
		D3D12_HEAP_PROPERTIES texHeapProp = {};

		texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		texHeapProp.CreationNodeMask = 0;
		texHeapProp.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resDesc = {};

		resDesc.Format = m_metadata.format;
		resDesc.Width = m_metadata.width;
		resDesc.Height = m_metadata.height;
		resDesc.DepthOrArraySize = m_metadata.arraySize;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.MipLevels = m_metadata.mipLevels;
		resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(m_metadata.dimension);
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		result = m_device->CreateCommittedResource(
			&texHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, // ?R?s?[??
			nullptr,
			IID_PPV_ARGS(m_texBuff.ReleaseAndGetAddressOf())
		);

	}

	// ?A?b?v???[?h???\?[?X?փ}?b?v???s??
	uint8_t* mapforImg = nullptr;
	result = m_pUploadBuff->Map(0, nullptr, (void**)&mapforImg);

	std::copy_n(img->pixels, img->slicePitch, mapforImg);
	m_pUploadBuff->Unmap(0, nullptr);

	D3D12_TEXTURE_COPY_LOCATION src = {};

	// ?R?s?[??(?A?b?v???[?h??)?ݒ?
	src.pResource = m_pUploadBuff.Get(); // ???ԃo?b?t?@
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = m_metadata.width;
	src.PlacedFootprint.Footprint.Height = m_metadata.height;
	src.PlacedFootprint.Footprint.Depth = m_metadata.depth;
	src.PlacedFootprint.Footprint.RowPitch = img->rowPitch;
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};

	// ?R?s?[???̐ݒ?
	dst.pResource = m_texBuff.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	m_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	D3D12_RESOURCE_BARRIER BarrierDesc = {};

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = m_texBuff.Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore =
		D3D12_RESOURCE_STATE_COPY_DEST;
	BarrierDesc.Transition.StateAfter =
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_cmdList->ResourceBarrier(1, &BarrierDesc);
	m_cmdList->Close();

	ID3D12CommandList* cmdlists[] = { m_cmdList.Get() };

	m_cmdQueue->ExecuteCommandLists(1, cmdlists);

	m_cmdQueue->Signal(m_fence.Get(), ++m_fenceVal);

	if (m_fence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event); 
	}
	m_cmdAllocator->Reset();//?L???[???N???A
	m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);

	return true;
}

bool DirectGraphics::CreateDepthBuffer()
{
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension =
		D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = WINDOW_WIDTH;
	depthResDesc.Height = WINDOW_HEIGHT;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags =
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthResDesc.MipLevels = 1;
	depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResDesc.Alignment = 0;


	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	auto result = m_device->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(m_depthBuff.ReleaseAndGetAddressOf()));
	if (result != S_OK)
	{
		return false;
	}

	depthResDesc.Width = shadow_difinition;
	depthResDesc.Height = shadow_difinition;

	result = m_device->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(m_lightDepthBuffer.ReleaseAndGetAddressOf()));

	return true;
}

bool DirectGraphics::CreateShaderConstResourceView()
{
	// ?e?N?X?`?????萔?o?b?t?@?Ɋւ????r???[?̓f?B?X?N???v?^?q?[?v???ɍ???

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};

	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 5;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;// ?V?F?[?_?[???\?[?X?r???[?p

	auto result = m_device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(m_pDescHeap.ReleaseAndGetAddressOf()));

	if (result != S_OK)
	{
		return false;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = m_metadata.format; // RGBA(0.0f~1.0f?ɐ??K?????Ă???)
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	auto descHeapHandle = m_pDescHeap->GetCPUDescriptorHandleForHeapStart();

	m_device->CreateShaderResourceView(
		m_texBuff.Get(),	// ?r???[?Ɗ֘A?Â????o?b?t?@
		&srvDesc,	// ?e?N?X?`???ݒ?????
		descHeapHandle // ?q?[?v?̂ǂ??Ɋ??蓖?Ă邩
	);

	// ?L???[?u?p
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_constBuff->GetDesc().Width;
	/*
		GetCPUDescriptorHandleForHeapStart()?̓f?B?X?N???v?^?̓????Ԃ??B
		???{?r???[?̓f?B?X?N???v?^?q?[?v???ɘA???Œu?????邽?߈????̑傫???????????΃I?t?Z?b?g???\?ł????B
	*/
	descHeapHandle.ptr +=
		m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_device->CreateConstantBufferView(&cbvDesc, descHeapHandle);

	
	// Plane?p
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvPlaneDesc = {};
	cbvPlaneDesc.BufferLocation = m_PlaneConstBuff->GetGPUVirtualAddress();
	cbvPlaneDesc.SizeInBytes = m_PlaneConstBuff->GetDesc().Width;

	// CBV??OFFSET???u??
	descHeapHandle.ptr +=
		m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_device->CreateConstantBufferView(&cbvPlaneDesc, descHeapHandle);

	// ?V???h?E?}?b?v?p
	D3D12_SHADER_RESOURCE_VIEW_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	resDesc.Texture2D.MipLevels = 1;
	resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	// ?J?????[?x?l
	descHeapHandle.ptr +=
		m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	m_device->CreateShaderResourceView(m_depthBuff.Get(), &resDesc, descHeapHandle);

	// ???C?g?[?x?l
	descHeapHandle.ptr += 
		m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_device->CreateShaderResourceView(m_lightDepthBuffer.Get(), &resDesc, descHeapHandle);

	return true;
}

bool DirectGraphics::CreateDepthBufferView()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 2;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	auto result = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.ReleaseAndGetAddressOf()));

	if (result != S_OK)
	{
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	// ?ʏ??f?v?X
	m_device->CreateDepthStencilView(m_depthBuff.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// ???C?g?f?v?X

	auto handle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_device->CreateDepthStencilView(m_lightDepthBuffer.Get(), &dsvDesc, handle);

	return true;
}

bool DirectGraphics::CreateRootSignature()
{
	// ?f?B?X?N???v?^?????W?̍쐬
	D3D12_DESCRIPTOR_RANGE descTblRange[5] = {};

	// ?e?N?X?`???p???W?X?^?[0??
	descTblRange[0].NumDescriptors = 1; // ?????g?p?????e?N?X?`????1??
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // ???ʂ̓e?N?X?`??
	descTblRange[0].BaseShaderRegister = 0; // t0
	descTblRange[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ?萔?p???W?X?^?[0??
	descTblRange[1].NumDescriptors = 1;
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[1].BaseShaderRegister = 0; // b0
	descTblRange[1].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ?萔?p???W?X?^?[1??
	descTblRange[2].NumDescriptors = 1;
	descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[2].BaseShaderRegister = 1; // b1
	descTblRange[2].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ?e?N?X?`???p???W?X?^?[1??
	descTblRange[3].NumDescriptors = 1;
	descTblRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange[3].BaseShaderRegister = 1; // t1
	descTblRange[3].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ?e?N?X?`???p???W?X?^?[2??
	descTblRange[4].NumDescriptors = 1;
	descTblRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange[4].BaseShaderRegister = 2; // t2
	descTblRange[4].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	// ???[?g?p?????[?^?[?쐬
	D3D12_ROOT_PARAMETER rootparam = {};

	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	// ?z???擪?A?h???X???w??
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange[0];

	rootparam.DescriptorTable.NumDescriptorRanges = 5;

	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// ?T???v???[?ݒ??̍쐬
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor =
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	// ???[?g?V?O?l?`???????ō쐬
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr; // ???[?h?V?O?l?`???̃o?C?i???R?[?h???쐬????
	ID3DBlob* errorBlob = nullptr;

	auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	if (result != S_OK)
	{
		// ?G???[???b?Z?[?W?\??
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;
	}

	result = m_device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(m_pRootSignature.ReleaseAndGetAddressOf()));

	rootSigBlob->Release();

	return true;
}

bool DirectGraphics::CreateShader()
{
	ID3DBlob* errorBlob = nullptr;

	// vertexShader?̃R???p?C??
	auto result = 
		D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, m_pVsShader.ReleaseAndGetAddressOf(), &errorBlob);
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::OutputDebugStringA("?t?@?C?????????????܂???");
		return 0;
	}
	else if(result != S_OK)
	{
		// ?G???[???b?Z?[?W?\??
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),errorBlob->GetBufferSize(),errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;
	}
	
	// pixelShader?̃R???p?C??
	result = 
		D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, m_pPsShader.ReleaseAndGetAddressOf(), &errorBlob);
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::OutputDebugStringA("?t?@?C?????????????܂???");
		return 0;
	}
	else if(result != S_OK)
	{
		// ?G???[???b?Z?[?W?\??
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;

	}
}

bool DirectGraphics::CreateInputLayout()
{

	// ?????ł̏??񂪃V?F?[?_?[?ɓn?????邼?I

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		// ???_????
		{
			"POSITION",									// ?Z?}???e?B?N?X
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,				// ?t?H?[?}?b?g
			0,											// ???̓X???b?g?C???f?b?N?X
			D3D12_APPEND_ALIGNED_ELEMENT,				// ?f?[?^?̕??т???
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// ?꒸?_???ɂ??̃??C?A?E?g???????Ă???
			0
		},
		// ?@??????
		{
			"NORMAL",									// ?Z?}???e?B?N?X
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,				// ?t?H?[?}?b?g
			0,											// ???̓X???b?g?C???f?b?N?X
			D3D12_APPEND_ALIGNED_ELEMENT,				// ?f?[?^?̕??т???
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// ?꒸?_???ɂ??̃??C?A?E?g???????Ă???
			0
		},
		// UV????
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,					// ??????XMFLOAT2?Ȃ̂œ??Ԃ?
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	// ?????ŃO???t?B?b?N?X?p?C?v???C???X?e?[?g??????

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	// ???_?V?F?[?_?[?ƃs?N?Z???V?F?[?_?[???ݒ?
	gpipeline.VS.pShaderBytecode = m_pVsShader->GetBufferPointer();
	gpipeline.VS.BytecodeLength = m_pVsShader->GetBufferSize();
	gpipeline.PS.pShaderBytecode = m_pPsShader->GetBufferPointer();
	gpipeline.PS.BytecodeLength = m_pPsShader->GetBufferSize();

	// ?T???v???}?X?N?E???X?^???C?U?[?̐ݒ?
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	
	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // ?J?????O???Ȃ?
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ???g?̓h???Ԃ?
	gpipeline.RasterizerState.DepthClipEnable = true; // ?[?x?????̃N???b?s???O?͗L????
	//?c??
	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	// ?u?????h?X?e?[?g?̐ݒ?
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	// ???̓??C?A?E?g?̐ݒ?
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	gpipeline.pRootSignature = m_pRootSignature.Get();

	auto result = m_device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(m_pPipelineState.ReleaseAndGetAddressOf()));


	if (result == S_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DirectGraphics::CreatePlanePipelineState()
{
	// ?V?F?[?_?[?̍쐬???܂Ƃ߂?

	ID3DBlob* errorBlob = nullptr;

	// vertexShader?̃R???p?C??
	auto result =
		D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PlaneVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, m_PlaneVertexShader.ReleaseAndGetAddressOf(), &errorBlob);
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::OutputDebugStringA("?t?@?C?????????????܂???");
		return 0;
	}
	else if (result != S_OK)
	{
		// ?G???[???b?Z?[?W?\??
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;
	}

	// pixelShader?̃R???p?C??
	result =
		D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PlanePS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, m_PlanePixelShader.ReleaseAndGetAddressOf(), &errorBlob);
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::OutputDebugStringA("?t?@?C?????????????܂???");
		return 0;
	}
	else if (result != S_OK)
	{
		// ?G???[???b?Z?[?W?\??
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;

	}


	// ?????ł̏??񂪃V?F?[?_?[?ɓn?????邼?I

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		// ???_????
		{
			"POSITION",									// ?Z?}???e?B?N?X
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,				// ?t?H?[?}?b?g
			0,											// ???̓X???b?g?C???f?b?N?X
			D3D12_APPEND_ALIGNED_ELEMENT,				// ?f?[?^?̕??т???
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// ?꒸?_???ɂ??̃??C?A?E?g???????Ă???
			0
		},
		// ?@??????
		{
			"NORMAL",									// ?Z?}???e?B?N?X
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,				// ?t?H?[?}?b?g
			0,											// ???̓X???b?g?C???f?b?N?X
			D3D12_APPEND_ALIGNED_ELEMENT,				// ?f?[?^?̕??т???
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// ?꒸?_???ɂ??̃??C?A?E?g???????Ă???
			0
		},
		// UV????
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,					// ??????XMFLOAT2?Ȃ̂œ??Ԃ?
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	// ?????ŃO???t?B?b?N?X?p?C?v???C???X?e?[?g??????

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	// ???_?V?F?[?_?[?ƃs?N?Z???V?F?[?_?[???ݒ?
	gpipeline.VS.pShaderBytecode = m_PlaneVertexShader->GetBufferPointer();
	gpipeline.VS.BytecodeLength = m_PlaneVertexShader->GetBufferSize();
	gpipeline.PS.pShaderBytecode = m_PlanePixelShader->GetBufferPointer();
	gpipeline.PS.BytecodeLength = m_PlanePixelShader->GetBufferSize();

	// ?T???v???}?X?N?E???X?^???C?U?[?̐ݒ?
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // ?J?????O???Ȃ?
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ???g?̓h???Ԃ?
	gpipeline.RasterizerState.DepthClipEnable = true; // ?[?x?????̃N???b?s???O?͗L????
	//?c??
	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	// ?u?????h?X?e?[?g?̐ݒ?
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	// ???̓??C?A?E?g?̐ݒ?
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	gpipeline.pRootSignature = m_pRootSignature.Get();

	result = m_device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(m_peraPipeline.ReleaseAndGetAddressOf()));


	if (result == S_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DirectGraphics::CreateShadowPipelineState()
{
	// ?e?p?̃p?C?v???C???X?e?[?g??????


	// ?V?F?[?_?[????
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	auto result =
		D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "shadowVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::OutputDebugStringA("?t?@?C?????????????܂???");
		return 0;
	}
	else if (result != S_OK)
	{
		// ?G???[???b?Z?[?W?\??
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		// ???_????
		{
			"POSITION",									// ?Z?}???e?B?N?X
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,				// ?t?H?[?}?b?g
			0,											// ???̓X???b?g?C???f?b?N?X
			D3D12_APPEND_ALIGNED_ELEMENT,				// ?f?[?^?̕??т???
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// ?꒸?_???ɂ??̃??C?A?E?g???????Ă???
			0
		},
		// ?@??????
		{
			"NORMAL",									// ?Z?}???e?B?N?X
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,				// ?t?H?[?}?b?g
			0,											// ???̓X???b?g?C???f?b?N?X
			D3D12_APPEND_ALIGNED_ELEMENT,				// ?f?[?^?̕??т???
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// ?꒸?_???ɂ??̃??C?A?E?g???????Ă???
			0
		},
		// UV????
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,					// ??????XMFLOAT2?Ȃ̂œ??Ԃ?
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC plsDesc = {};

	plsDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob);
	plsDesc.PS.BytecodeLength = 0;
	plsDesc.PS.pShaderBytecode = nullptr;
	plsDesc.NumRenderTargets = 0;
	plsDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;

	// ?T???v???}?X?N?E???X?^???C?U?[?̐ݒ?
	plsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	plsDesc.RasterizerState.MultisampleEnable = false;
	plsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // ?J?????O???Ȃ?
	plsDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // ???g?̓h???Ԃ?
	plsDesc.RasterizerState.DepthClipEnable = true; // ?[?x?????̃N???b?s???O?͗L????
	//?c??
	plsDesc.RasterizerState.FrontCounterClockwise = false;
	plsDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	plsDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	plsDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	plsDesc.RasterizerState.AntialiasedLineEnable = false;
	plsDesc.RasterizerState.ForcedSampleCount = 0;
	plsDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	plsDesc.DepthStencilState.DepthEnable = true;
	plsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	plsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	plsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	// ?u?????h?X?e?[?g?̐ݒ?
	plsDesc.BlendState.AlphaToCoverageEnable = false;
	plsDesc.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	plsDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	// ???̓??C?A?E?g?̐ݒ?
	plsDesc.InputLayout.pInputElementDescs = inputLayout;
	plsDesc.InputLayout.NumElements = _countof(inputLayout);

	plsDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	plsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	plsDesc.NumRenderTargets = 1;
	plsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	plsDesc.SampleDesc.Count = 1;
	plsDesc.SampleDesc.Quality = 0;

	plsDesc.pRootSignature = m_pRootSignature.Get();

	result = m_device->CreateGraphicsPipelineState(&plsDesc, IID_PPV_ARGS(m_shadowPipeline.ReleaseAndGetAddressOf()));

	return true;
}

void DirectGraphics::setViewPort()
{
	// ?r???[?|?[?g?ݒ?
	m_viewport = {};

	m_viewport.Width = WINDOW_WIDTH;
	m_viewport.Height = WINDOW_HEIGHT;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MaxDepth = 1.f;
	m_viewport.MinDepth = 0.f;


	// ?V?U?[???`?ݒ?
	m_scissorrect = {};

	m_scissorrect.top = 0;
	m_scissorrect.left = 0;
	m_scissorrect.right = m_scissorrect.left + WINDOW_WIDTH;
	m_scissorrect.bottom = m_scissorrect.top + WINDOW_HEIGHT;
}

/*
bool DirectGraphics::CreateDepthSRVHeapAndView()
{
	// ?[?x?l?e?N?X?`???p?̃q?[?v??????
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto result = m_device->CreateDescriptorHeap(
		&heapDesc, IID_PPV_ARGS(&m_PlaneDepthSRVHeap)
	);

	if (result != S_OK)
	{
		return false;
	}

	// ?[?x?l?e?N?X?`???p?̃r???[??????
	D3D12_SHADER_RESOURCE_VIEW_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	resDesc.Texture2D.MipLevels = 1;
	resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	auto handle = m_PlaneDepthSRVHeap->GetCPUDescriptorHandleForHeapStart();

	m_device->CreateShaderResourceView(m_depthBuff.Get(), &resDesc, handle);

	return true;
}
*/