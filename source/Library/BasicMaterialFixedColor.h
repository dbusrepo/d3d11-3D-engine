#pragma once

#include "Common.h"
#include "Material.h"


namespace Library
{
	class FixedColorShader;

	struct BasicMaterialFixedColorVertex
	{
		XMFLOAT4 Position;

		BasicMaterialFixedColorVertex() { }

		BasicMaterialFixedColorVertex(XMFLOAT4 position)
			: Position(position) { }
	};

	class BasicMaterialFixedColor : public Material
	{
		RTTI_DECLARATIONS(BasicMaterialFixedColor, Material)

			//	MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)

	public:
		BasicMaterialFixedColor();
		~BasicMaterialFixedColor();

		virtual void Initialize(ID3D11Device* device, HWND hwnd) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, BasicMaterialFixedColorVertex* vertices, size_t vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual size_t VertexSize() const override;

		void UpdateMVP(ID3D11DeviceContext *deviceContext, XMMATRIX wvp);
		void UpdateColor(ID3D11DeviceContext *deviceContext, XMVECTOR color);
		void Apply(ID3D11DeviceContext *deviceContext);

	private:
		FixedColorShader *mShader;
	};
}