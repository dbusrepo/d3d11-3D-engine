#include "D3DAppException.h"
#include "FixedColorShader.h"
#include "ColorHelper.h"
#include "Mesh.h"
#include "BasicMaterialFixedColor.h"

namespace Library
{
	RTTI_DEFINITIONS(BasicMaterialFixedColor)

	BasicMaterialFixedColor::BasicMaterialFixedColor()
	{
		mShader = nullptr;
	}

	BasicMaterialFixedColor::~BasicMaterialFixedColor()
	{
		delete mShader;
		mShader = nullptr;
	}

	void BasicMaterialFixedColor::Initialize(ID3D11Device* device, HWND hwnd)
	{
		mShader = new FixedColorShader;
		mShader->Initialize(device, hwnd);
	}

	void BasicMaterialFixedColor::CreateVertexBuffer(ID3D11Device * device, const Mesh & mesh, ID3D11Buffer ** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

		std::vector<BasicMaterialFixedColorVertex> vertices;
		vertices.reserve(sourceVertices.size());

		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			vertices.push_back(BasicMaterialFixedColorVertex(XMFLOAT4(position.x, position.y, position.z, 1.0f)));
		}
		

		CreateVertexBuffer(device, &vertices[0], vertices.size(), vertexBuffer);
	}

	void BasicMaterialFixedColor::CreateVertexBuffer(ID3D11Device * device, BasicMaterialFixedColorVertex * vertices, size_t vertexCount, ID3D11Buffer ** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = (UINT)(VertexSize() * vertexCount);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer)))
		{
			throw D3DAppException("ID3D11Device::CreateBuffer() failed.");
		}
	}

	size_t BasicMaterialFixedColor::VertexSize() const
	{
		return sizeof(BasicMaterialFixedColor);
	}

	void BasicMaterialFixedColor::Apply(ID3D11DeviceContext *deviceContext)
	{
		mShader->Apply(deviceContext);
	}

	void BasicMaterialFixedColor::UpdateMVP(ID3D11DeviceContext *deviceContext, XMMATRIX wvp)
	{
		mShader->SetParameters(deviceContext, wvp);
	}

	void BasicMaterialFixedColor::UpdateColor(ID3D11DeviceContext *deviceContext, XMVECTOR color)
	{
		mShader->SetParameters(deviceContext, color);
	}

}