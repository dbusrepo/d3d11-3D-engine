#include "Model.h"
#include "D3DApp.h"
#include "D3DAppException.h"
#include "Mesh.h"
#include "ModelMaterial.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Library
{

	Model::Model()
	{
	}

	Model::Model(const std::string& filename, bool flipUVs)
		: mMeshes(), mMaterials()
	{
		Assimp::Importer importer;

		UINT flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipWindingOrder;
		if (flipUVs)
		{
			flags |= aiProcess_FlipUVs;
		}

		const aiScene* scene = importer.ReadFile(filename, flags);
		if (scene == nullptr)
		{
			throw D3DAppException(importer.GetErrorString());
		}

		if (scene->HasMaterials())
		{
			for (UINT i = 0; i < scene->mNumMaterials; i++)
			{
				mMaterials.push_back(new ModelMaterial(*this, scene->mMaterials[i]));
			}
		}

		if (scene->HasMeshes())
		{
			for (UINT i = 0; i < scene->mNumMeshes; i++)
			{
				ModelMaterial* material = (mMaterials.size() > i ? mMaterials.at(i) : nullptr);

				Mesh* mesh = new Mesh(*this, *(scene->mMeshes[i]));
				mMeshes.push_back(mesh);
			}
		}
	}

	Model::~Model()
	{
		for (Mesh* mesh : mMeshes)
		{
			delete mesh;
		}

		for (ModelMaterial* material : mMaterials)
		{
			delete material;
		}
	}

	bool Model::HasMeshes() const
	{
		return (mMeshes.size() > 0);
	}

	bool Model::HasMaterials() const
	{
		return (mMaterials.size() > 0);
	}

	const std::vector<Mesh*>& Model::Meshes() const
	{
		return mMeshes;
	}

	const std::vector<ModelMaterial*>& Model::Materials() const
	{
		return mMaterials;
	}

	Model * Model::CreateGrid(float width, float depth, UINT m, UINT n)
	{
		Model *gridModel = new Model;
		Mesh *gridMesh = new Mesh(*gridModel, "Grid");
		ModelMaterial *gridMaterial = new ModelMaterial(*gridModel);
		std::vector<std::wstring>* textures = new std::vector<std::wstring>();
		textures->push_back(L"Checkerboard.dds");
		gridMaterial->mTextures.insert(std::pair<TextureType, std::vector<std::wstring>*>(TextureTypeDiffuse, textures));
		
		gridModel->mMeshes.push_back(gridMesh);
		gridModel->mMaterials.push_back(gridMaterial);

		UINT vertexCount = m*n;
		UINT faceCount = (m - 1)*(n - 1) * 2;

		gridMesh->mFaceCount = faceCount;

		//
		// Create the vertices.
		//

		float halfWidth = 0.5f*width;
		float halfDepth = 0.5f*depth;

		float dx = width / (n - 1);
		float dz = depth / (m - 1);

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);

		gridMesh->mVertices.resize(vertexCount);
		gridMesh->mNormals.resize(vertexCount);
		gridMesh->mTangents.resize(vertexCount);
		gridMesh->mTextureCoordinates.push_back(new std::vector<XMFLOAT3>());
		gridMesh->mTextureCoordinates[0]->resize(vertexCount);
		std::vector<XMFLOAT3> &texCoords = *gridMesh->mTextureCoordinates[0];

		for (UINT i = 0; i < m; ++i)
		{
			float z = halfDepth - i*dz;
			for (UINT j = 0; j < n; ++j)
			{
				float x = -halfWidth + j*dx;

				gridMesh->mVertices[i*n + j] = XMFLOAT3(x, 0.0f, z);
				gridMesh->mNormals[i*n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
				gridMesh->mTangents[i*n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				texCoords[i*n + j] = XMFLOAT3(j*du, i*dv, 0.0f);
			}
		}

		gridMesh->mIndices.resize(faceCount * 3); // 3 indices per face
											  // Iterate over each quad and compute indices.
		UINT k = 0;
		for (UINT i = 0; i < m - 1; ++i)
		{
			for (UINT j = 0; j < n - 1; ++j)
			{
				gridMesh->mIndices[k] = i*n + j;
				gridMesh->mIndices[k + 1] = i*n + j + 1;
				gridMesh->mIndices[k + 2] = (i + 1)*n + j;

				gridMesh->mIndices[k + 3] = (i + 1)*n + j;
				gridMesh->mIndices[k + 4] = i*n + j + 1;
				gridMesh->mIndices[k + 5] = (i + 1)*n + j + 1;

				k += 6; // next quad
			}
		}

		return gridModel;
	}

	Model * Model::CreateSphere(float radius, UINT sliceCount, UINT stackCount)
	{
		struct Vertex
		{
			Vertex() {}
			Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT3& uvw)
				: Position(p), Normal(n), TangentU(t), TexC(uvw) {}
			Vertex(
				float px, float py, float pz,
				float nx, float ny, float nz,
				float tx, float ty, float tz,
				float u, float v, float w)
				: Position(px, py, pz), Normal(nx, ny, nz),
				TangentU(tx, ty, tz), TexC(u, v, w) {}

			XMFLOAT3 Position;
			XMFLOAT3 Normal;
			XMFLOAT3 TangentU;
			XMFLOAT3 TexC;
		};

		Model *gridModel = new Model;
		Mesh *gridMesh = new Mesh(*gridModel, "Sphere");

		gridModel->mMeshes.push_back(gridMesh);

		float phiStep = XM_PI / stackCount;
		float thetaStep = 2.0f*XM_PI / sliceCount;

		gridMesh->mVertices.push_back(XMFLOAT3(0.0f, +radius, 0.0f));
		gridMesh->mNormals.push_back(XMFLOAT3(0.0f, +1.0f, 0.0f));
		gridMesh->mTangents.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
		gridMesh->mTextureCoordinates.push_back(new std::vector<XMFLOAT3>());
		std::vector<XMFLOAT3> &texCoords = *gridMesh->mTextureCoordinates[0];
		
		texCoords.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));

		// Compute vertices for each stack ring (do not count the poles as rings).
		for (UINT i = 1; i <= stackCount - 1; ++i)
		{
			float phi = i*phiStep;

			// Vertices of ring.
			for (UINT j = 0; j <= sliceCount; ++j)
			{
				float theta = j*thetaStep;

				Vertex v;

				// spherical to cartesian
				v.Position.x = radius*sinf(phi)*cosf(theta);
				v.Position.y = radius*cosf(phi);
				v.Position.z = radius*sinf(phi)*sinf(theta);

				// Partial derivative of P with respect to theta
				v.TangentU.x = -radius*sinf(phi)*sinf(theta);
				v.TangentU.y = 0.0f;
				v.TangentU.z = +radius*sinf(phi)*cosf(theta);

				XMVECTOR T = XMLoadFloat3(&v.TangentU);
				XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

				XMVECTOR p = XMLoadFloat3(&v.Position);
				XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

				v.TexC.x = theta / XM_2PI;
				v.TexC.y = phi / XM_PI;
				v.TexC.z = 0.0f;

				gridMesh->mVertices.push_back(v.Position);
				gridMesh->mNormals.push_back(v.Normal);
				gridMesh->mTangents.push_back(v.TangentU);
				texCoords.push_back(v.TexC);
			}
		}

		gridMesh->mVertices.push_back(XMFLOAT3(0.0f, -radius, 0.0f));
		gridMesh->mNormals.push_back(XMFLOAT3(0.0f, -1.0f, 0.0f));
		gridMesh->mTangents.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));
		texCoords.push_back(XMFLOAT3(0.0f, 1.0f, 0.0f));

		//
		// Compute indices for top stack.  The top stack was written first to the vertex buffer
		// and connects the top pole to the first ring.
		//

		for (UINT i = 1; i <= sliceCount; ++i)
		{
			gridMesh->mIndices.push_back(0);
			gridMesh->mIndices.push_back(i + 1);
			gridMesh->mIndices.push_back(i);
		}


		//
		// Compute indices for inner stacks (not connected to poles).
		//

		// Offset the indices to the index of the first vertex in the first ring.
		// This is just skipping the top pole vertex.
		UINT baseIndex = 1;
		UINT ringVertexCount = sliceCount + 1;
		for (UINT i = 0; i < stackCount - 2; ++i)
		{
			for (UINT j = 0; j < sliceCount; ++j)
			{
				gridMesh->mIndices.push_back(baseIndex + i*ringVertexCount + j);
				gridMesh->mIndices.push_back(baseIndex + i*ringVertexCount + j + 1);
				gridMesh->mIndices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

				gridMesh->mIndices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
				gridMesh->mIndices.push_back(baseIndex + i*ringVertexCount + j + 1);
				gridMesh->mIndices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
			}
		}


		//
		// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
		// and connects the bottom pole to the bottom ring.
		//

		// South pole vertex was added last.
		UINT southPoleIndex = (UINT)gridMesh->mVertices.size() - 1;

		// Offset the indices to the index of the first vertex in the last ring.
		baseIndex = southPoleIndex - ringVertexCount;

		for (UINT i = 0; i < sliceCount; ++i)
		{
			gridMesh->mIndices.push_back(southPoleIndex);
			gridMesh->mIndices.push_back(baseIndex + i);
			gridMesh->mIndices.push_back(baseIndex + i + 1);
		}

		gridMesh->mFaceCount = (UINT)(gridMesh->mIndices.size() / 3);

		return gridModel;

	}


}
