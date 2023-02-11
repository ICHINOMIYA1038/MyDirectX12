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
/// ���_�o�b�t�@�̃r���[���쐬
/// </summary>
void Model::initVbView() {

}

void Model::sendToGPU() {
	ID3D12Resource* vertBuff = nullptr;
	XMFLOAT3* vertMap = nullptr;
	//���������o�b�t�@��CPU����vertMap�Ɍ��ѕt����B
	auto result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	//�o�b�t�@��GPU��ł̃A�h���X���擾
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//�S�o�C�g��
	vbView.SizeInBytes = sizeof(vertices);
	//1���_������̃o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);
	//���_�o�b�t�@�r���[��GPU�ɓ`���Ă����鏈��
	//_cmdList->IASetVertexBuffers(0, 1, &vbView);
}



