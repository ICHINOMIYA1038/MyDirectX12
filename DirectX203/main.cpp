#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
//オブジェクトファイルに、リンカでリンクするライブラリの名前を記述するもの。


#ifdef _DEBUG
#include<iostream>
#endif
#include <vector>
#include"Model.h"
using namespace std;
using namespace DirectX;

void DebugOutputFormatString(const char* format, ...)
{
	#ifdef _DEBUG
		va_list valist;
		va_start(valist, format);
		printf(format, valist);
		va_end(valist);
	#endif
}

//*****************************//
// global変数
//*****************************//
int window_width = 960;
int window_height = 540;
WNDCLASSEX w = {};
RECT wrc = { 0,0,window_width,window_height };
ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
IDXGISwapChain4* _swapchain = nullptr;
HWND hwnd;
ID3D12CommandQueue* _cmdQueue;
ID3D12DescriptorHeap* rtvHeaps = nullptr;
std::vector<ID3D12Resource*> _backBuffers;
ID3D12Fence* _fence = nullptr;
UINT64 _fenceVal = 0;
ID3D12PipelineState* _pipelinestate = nullptr;
ID3D12RootSignature* rootsignature = nullptr;
D3D12_RECT scissorrect = {};
D3D12_VIEWPORT viewport = {};
D3D12_VERTEX_BUFFER_VIEW vbView = {};
HRESULT result;


/****************************/
//　プロトタイプ宣言
/****************************/

void EnableDebugLayer();
int receiveMSG();
int CreateWindowClass();
int CreateWindowObject();
int receiveMSG();
int createDeveice();
int createCommand();
int CreateCommandQueue();
int CreateSwapChain();
int CreateDescHeap();
int CreateRenderTarget();
int Paint();
int readShader();

#ifdef _DEBUG

int main()
{
	EnableDebugLayer();
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
	DebugOutputFormatString("Show window test.");
	int a = CreateWindowClass();
	if (a != 0)return -1;
	a = CreateWindowObject();
	if (a != 0)return -1;
	a = createDeveice();
	if (a != 0)return -1;
	a = createCommand();
	if (a != 0)return -1;
	a = CreateCommandQueue();
	if (a != 0)return -1;
	a = CreateSwapChain();
	if (a != 0)return -1;
	a = CreateDescHeap();
	if (a != 0)return -1;
	a = CreateRenderTarget();
	if (a != 0)return -1;
	readShader();
	a = receiveMSG();
	if (a != 0)return -1;

}

void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	 result = D3D12GetDebugInterface(
		IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


int CreateWindowClass() 
{
	
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = "First App";
	w.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&w);
	
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	return 0;
}

int CreateWindowObject()
{
	hwnd = CreateWindow(w.lpszClassName,
		"app",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	ShowWindow(hwnd, SW_SHOW);
	return 0;
}

int receiveMSG() {
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}
		Paint();
	}
	return 0;
}

int createDeveice() {
	if (D3D12CreateDevice(
		nullptr,//複数のグラボの場合は指定する
		D3D_FEATURE_LEVEL_12_1,
		IID_PPV_ARGS(&_dev)
	) != S_OK)
		return -1;
	
#ifdef _DEBUG
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	return 0;

}

int createCommand() {
	 result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (result != S_OK) {
		return -1;
	}
	result = _dev->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,_cmdAllocator,nullptr,IID_PPV_ARGS(&_cmdList));
	if (result != S_OK) {
		return -1;
	}
	return 0;
}

int CreateCommandQueue() {
	_cmdQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_LIST_TYPE_DIRECT;
	 result = _dev->CreateCommandQueue(&cmdQueueDesc,IID_PPV_ARGS(&_cmdQueue));
	if (result != S_OK) {
		return -1;
	}
	return 0;
}

int CreateSwapChain() {

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	/// <summary>
	/// BUfferCountだけメモリを確保している
	/// </summary>
	/// <returns></returns>

	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);
	if (result != S_OK) {
		return -1;
	}
	return 0;
}


/// <summary>
/// ディスクリプタは、GPUリソースの用途や使用法について説明するデータである。
/// ヒープをGPU上に作成する。
/// </summary>
/// <returns></returns>
int CreateDescHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	/// D3D12_DESCRIPTOR_HEAP_TYPE_RTV レンダーターゲットビュー
	/// D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV　定数バッファ　テクスチャバッファ　コンピュートシェーダー
	/// D3D12_DESCRIPTOR_HEAP_TYPE_DSV　深度ステンシルビュー
	/// 
	heapDesc.NodeMask = 0;
	//複数のGPUがある場合に識別を行うためのビットフラグです。
	heapDesc.NumDescriptors = 2;
	//ディスクリプタの数を表すメンバーです。
	//ダブルバッファリングの場合、表画面・裏画面それぞれのバッファに対応するビューなので、2を指定する。
	//Flagsは、このビューに当たる情報をシェーダー側から見える必要があるかど指定する列挙子である。
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	 result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	///ビュー用のメモリを確保した。
	return 0;
}

