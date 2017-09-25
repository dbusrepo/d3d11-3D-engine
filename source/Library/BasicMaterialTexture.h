#pragma once

#include "Common.h"
#include "Material.h"


namespace Library
{
	class TextureShader;

	struct BasicMaterialTextureVertex
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;

		BasicMaterialTextureVertex() { }

		BasicMaterialTextureVertex(XMFLOAT4 position, XMFLOAT2 textureCoordinates)
			: Position(position), TextureCoordinates(textureCoordinates) { }
	};

	class BasicMaterialTexture : public Material
	{
		RTTI_DECLARATIONS(BasicMaterialTexture, Material)

			//	MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)

	public:
		BasicMaterialTexture();
		~BasicMaterialTexture();

		virtual void Initialize(ID3D11Device* device, HWND hwnd) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, BasicMaterialTextureVertex* vertices, size_t vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual size_t VertexSize() const override;

		void Update(ID3D11DeviceContext *deviceContext, XMMATRIX wvp, XMMATRIX texMat, ID3D11ShaderResourceView* textureView);
		void Apply(ID3D11DeviceContext *deviceContext);

	private:
		TextureShader *mShader;
	};
}