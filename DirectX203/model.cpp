#include"Model.h"
#include"util.h"

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

Model::Model(ID3D12Device* device, XMFLOAT3* vertArray) {
	_dev = device;
	vertices[0] = vertArray[0];
	vertices[1] = vertArray[1];
	vertices[2] = vertArray[2];
	vertices[3] = vertArray[3];
}

Model::Model(ID3D12Device* device) {
	_dev = device;
}

void Model::setRootSignature(ID3D12RootSignature* _rootSignature) {
	rootsignature = _rootSignature;
}

void Model::setVertices() {
	//vertices[0] = { -0.4f,-0.7f,0.0f };//左下
	//vertices[1] = { -0.4f,0.7f,0.0f };//左上
	//vertices[2] = { 0.4f,-0.7f,0.0f };//右下
	//vertices[3] = { 0.4f,0.7f,0.0f };//右下
}

void Model::setIndices() {
	unsigned short array[6] = {
		0,1,2,2,1,3
	};
	std::copy(std::begin(array), std::end(array), indices);
}

void Model::draw(ID3D12GraphicsCommandList* _cmdList) {

	setVertices();
	setIndices();
	initResDesc();
	initHeapProp();
	sendTexture(_cmdList);
	sendToGPU(_cmdList);
}


void Model::initResDesc() {
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
}



void Model::initHeapProp() {
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
}

void Model::CreateTextureHeap() {
	heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
}

void Model::CreateTextureResDesc() {
	resdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resdesc.Width = 256;
	resdesc.Height = 256;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.SampleDesc.Quality = 0;
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
}

void Model::sendTexture(ID3D12GraphicsCommandList* _cmdList)
{

	struct TexRGBA
	{
		unsigned char R, G, B, A;
	};
	std::vector<TexRGBA>texturedata(256 * 256);

	for (auto& rgba : texturedata)
	{
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;
	}

	ID3D12Resource* texbuff = nullptr;
	_dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&textureResdesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);

	texbuff->WriteToSubresource(
		0,
		nullptr,
		texturedata.data(),
		sizeof(TexRGBA) * 256,
		sizeof(TexRGBA)* texturedata.size()
	);

	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	_dev->CreateShaderResourceView(texbuff, &srvDesc, texDescHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = 1;

	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 1;

	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;


	_cmdList->SetGraphicsRootSignature(rootsignature);
	_cmdList->SetDescriptorHeaps(1, &texDescHeap);
	_cmdList->SetGraphicsRootDescriptorTable(
		0,//ルートパラメーターインデックス
		texDescHeap->GetGPUDescriptorHandleForHeapStart());

	
}

void Model::sendToGPU(ID3D12GraphicsCommandList* _cmdList) {
	//**頂点バッファの送信**//

	ID3D12Resource* vertBuff = nullptr;
	auto result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));
	if (result != S_OK)
	{
		exit(-1);
	}
	XMFLOAT3* vertMap = nullptr;
	//生成したバッファをCPU側のvertMapに結び付ける。
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (result != S_OK)
	{
		exit(-1);
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	//バッファのGPU上でのアドレスを取得
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//全バイト数
	vbView.SizeInBytes = sizeof(vertices);
	//1頂点あたりのバイト数
	vbView.StrideInBytes = sizeof(vertices[0]);
	//頂点バッファビューをGPUに伝えてあげる処理
	

	//**インデックスバッファの送信**//
	ID3D12Resource* idxBuff = nullptr;

	//実際に、バッファをGPU上に生成する関数
	result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&idxBuff));
	if (result != S_OK)
	{
		exit(-1);
	}

	unsigned short* mappedIdx = nullptr;
	idxBuff->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indices), std::end(indices), mappedIdx);
	idxBuff->Unmap(0, nullptr);

	
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	_cmdList->IASetIndexBuffer(&ibView);
	_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Model::setVertex(Vertex* vertData)
{
	memcpy(&vertData, &vertex, sizeof(vertData));
}




