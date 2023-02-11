#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>  

using namespace DirectX;
using namespace std;

class Model {
	vector<XMFLOAT3>vertices = {};
	unsigned short vertNum = 3;
	vector<unsigned short>indices = {};
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_RESOURCE_DESC resdesc;
	
public:
	Model();
	void draw();
	void initResDesc();
	void sendToGPU();
	void initVbView();
};

Model::Model() {
	indices = vector<unsigned short>(vertNum);
	
}

void Model::draw() {

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

/// <summary>
/// 頂点バッファのビューを作成
/// </summary>
void Model::initVbView() {

}

void Model::sendToGPU() {
	ID3D12Resource* vertBuff = nullptr;
	XMFLOAT3* vertMap = nullptr;
	//生成したバッファをCPU側のvertMapに結び付ける。
	auto result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	//バッファのGPU上でのアドレスを取得
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//全バイト数
	vbView.SizeInBytes = sizeof(vertices);
	//1頂点あたりのバイト数
	vbView.StrideInBytes = sizeof(vertices[0]);
	//頂点バッファビューをGPUに伝えてあげる処理
	//_cmdList->IASetVertexBuffers(0, 1, &vbView);
}



