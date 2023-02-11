#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <vector> 

using namespace DirectX;
using namespace std;

class Model {
	XMFLOAT3 vertices[4];
	unsigned short indices[6];
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	D3D12_RESOURCE_DESC resdesc;
	D3D12_HEAP_PROPERTIES heapprop = {};
	ID3D12Device* _dev = nullptr;

public:
	Model(ID3D12Device* device);
	Model(ID3D12Device* device,XMFLOAT3* vertArray);
	void setVertices();
	void setIndices();
	void draw(ID3D12GraphicsCommandList* _cmdList);
	void initResDesc();
	void sendToGPU(ID3D12GraphicsCommandList* _cmdList);

	void initHeapProp();
};