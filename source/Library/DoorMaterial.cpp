#include "Polygon.h"
#include "DoorShader.h"
#include "DoorMaterial.h"
#include "WeldBuffer.h"

Library::BSPEngine::DoorMaterial::DoorMaterial()
{
	mShader = nullptr;
}

Library::BSPEngine::DoorMaterial::~DoorMaterial()
{
	DeleteObject(mShader);
}

void Library::BSPEngine::DoorMaterial::Initialize(ID3D11Device * device, HWND hwnd)
{
	mShader = new DoorShader;
	mShader->Initialize(device, hwnd);
}

void Library::BSPEngine::DoorMaterial::CreateVertexBuffer(ID3D11Device * device, const byte * pMem, size_t numBytes, ID3D11Buffer ** vertexBuffer) const
{
	D3D11_SUBRESOURCE_DATA vertexSubResourceData;
	ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
	vertexSubResourceData.pSysMem = pMem;
	*vertexBuffer = CreateD3DVertexBuffer(device, numBytes, false, false, &vertexSubResourceData);
}

void Library::BSPEngine::DoorMaterial::WriteVertex(byte * pMem, size_t offset, const Vertex & vertex, uint16_t texArrayID)
{
	DoorMaterialVertex *pDstVertex = reinterpret_cast<DoorMaterialVertex*>(pMem) + offset;
	pDstVertex->Position = { vertex.x, vertex.y, vertex.z };
	pDstVertex->TextureCoordinates = { vertex.tu, vertex.tv, (float)texArrayID };
	//pDstVertex->LightMapCoordinates = { vertex.lu, vertex.lv };
	//pDstVertex->Normal = vertex.Normal;
	//pDstVertex->Tangent = vertex.Tangent;
	//pDstVertex->LeafIndex = vertex.leafIndex;
}


size_t Library::BSPEngine::DoorMaterial::VertexSize() const
{
	return sizeof(DoorMaterialVertex);
}

void Library::BSPEngine::DoorMaterial::SetCBufferVS(ID3D11DeviceContext * deviceContext, XMMATRIX wvp)
{
	mShader->SetCBufferVS(deviceContext, wvp);
}

void Library::BSPEngine::DoorMaterial::SetCBufferPS(ID3D11DeviceContext * deviceContext, XMFLOAT3 worldCamPos, XMFLOAT2 clusterMapSize)
{
	mShader->SetCBufferPS(deviceContext, worldCamPos, clusterMapSize);
}

void Library::BSPEngine::DoorMaterial::SetDiffuseAndNormalTextureArrayViews(ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * textureArrDiffuseMapsView, ID3D11ShaderResourceView* textureArrNormalMapsView)
{
	mShader->SetDiffuseAndNormalTextureArrayViews(deviceContext, textureArrDiffuseMapsView, textureArrNormalMapsView);
}

void Library::BSPEngine::DoorMaterial::SetLightmapClusterMapsTextureArrayView(ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * lightMapTexArrayView, ID3D11ShaderResourceView * clusterMapTexArrayView)
{
	mShader->SetLightmapClusterMapsTextureArrayView(deviceContext, lightMapTexArrayView, clusterMapTexArrayView);
}

void Library::BSPEngine::DoorMaterial::Apply(ID3D11DeviceContext * deviceContext)
{
	mShader->Apply(deviceContext);
}

bool Library::BSPEngine::DoorMaterialVertex::same(const DoorMaterialVertex & v) const
{
	float f = 0.0f, d;

	d = this->Position.x - v.Position.x;
	f += d*d;
	d = this->Position.y - v.Position.y;
	f += d*d;
	d = this->Position.z - v.Position.z;
	f += d*d;
	d = this->TextureCoordinates.x - v.TextureCoordinates.x;
	f += d*d;
	d = this->TextureCoordinates.y - v.TextureCoordinates.y;
	f += d*d;
	d = this->TextureCoordinates.z - v.TextureCoordinates.z;
	f += d*d;
	/*
	d = this->Normal.x - v.Normal.x;
	f += d*d;
	d = this->Normal.y - v.Normal.y;
	f += d*d;
	d = this->Normal.z - v.Normal.z;
	f += d*d;
	d = this->Tangent.x - v.Tangent.x;
	f += d*d;
	d = this->Tangent.y - v.Tangent.y;
	f += d*d;
	d = this->Tangent.z - v.Tangent.z;
	f += d*d;
	d = this->LightMapCoordinates.x - v.LightMapCoordinates.x;
	f += d*d;
	d = this->LightMapCoordinates.y - v.LightMapCoordinates.y;
	f += d*d;
	*/
	return f < 0.01f;
}


size_t Library::BSPEngine::DoorMaterial::WeldBuffers(byte * pMem, size_t numVertices, ULONG * pIndices, size_t numIndices)
{
	return WeldBuffersT<DoorMaterialVertex>(pMem, numVertices, pIndices, numIndices);
}

void Library::BSPEngine::DoorMaterial::SetUseLighting(bool useLighting)
{
	mShader->SetUseLighting(useLighting);
}
