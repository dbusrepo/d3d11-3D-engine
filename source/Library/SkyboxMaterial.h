#pragma once

#include "Common.h"
#include "Material.h"


namespace Library
{
	class SkyboxShader;

	struct SkyboxMaterialVertex
	{
		XMFLOAT4 Position;

		SkyboxMaterialVertex() { }

		SkyboxMaterialVertex(XMFLOAT4 position)
			: Position(position) { }
	};

	class SkyboxMaterial : public Material
	{
		RTTI_DECLARATIONS(SkyboxMaterial, Material)


	public:
		SkyboxMaterial();
		~SkyboxMaterial();

		virtual void Initialize(ID3D11Device* device, HWND hwnd) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, SkyboxMaterialVertex* vertices, size_t vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual size_t VertexSize() const override;

		void Update(ID3D11DeviceContext *deviceContext, XMMATRIX wvp, ID3D11ShaderResourceView* textureView);
		void Apply(ID3D11DeviceContext *deviceContext);

	private:
		SkyboxShader *mShader;
	};
}