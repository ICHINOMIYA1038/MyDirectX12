#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
//�I�u�W�F�N�g�t�@�C���ɁA�����J�Ń����N���郉�C�u�����̖��O���L�q������́B


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
// global�ϐ�
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
//�@�v���g�^�C�v�錾
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
		nullptr,//�����̃O���{�̏ꍇ�͎w�肷��
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
	/// BUfferCount�������������m�ۂ��Ă���
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
/// �f�B�X�N���v�^�́AGPU���\�[�X�̗p�r��g�p�@�ɂ��Đ�������f�[�^�ł���B
/// �q�[�v��GPU��ɍ쐬����B
/// </summary>
/// <returns></returns>
int CreateDescHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	/// D3D12_DESCRIPTOR_HEAP_TYPE_RTV �����_�[�^�[�Q�b�g�r���[
	/// D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV�@�萔�o�b�t�@�@�e�N�X�`���o�b�t�@�@�R���s���[�g�V�F�[�_�[
	/// D3D12_DESCRIPTOR_HEAP_TYPE_DSV�@�[�x�X�e���V���r���[
	/// 
	heapDesc.NodeMask = 0;
	//������GPU������ꍇ�Ɏ��ʂ��s�����߂̃r�b�g�t���O�ł��B
	heapDesc.NumDescriptors = 2;
	//�f�B�X�N���v�^�̐���\�������o�[�ł��B
	//�_�u���o�b�t�@�����O�̏ꍇ�A�\��ʁE����ʂ��ꂼ��̃o�b�t�@�ɑΉ�����r���[�Ȃ̂ŁA2���w�肷��B
	//Flags�́A���̃r���[�ɓ���������V�F�[�_�[�����猩����K�v�����邩�ǎw�肷��񋓎q�ł���B
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	 result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	///�r���[�p�̃��������m�ۂ����B
	return 0;
}

///�f�B�X�N���v�^�ƃX���b�v�`�F�[����̃o�b�t�@�̊֘A�t��
int CreateRenderTarget() {
	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	/// <summary>
	/// swapchain�̐�����swcDesc�ɓ����
	/// </summary>
	/// <returns></returns>
	 result = _swapchain->GetDesc(&swcDesc);
	_backBuffers.resize(swcDesc.BufferCount);

	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		///�o�b�t�@�̎󂯎��
		 result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		if (result != S_OK) {
			return -1;
		}
		//�f�B�X�N���v�^�q�[�v�n���h���̓q�[�v��̃A�h���X�̂悤�Ȃ���
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		//�f�B�X�N���v�^�̎�ނ��w�肷�邱�ƂŁA�T�C�Y���擾���Ă���B
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		//�����_�[�^�[�Q�b�g�̍쐬
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
		
	}
	return 0;
}

