#include "Model.h"
#include "Material.h"

namespace Library
{
	RTTI_DEFINITIONS(Material)

	Material::Material()
	{
	}

	Material::~Material()
	{
	}

	void Material::CreateVertexBuffer(ID3D11Device * device, const Model & model, std::vector<ID3D11Buffer*>& vertexBuffers) const
	{
		vertexBuffers.reserve(model.Meshes().size());
		for (Mesh* mesh : model.Meshes())
		{
			ID3D11Buffer* vertexBuffer;
			CreateVertexBuffer(device, *mesh, &vertexBuffer);
			vertexBuffers.push_back(vertexBuffer);
		}
	}

}