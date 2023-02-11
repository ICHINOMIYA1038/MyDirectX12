# include "Model.h"

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

void Model::setVertices() {
	//vertices[0] = { -0.4f,-0.7f,0.0f };//����
	//vertices[1] = { -0.4f,0.7f,0.0f };//����
	//vertices[2] = { 0.4f,-0.7f,0.0f };//�E��
	//vertices[3] = { 0.4f,0.7f,0.0f };//�E��

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


void Model::sendToGPU(ID3D12GraphicsCommandList* _cmdList) {
	//**���_�o�b�t�@�̑��M**//

	ID3D12Resource* vertBuff = nullptr;
	auto result = _dev->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));
	if (result != S_OK)
	{
		exit(-1);
	}
	XMFLOAT3* vertMap = nullptr;
	//���������o�b�t�@��CPU����vertMap�Ɍ��ѕt����B
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (result != S_OK)
	{
		exit(-1);
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	//�o�b�t�@��GPU��ł̃A�h���X���擾
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//�S�o�C�g��
	vbView.SizeInBytes = sizeof(vertices);
	//1���_������̃o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);
	//���_�o�b�t�@�r���[��GPU�ɓ`���Ă����鏈��
	

	//**�C���f�b�N�X�o�b�t�@�̑��M**//
	ID3D12Resource* idxBuff = nullptr;

	//���ۂɁA�o�b�t�@��GPU��ɐ�������֐�
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




