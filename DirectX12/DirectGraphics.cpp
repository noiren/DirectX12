#include "DirectGraphics.h"
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

D3D12_RESOURCE_BARRIER BarrierDesc = {};
using namespace DirectX;
namespace {
	const float WINDOW_WIDTH = 900.f;
	const float WINDOW_HEIGHT = 900.f;
}

std::vector<DirectGraphics::TexRGBA> textureData(256 * 256);

DirectGraphics::Vertex vertices[] =
{
	{{-0.4f,-0.7f,0.0f},{0.0f,1.0f}}, // 左下
	{{-0.4f, 0.7f,0.0f},{0.0f,0.0f}}, // 左上
	{{ 0.4f,-0.7f,0.0f},{1.0f,1.0f}}, // 左下
	{{ 0.4f, 0.7f,0.0f},{1.0f,0.0f}}, // 左下
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
	m_pTexDescHeap(nullptr),
	m_vbView(),
	m_ibView(),
	m_pPipelineState(nullptr),
	m_pRootSignature(nullptr),
	m_viewport(),
	m_scissorrect(),
	m_metadata()
{
	// テクスチャのデータ作る
	// これはノイズ画像になるはずです。
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

	if (CreateTextureBuffer() == false)
	{
		return false;
	}

	if (CreateShaderResourceView() == false)
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

	setViewPort();
	return true;
}




void DirectGraphics::Release()
{
	delete(m_rtvHeaps);	
	delete(m_cmdQueue);
	delete(m_cmdList);
	delete(m_cmdAllocator);
	delete(m_swapChain);
	delete(m_dxgiFactory);
	delete(m_device);
}

bool DirectGraphics::CreateDevice()
{
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));

	std::vector<IDXGIAdapter*> adapters;

	IDXGIAdapter* tmpAdapter = nullptr;

	// 利用可能なアダプターの列挙を行う
	for (int i = 0; m_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}

	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		// 探したいアダプターの名前を確認
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
		if (D3D12CreateDevice(tmpAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)) == S_OK)
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
		if (m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator)) == S_OK &&
			m_device->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,m_cmdAllocator,nullptr,IID_PPV_ARGS(&m_cmdList)) == S_OK)
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

	if (m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_cmdQueue)) == S_OK)
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
	// スワップチェーン関連の設定を行う
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
		m_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&m_swapChain
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

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // ビューのタイプを設定 今回はレンダーターゲットビュー
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;// ディスクリプタは表・裏の二つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeaps)) == S_OK)
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

	m_backBuffers.clear();// 一応初期化

	m_backBuffers.reserve(swcDesc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	// SRGB用のレンダーターゲットビューの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // ガンマ補正アリ
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		m_backBuffers.emplace_back(nullptr);
		m_swapChain->GetBuffer(static_cast<UINT>(idx),IID_PPV_ARGS(&m_backBuffers[idx]));

		m_device->CreateRenderTargetView(m_backBuffers[idx], &rtvDesc, handle);

		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);// ここの分だけずらして取得
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