///ディスクリプタとスワップチェーン上のバッファの関連付け
int CreateRenderTarget() {
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	/// <summary>
	/// swapchainの説明をswcDescに入れる
	/// </summary>
	/// <returns></returns>
	 result = _swapchain->GetDesc(&swcDesc);
	_backBuffers.resize(swcDesc.BufferCount);

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		///バッファの受け取り
		 result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		if (result != S_OK) {
			return -1;
		}
		//ディスクリプタヒープハンドルはヒープ上のアドレスのようなもの
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		//ディスクリプタの種類を指定することで、サイズを取得している。
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		//レンダーターゲットの作成
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
		
	}
	return 0;
}

XMFLOAT3 vertices[4];
///スワップチェーンの動作
int Paint() {
	///流れ
	///1.パイプラインステートのセット
	/// 2.ルートシグネチャのセット
	/// 3.ビューポートとシザー矩形のセット
	/// 4.プリミティブトポロジのセット
	/// 5.頂点情報のセット
	/// 6.描画命令
	
	
	/*
	vertices[0] = { -0.4f,-0.7f,0.0f };//左下
	vertices[1] = { -0.4f,0.7f,0.0f };//左上
	vertices[2] = { 0.4f,-0.7f,0.0f };//右下
	vertices[3] = { 0.4f,0.7f,0.0f };//右下

	unsigned short indices[] = {
		0,1,2,
		2,1,3
	};

	

	/// <summary>
	/// GPU上に生成するバッファ
	/// </summary>
	/// <returns></returns>
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
	
	*/

	XMFLOAT3 vertArray[4] = { { -0.4f,-0.7f,0.0f } ,{ -0.4f,0.7f,0.0f } ,{0.4f,-0.7f,0.0f} ,{0.4f,0.7f,0.0f } };
	Model model = Model(_dev, vertArray);
	XMFLOAT3 vertArray2[4] = {{ -1.0f,-0.5f,0.0f } ,{ -1.0f,-0.8f,0.0f } ,{-0.8f,-0.5f,0.0f} ,{-0.8f,-0.8f,0.0f }};
	Model model2 = Model(_dev, vertArray2);

	/*
	//******頂点バッファの作成*************
	ID3D12Resource* vertBuff = nullptr;
	
	//実際に、バッファをGPU上に生成する関数
	 result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));
	if (result != S_OK) {
		return -1;
	}

	///頂点の座標
	XMFLOAT3* vertMap = nullptr;
	//生成したバッファをCPU側のvertMapに結び付ける。
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr); 

	//バッファのGPU上でのアドレスを取得
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//全バイト数
	vbView.SizeInBytes = sizeof(vertices);
	//1頂点あたりのバイト数
	vbView.StrideInBytes = sizeof(vertices[0]);
	//頂点バッファビューをGPUに伝えてあげる処理
	

	//****************************************
	

	
	//******インデックスバッファの作成*************
	ID3D12Resource* idxBuff = nullptr;

	//実際に、バッファをGPU上に生成する関数
	result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff));
	if (result != S_OK) {
		return -1;
	}

	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);


	//*****************************************/

	///スワップチェーンが機能していたら、0か1のどちらかが帰ってくる。
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	//リソースバリアの設定 
	//レンダリングする前にレンダーターゲットとして使用するという設定をする。
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
	BarrierDesc.Transition.Subresource = 0;

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	_cmdList->ResourceBarrier(1, &BarrierDesc);



	//コマンドリストにこれから利用するレンダーターゲットビューをセットします。
	_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	//単一色に画面をクリア
	float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	
	_cmdList->SetPipelineState(_pipelinestate);
	_cmdList->SetGraphicsRootSignature(rootsignature);
	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	model2.draw(_cmdList);
	model.draw(_cmdList);
	
	//頂点バッファのセット
	//第一引数にはスロット番号
	//第二引数には頂点バッファービューの数を
	//第三引数には、頂点バッファビューの配列をセットします。

	//_cmdList->IASetVertexBuffers(0, 1, &vbView);

	//描画命令の設定
	//第一引数には、頂点数
	//第二引数にはインスタンス数
	//第三引数には頂点データのオフセット
	//第四引数には、インスタンスのオフセット
	//_cmdList->DrawInstanced(3, 1, 0, 0);
	//_cmdList->IASetVertexBuffers(0, 1, &vbView);
	//_cmdList->IASetIndexBuffer(&ibView);
	//_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();
	ID3D12CommandList* cmdlists[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1,  cmdlists );
	
	///フェンスオブジェクトの作成
	///フェンスの役割はコマンドリストの命令がすべて完了したことを知ることです。
	///今回はコマンドを実行した後に、シグナルメソッドを呼び出すことにする。
	 result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	
	_cmdQueue->Signal(_fence, ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		//フェンス値が第一引数に与えられた値になったときに、第二引数で指定したイベントを通知します。

		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}



	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator, nullptr);


	
	//第一引数はフリップまでの待ちフレーム数
	//VSYNCは、画面内の全てのピクセルを置き換えることを知らせる信号である。
	//この間にゲーム側の処理を終えないとバッファの切り替えに間に合わず、
	//処理落ちしてしまう。
	//第二引数はさまざまな指定
	_swapchain->Present(1, 0);
	return 0;
}

