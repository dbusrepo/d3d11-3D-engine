#include "D3DAppException.h"
#include "TextureShader.h"
#include "Mesh.h"
#include "BasicMaterialTexture.h"

namespace Library
{
	RTTI_DEFINITIONS(BasicMaterialTexture)

	BasicMaterialTexture::BasicMaterialTexture()
	{
		mShader = nullptr;
	}

	BasicMaterialTexture::~BasicMaterialTexture()
	{
		delete mShader;
		mShader = nullptr;
	}

	void BasicMaterialTexture::Initialize(ID3D11Device* device, HWND hwnd)
	{
		mShader = new TextureShader;
		mShader->Initialize(device, hwnd);
	}

	void BasicMaterialTexture::CreateVertexBuffer(ID3D11Device * device, const Mesh & mesh, ID3D11Buffer ** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();

		std::vector<BasicMaterialTextureVertex> vertices;
		vertices.reserve(sourceVertices.size());

		std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());

		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			vertices.push_back(BasicMaterialTextureVertex(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y)));
		}
		
		CreateVertexBuffer(device, &vertices[0], vertices.size(), vertexBuffer);
	}

	void BasicMaterialTexture::CreateVertexBuffer(ID3D11Device * device, BasicMaterialTextureVertex * vertices, size_t vertexCount, ID3D11Buffer ** vertexBuffer) const
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

	size_t BasicMaterialTexture::VertexSize() const
	{
		return sizeof(BasicMaterialTextureVertex);
	}

	void BasicMaterialTexture::Apply(ID3D11DeviceContext *deviceContext)
	{
		mShader->Apply(deviceContext);
	}

	void BasicMaterialTexture::Update(ID3D11DeviceContext *deviceContext, XMMATRIX wvp, XMMATRIX texMat, ID3D11ShaderResourceView* textureView)
	{
		mShader->SetParameters(deviceContext, wvp, texMat, textureView);
	}

}