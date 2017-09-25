#pragma once

#include "Common.h"

namespace Library
{
	class Model;
	class Mesh;

	class Material : public RTTI
	{
		RTTI_DECLARATIONS(Material, RTTI)

	public:
		Material();
		virtual ~Material();

		virtual void Initialize(ID3D11Device* device, HWND hwnd) = 0;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Model& model, std::vector<ID3D11Buffer*>& vertexBuffers) const;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const = 0;
		virtual size_t VertexSize() const = 0;

	protected:
		Material(const Material& rhs);
		Material& operator=(const Material& rhs);
		
	};

}