int readShader() {
	//BLOBとは、BinaryLargeObjectの略で、大きなデータの塊を指す言葉です。
	//ともかく不定形のデータに対して、この名称が
	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	result = D3DCompileFromFile(L"BasicVertexShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_vsBlob, &errorBlob);
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);//行儀悪いかな…
	}

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob, &errorBlob
	);

	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			::OutputDebugStringA("ファイルが見当たりません");
			return 0;
		}
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";

			::OutputDebugStringA(errstr.c_str());
		}
		exit(1);//行儀悪いかな…
	}

	//GPUの頂点データへの解釈
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	{
		"POSITION", //セマンティック
		0,//同じセマンティックが存在するときのインデックス
		DXGI_FORMAT_R32G32B32_FLOAT,//Format
		0,D3D12_APPEND_ALIGNED_ELEMENT,//入力スロットインデックス
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,//インスタンシング関連
		0//一度に描画するインスタンスの数
	},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr;
	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState.MultisampleEnable = false;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリング
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//中身を塗りつぶす
	gpipeline.RasterizerState.DepthClipEnable = true;//深度方向のクリッピングは有効に。

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	//gpipeline.InputLayout.NumElements = sizeof(inputLayout)/sizeof(inputLayout[0])


	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	//レンダーターゲットの設定;
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	
	//アンチエイリアシング,ジャギーを防ぐ。
	gpipeline.SampleDesc.Count = 1;//1ピクセルあたり、1のサンプリング
	gpipeline.SampleDesc.Quality = 0;
	
	///ルートシグネチャ
	///ディスクリプタテーブルをまとめたもの
	///ディスクリプタテーブルは、ディスクリプタヒープとシェーダーのレジスターを関連付けるテーブルです。
	/// どのスロットから何個のテクスチャのデータを利用するのかというのをGPUに教える役割。
	/// シグネチャとは、パラメーターの型と並びを規定したもの。
	/// 

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	
	ID3DBlob* rootSigBlob = nullptr;

	///バイナリコードの生成
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	//バイナリコードの生成後、ルートシグネチャオブジェクトを作成します。

	result = _dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootsignature));
	
	//開放
	rootSigBlob->Release();

	gpipeline.pRootSignature = rootsignature;

	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));


	///ビューポート
	///画面に対してレンダリング結果をどう表示するか。
	///ウィンドウのサイズと深度(デプス範囲)を指定する。
	/// これに加えてシザー矩形というものを設定する必要がある。

	///シザー矩形とは、ビューポートに出力された画像のどこからどこまでを実際に画面に映し出すか。
	viewport.Width = window_width;//出力先の幅(ピクセル数)
	viewport.Height = window_height;//出力先の高さ(ピクセル数)
	viewport.TopLeftX = 0;//出力先の左上座標X
	viewport.TopLeftY = 0;//出力先の左上座標Y
	viewport.MaxDepth = 1.0f;//深度最大値
	viewport.MinDepth = 0.0f;//深度最小値


	scissorrect.top = 0;//切り抜き上座標
	scissorrect.left = 0;//切り抜き左座標
	scissorrect.right = scissorrect.left + window_width;//切り抜き右座標
	scissorrect.bottom = scissorrect.top + window_height;//切り抜き下座標

	
}