XMFLOAT3 vertices[4];
///�X���b�v�`�F�[���̓���
int Paint() {
	///����
	///1.�p�C�v���C���X�e�[�g�̃Z�b�g
	/// 2.���[�g�V�O�l�`���̃Z�b�g
	/// 3.�r���[�|�[�g�ƃV�U�[��`�̃Z�b�g
	/// 4.�v���~�e�B�u�g�|���W�̃Z�b�g
	/// 5.���_���̃Z�b�g
	/// 6.�`�施��
	
	
	/*
	vertices[0] = { -0.4f,-0.7f,0.0f };//����
	vertices[1] = { -0.4f,0.7f,0.0f };//����
	vertices[2] = { 0.4f,-0.7f,0.0f };//�E��
	vertices[3] = { 0.4f,0.7f,0.0f };//�E��

	unsigned short indices[] = {
		0,1,2,
		2,1,3
	};

	

	/// <summary>
	/// GPU��ɐ�������o�b�t�@
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
	//******���_�o�b�t�@�̍쐬*************
	ID3D12Resource* vertBuff = nullptr;
	
	//���ۂɁA�o�b�t�@��GPU��ɐ�������֐�
	 result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));
	if (result != S_OK) {
		return -1;
	}

	///���_�̍��W
	XMFLOAT3* vertMap = nullptr;
	//���������o�b�t�@��CPU����vertMap�Ɍ��ѕt����B
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr); 

	//�o�b�t�@��GPU��ł̃A�h���X���擾
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//�S�o�C�g��
	vbView.SizeInBytes = sizeof(vertices);
	//1���_������̃o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);
	//���_�o�b�t�@�r���[��GPU�ɓ`���Ă����鏈��
	

	//****************************************
	

	
	//******�C���f�b�N�X�o�b�t�@�̍쐬*************
	ID3D12Resource* idxBuff = nullptr;

	//���ۂɁA�o�b�t�@��GPU��ɐ�������֐�
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

	///�X���b�v�`�F�[�����@�\���Ă�����A0��1�̂ǂ��炩���A���Ă���B
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	//���\�[�X�o���A�̐ݒ� 
	//�����_�����O����O�Ƀ����_�[�^�[�Q�b�g�Ƃ��Ďg�p����Ƃ����ݒ������B
	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
	BarrierDesc.Transition.Subresource = 0;

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	_cmdList->ResourceBarrier(1, &BarrierDesc);



	//�R�}���h���X�g�ɂ��ꂩ�痘�p���郌���_�[�^�[�Q�b�g�r���[���Z�b�g���܂��B
	_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	//�P��F�ɉ�ʂ��N���A
	float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	
	_cmdList->SetPipelineState(_pipelinestate);
	_cmdList->SetGraphicsRootSignature(rootsignature);
	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	model2.draw(_cmdList);
	model.draw(_cmdList);
	
	//���_�o�b�t�@�̃Z�b�g
	//�������ɂ̓X���b�g�ԍ�
	//�������ɂ͒��_�o�b�t�@�[�r���[�̐���
	//��O�����ɂ́A���_�o�b�t�@�r���[�̔z����Z�b�g���܂��B

	//_cmdList->IASetVertexBuffers(0, 1, &vbView);

	//�`�施�߂̐ݒ�
	//�������ɂ́A���_��
	//�������ɂ̓C���X�^���X��
	//��O�����ɂ͒��_�f�[�^�̃I�t�Z�b�g
	//��l�����ɂ́A�C���X�^���X�̃I�t�Z�b�g
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
	
	///�t�F���X�I�u�W�F�N�g�̍쐬
	///�t�F���X�̖����̓R�}���h���X�g�̖��߂����ׂĊ����������Ƃ�m�邱�Ƃł��B
	///����̓R�}���h�����s������ɁA�V�O�i�����\�b�h���Ăяo�����Ƃɂ���B
	 result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	
	_cmdQueue->Signal(_fence, ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		//�t�F���X�l���������ɗ^����ꂽ�l�ɂȂ����Ƃ��ɁA�������Ŏw�肵���C�x���g��ʒm���܂��B

		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}



	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator, nullptr);


	
	//�������̓t���b�v�܂ł̑҂��t���[����
	//VSYNC�́A��ʓ��̑S�Ẵs�N�Z����u�������邱�Ƃ�m�点��M���ł���B
	//���̊ԂɃQ�[�����̏������I���Ȃ��ƃo�b�t�@�̐؂�ւ��ɊԂɍ��킸�A
	//�����������Ă��܂��B
	//�������͂��܂��܂Ȏw��
	_swapchain->Present(1, 0);
	return 0;
}

int readShader() {
	//BLOB�Ƃ́ABinaryLargeObject�̗��ŁA�傫�ȃf�[�^�̉���w�����t�ł��B
	//�Ƃ������s��`�̃f�[�^�ɑ΂��āA���̖��̂�
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
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);//�s�V�������ȁc
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
			::OutputDebugStringA("�t�@�C������������܂���");
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
		exit(1);//�s�V�������ȁc
	}

	//GPU�̒��_�f�[�^�ւ̉���
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	{
		"POSITION", //�Z�}���e�B�b�N
		0,//�����Z�}���e�B�b�N�����݂���Ƃ��̃C���f�b�N�X
		DXGI_FORMAT_R32G32B32_FLOAT,//Format
		0,D3D12_APPEND_ALIGNED_ELEMENT,//���̓X���b�g�C���f�b�N�X
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,//�C���X�^���V���O�֘A
		0//��x�ɕ`�悷��C���X�^���X�̐�
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
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L���ɁB

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
	
	//�����_�[�^�[�Q�b�g�̐ݒ�;
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	
	//�A���`�G�C���A�V���O,�W���M�[��h���B
	gpipeline.SampleDesc.Count = 1;//1�s�N�Z��������A1�̃T���v�����O
	gpipeline.SampleDesc.Quality = 0;
	
	///���[�g�V�O�l�`��
	///�f�B�X�N���v�^�e�[�u�����܂Ƃ߂�����
	///�f�B�X�N���v�^�e�[�u���́A�f�B�X�N���v�^�q�[�v�ƃV�F�[�_�[�̃��W�X�^�[���֘A�t����e�[�u���ł��B
	/// �ǂ̃X���b�g���牽�̃e�N�X�`���̃f�[�^�𗘗p����̂��Ƃ����̂�GPU�ɋ���������B
	/// �V�O�l�`���Ƃ́A�p�����[�^�[�̌^�ƕ��т��K�肵�����́B
	/// 

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	
	ID3DBlob* rootSigBlob = nullptr;

	///�o�C�i���R�[�h�̐���
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	//�o�C�i���R�[�h�̐�����A���[�g�V�O�l�`���I�u�W�F�N�g���쐬���܂��B

	result = _dev->CreateRootSignature(
		0,
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootsignature));
	
	//�J��
	rootSigBlob->Release();

	gpipeline.pRootSignature = rootsignature;

	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));


	///�r���[�|�[�g
	///��ʂɑ΂��ă����_�����O���ʂ��ǂ��\�����邩�B
	///�E�B���h�E�̃T�C�Y�Ɛ[�x(�f�v�X�͈�)���w�肷��B
	/// ����ɉ����ăV�U�[��`�Ƃ������̂�ݒ肷��K�v������B

	///�V�U�[��`�Ƃ́A�r���[�|�[�g�ɏo�͂��ꂽ�摜�̂ǂ�����ǂ��܂ł����ۂɉ�ʂɉf���o�����B
	viewport.Width = window_width;//�o�͐�̕�(�s�N�Z����)
	viewport.Height = window_height;//�o�͐�̍���(�s�N�Z����)
	viewport.TopLeftX = 0;//�o�͐�̍�����WX
	viewport.TopLeftY = 0;//�o�͐�̍�����WY
	viewport.MaxDepth = 1.0f;//�[�x�ő�l
	viewport.MinDepth = 0.0f;//�[�x�ŏ��l


	scissorrect.top = 0;//�؂蔲������W
	scissorrect.left = 0;//�؂蔲�������W
	scissorrect.right = scissorrect.left + window_width;//�؂蔲���E���W
	scissorrect.bottom = scissorrect.top + window_height;//�؂蔲�������W

	
}
