#pragma once

#include "Common.h"

namespace Library
{
	class D3DApp;
	class Mesh;
	class ModelMaterial;

	class Model
	{
	public:
		Model();
		Model(const std::string& filename, bool flipUVs = false);
		~Model();

		bool HasMeshes() const;
		bool HasMaterials() const;

		const std::vector<Mesh*>& Meshes() const;
		const std::vector<ModelMaterial*>& Materials() const;

		static Model *CreateGrid(float width, float depth, UINT m, UINT n);
		static Model *CreateSphere(float radius, UINT sliceCount, UINT stackCount);

	private:
		Model(const Model& rhs);
		Model& operator=(const Model& rhs);

		//D3DApp& mApp;
		std::vector<Mesh*> mMeshes;
		std::vector<ModelMaterial*> mMaterials;
	};
}