bool DirectGraphics::SetupSwapChain()
{
	auto bbIdx = m_swapChain->GetCurrentBackBufferIndex(); // 0か1が帰ってくる

	if (bbIdx > 2)
	{
		return false;
	}

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION; // 遷移
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	   // 特に指定なし
	BarrierDesc.Transition.pResource = m_backBuffers[bbIdx];   // バックバッファーリソース
	BarrierDesc.Transition.Subresource = 0;

	BarrierDesc.Transition.StateBefore
		= D3D12_RESOURCE_STATE_PRESENT;			// 直前はPRESENT状態

	BarrierDesc.Transition.StateAfter
		= D3D12_RESOURCE_STATE_RENDER_TARGET;	// 今からレンダーターゲット状態

	m_cmdList->ResourceBarrier(1, &BarrierDesc);

	m_cmdList->SetPipelineState(m_pPipelineState); // パイプラインステートの設定

	auto rtvH = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	rtvH.ptr += bbIdx * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	float clearColor[] = { 1.0f,1.0f,0.0f,1.0f }; // 黄色

	m_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr); // 画面クリア

	m_cmdList->RSSetViewports(1, &m_viewport);

	m_cmdList->RSSetScissorRects(1, &m_scissorrect);

	m_cmdList->SetGraphicsRootSignature(m_pRootSignature); // ルートシグネチャの設定

	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// プリミティブトポロジの設定

	m_cmdList->IASetVertexBuffers(0, 1, &m_vbView);// 頂点バッファビューの設定

	m_cmdList->IASetIndexBuffer(&m_ibView);// インデックスバッファビューの設定(1回に設定できるインデックスバッファーは1つだけ)

	m_cmdList->SetGraphicsRootSignature(m_pRootSignature); 

	m_cmdList->SetDescriptorHeaps(1, &m_pTexDescHeap); // ディスクリプタヒープの設定

	m_cmdList->SetGraphicsRootDescriptorTable(
		0,
		m_pTexDescHeap->GetGPUDescriptorHandleForHeapStart()
	);

	m_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// コマンドリストの実行
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_cmdList->ResourceBarrier(1, &BarrierDesc);

	m_cmdList->Close(); // 実行前には必ずクローズすること！

	ID3D12CommandList* cmdLists[] = { m_cmdList };
	m_cmdQueue->ExecuteCommandLists(1, cmdLists);

	m_cmdQueue->Signal(m_fence, ++m_fenceVal);

	if (m_fence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);

		m_fence->SetEventOnCompletion(m_fenceVal, event);

		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}

	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator, nullptr);

	return true;
}

void DirectGraphics::Render()
{
	if (SetupSwapChain() == false)
	{
		return;
	}
	m_swapChain->Present(1, 0);
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

		auto result = m_device->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

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
	// 頂点の大きさ分の空きをメモリに作る

	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;


	D3D12_RESOURCE_DESC resdesc = {};

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// バッファの確保
	if (FAILED(m_device->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertBuff))))
	{
		return false;
	}

	DirectGraphics::Vertex* vertMap = nullptr;

	if (FAILED(m_vertBuff->Map(0, nullptr, (void**)&vertMap)))// 確保したバッファの仮想アドレスを取得する
	{
		return false;
	}

	std::copy(std::begin(vertices), std::end(vertices), vertMap);// 実際にそのアドレスを編集すればOK!(今回はそのアドレスに頂点情報を流し込んでいる)

	m_vertBuff->Unmap(0, nullptr);

	// バッファを作成したらそこに合わせたビューの作成(今回はvertexBufferview)
	m_vbView = {};

	m_vbView.BufferLocation = m_vertBuff->GetGPUVirtualAddress(); // バッファの仮想アドレス
	m_vbView.SizeInBytes = sizeof(vertices);						// 全バイト数
	m_vbView.StrideInBytes = sizeof(vertices[0]);					// 一頂点のバイト数


	// ここからインデックスビュー
	unsigned short indices[] = {
		0,1,2,
		2,1,3
	};

	resdesc.Width = sizeof(indices);
	if (FAILED(m_device->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexBuff))))
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

bool DirectGraphics::CreateTextureBuffer()
{
	// テクスチャのロード

	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	ScratchImage scratchImg = {};

	result = LoadFromWICFile(
		L"kato.png",
		WIC_FLAGS_NONE,
		&m_metadata,
		scratchImg
	);

	if (result != S_OK)
	{
		return false;
	}

	auto img = scratchImg.GetImage(0, 0, 0);

	// ヒープの設定
	D3D12_HEAP_PROPERTIES heapprop = {};

	heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;

	//　ライトバック
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;

	//	転送はL0、つまりCPU側から直接
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	heapprop.CreationNodeMask = 0;
	heapprop.VisibleNodeMask = 0;

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

	result = m_device->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_texBuff));

	if (result != S_OK)
	{
		return false;
	}

	// テクスチャバッファの転送
	result = m_texBuff->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);

	if (result != S_OK)
	{
		return false;
	}

	return true;
}

