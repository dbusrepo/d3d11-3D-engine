#include "D3DAppException.h"
#include "SkyboxShader.h"
#include "Mesh.h"
#include "SkyboxMaterial.h"

namespace Library
{
	RTTI_DEFINITIONS(SkyboxMaterial)

	SkyboxMaterial::SkyboxMaterial()
	{
		mShader = nullptr;
	}

	SkyboxMaterial::~SkyboxMaterial()
	{
		delete mShader;
		mShader = nullptr;
	}

	void SkyboxMaterial::Initialize(ID3D11Device* device, HWND hwnd)
	{
		mShader = new SkyboxShader;
		mShader->Initialize(device, hwnd);
	}

	void SkyboxMaterial::CreateVertexBuffer(ID3D11Device * device, const Mesh & mesh, ID3D11Buffer ** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

		std::vector<SkyboxMaterialVertex> vertices;
		vertices.reserve(sourceVertices.size());

		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			vertices.push_back(SkyboxMaterialVertex(XMFLOAT4(position.x, position.y, position.z, 1.0f)));
		}

		CreateVertexBuffer(device, &vertices[0], vertices.size(), vertexBuffer);
	}

	void SkyboxMaterial::CreateVertexBuffer(ID3D11Device * device, SkyboxMaterialVertex * vertices, size_t vertexCount, ID3D11Buffer ** vertexBuffer) const
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

	size_t SkyboxMaterial::VertexSize() const
	{
		return sizeof(SkyboxMaterialVertex);
	}

	void SkyboxMaterial::Apply(ID3D11DeviceContext *deviceContext)
	{
		mShader->Apply(deviceContext);
	}

	void SkyboxMaterial::Update(ID3D11DeviceContext *deviceContext, XMMATRIX wvp, ID3D11ShaderResourceView* textureView)
	{
		mShader->SetParameters(deviceContext, wvp, textureView);
	}

}