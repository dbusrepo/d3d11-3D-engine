#include "Polygon.h"
#include "BasicShader.h"
#include "BasicMaterial.h"
#include "WeldBuffer.h"
#include "CommonShader.h"

Library::BSPEngine::BasicMaterial::BasicMaterial()
{
	mShader = nullptr;
}

Library::BSPEngine::BasicMaterial::~BasicMaterial()
{
	DeleteObject(mShader);
}

void Library::BSPEngine::BasicMaterial::Initialize(ID3D11Device * device, HWND hwnd)
{
	mShader = new BasicShader;
	mShader->Initialize(device, hwnd);
}

void Library::BSPEngine::BasicMaterial::CreateVertexBuffer(ID3D11Device * device, const byte * pMem, size_t numBytes, ID3D11Buffer ** vertexBuffer) const
{
	D3D11_SUBRESOURCE_DATA vertexSubResourceData;
	ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
	vertexSubResourceData.pSysMem = pMem;
	*vertexBuffer = CreateD3DVertexBuffer(device, numBytes, false, false, &vertexSubResourceData);
}

void Library::BSPEngine::BasicMaterial::WriteVertex(byte * pMem, size_t offset, const Vertex & vertex)
{
	BasicMaterialVertex *pDstVertex = reinterpret_cast<BasicMaterialVertex*>(pMem) + offset;
	pDstVertex->Position = { vertex.x, vertex.y, vertex.z };
}

size_t Library::BSPEngine::BasicMaterial::VertexSize() const
{
	return sizeof(BasicMaterialVertex);
}

size_t Library::BSPEngine::BasicMaterial::WeldBuffers(byte * pMem, size_t numVertices, ULONG * pIndices, size_t numIndices)
{
	return WeldBuffersT<BasicMaterialVertex>(pMem, numVertices, pIndices, numIndices);
}

void Library::BSPEngine::BasicMaterial::Update(ID3D11DeviceContext * deviceContext, XMMATRIX wvp, XMVECTOR color)
{
	mShader->SetParameters(deviceContext, wvp, color);
}

void Library::BSPEngine::BasicMaterial::Apply(ID3D11DeviceContext * deviceContext)
{
	mShader->Apply(deviceContext);
}