bool DirectGraphics::CreateShaderResourceView()
{
	// テクスチャや定数バッファに関するビューはディスクリプタヒープ上に作る

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};

	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;// シェーダーリソースビュー用

	auto result = m_device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&m_pTexDescHeap));

	if (result != S_OK)
	{
		return false;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = m_metadata.format; // RGBA(0.0f~1.0fに正規化している)
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	m_device->CreateShaderResourceView(
		m_texBuff,	// ビューと関連づけるバッファ
		&srvDesc,	// テクスチャ設定情報
		m_pTexDescHeap->GetCPUDescriptorHandleForHeapStart() // ヒープのどこに割り当てるか
	);

	return true;
}

bool DirectGraphics::CreateRootSignature()
{
	// ディスクリプタレンジの作成
	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // 種別はテクスチャ
	descTblRange.BaseShaderRegister = 0; // 0番スロットからスタート
	descTblRange.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメーター作成
	D3D12_ROOT_PARAMETER rootparam = {};

	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // ディスクリプタテーブルとして使用予定
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダーから見えるもの
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 1;

	// サンプラー設定の作成
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

	// ルートシグネチャここで作成
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr; // ルードシグネチャのバイナリコードを作成する
	ID3DBlob* errorBlob = nullptr;

	auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	if (result != S_OK)
	{
		// エラーメッセージ表示
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;
	}

	result = m_device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));

	rootSigBlob->Release();

	return true;
}

bool DirectGraphics::CreateShader()
{
	ID3DBlob* errorBlob = nullptr;

	// vertexShaderのコンパイル
	auto result = 
		D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_pVsShader, &errorBlob);
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::OutputDebugStringA("ファイルが見当たりません");
		return 0;
	}
	else if(result != S_OK)
	{
		// エラーメッセージ表示
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),errorBlob->GetBufferSize(),errstr.begin());
		errstr += "\n";
		::OutputDebugStringA(errstr.c_str());
		return false;
	}
	
	// pixelShaderのコンパイル
	result = 
		D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_pPsShader, &errorBlob);
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		::OutputDebugStringA("ファイルが見当たりません");
		return 0;
	}
	else if(result != S_OK)
	{
		// エラーメッセージ表示
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

	// ここでの情報がシェーダーに渡されるぞ！

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		// 頂点情報
		{
			"POSITION",									// セマンティクス
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,				// フォーマット
			0,											// 入力スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,				// データの並びかた
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// 一頂点毎にこのレイアウトが入っている
			0
		},
		// UV情報
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,					// 情報はXMFLOAT2なので二つぶん
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	// ここでグラフィックスパイプラインステートも作る

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	// 頂点シェーダーとピクセルシェーダーを設定
	gpipeline.VS.pShaderBytecode = m_pVsShader->GetBufferPointer();
	gpipeline.VS.BytecodeLength = m_pVsShader->GetBufferSize();
	gpipeline.PS.pShaderBytecode = m_pPsShader->GetBufferPointer();
	gpipeline.PS.BytecodeLength = m_pPsShader->GetBufferSize();

	// サンプルマスク・ラスタライザーの設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	
	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // カリングしない
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // 中身の塗りつぶし
	gpipeline.RasterizerState.DepthClipEnable = true; // 深度方向のクリッピングは有効に
	//残り
	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


	// ブレンドステートの設定
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	// 入力レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	gpipeline.SampleDesc.Count = 1;
	gpipeline.SampleDesc.Quality = 0;

	gpipeline.pRootSignature = m_pRootSignature;

	auto result = m_device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&m_pPipelineState));


	if (result == S_OK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DirectGraphics::setViewPort()
{
	// ビューポート設定
	m_viewport = {};

	m_viewport.Width = WINDOW_WIDTH;
	m_viewport.Height = WINDOW_HEIGHT;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MaxDepth = 1.f;
	m_viewport.MinDepth = 0.f;


	// シザー矩形設定
	m_scissorrect = {};

	m_scissorrect.top = 0;
	m_scissorrect.left = 0;
	m_scissorrect.right = m_scissorrect.left + WINDOW_WIDTH;
	m_scissorrect.bottom = m_scissorrect.top + WINDOW_HEIGHT;
}