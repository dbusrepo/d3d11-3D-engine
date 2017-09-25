#pragma once

#include "Common.h"

namespace Library
{
	namespace BSPEngine {

		class Vertex;
		class DoorShader;

		struct DoorMaterialVertex
		{
			XMFLOAT3 Position;
			XMFLOAT3 TextureCoordinates; // TextureCoordinates[2] is the texArrayIndex
			//XMFLOAT2 LightMapCoordinates;
			//XMFLOAT3 Normal;
			//XMFLOAT4 Tangent;
			//uint32_t LeafIndex;

			DoorMaterialVertex() { }

			DoorMaterialVertex(XMFLOAT3 position, XMFLOAT3 texcoords,
				XMFLOAT3 normal, XMFLOAT4 tangent, uint32_t leafIndex)
				: Position(position), TextureCoordinates(texcoords) {}
				//Normal(normal), Tangent(tangent), LeafIndex(leafIndex) { }

			bool same(const DoorMaterialVertex &vertex) const;
		};

		class DoorMaterial
		{
		public:
			DoorMaterial();
			~DoorMaterial();

			void Initialize(ID3D11Device* device, HWND hwnd);
			//void CreateVertexBuffer(ID3D11Device* device, const Vertex *vertices, size_t numVertices, ID3D11Buffer** vertexBuffer) const;
			void CreateVertexBuffer(ID3D11Device* device, const byte *pMem, size_t numBytes, ID3D11Buffer** vertexBuffer) const;
			void WriteVertex(byte *pMem, size_t offset, const Vertex &vertex, uint16_t texArrayID);
			size_t VertexSize() const;
			size_t WeldBuffers(byte *pMem, size_t numVertices, ULONG *pIndices, size_t numIndices);
			void SetUseLighting(bool useLighting);
			void SetCBufferVS(ID3D11DeviceContext *deviceContext, XMMATRIX wvp);
			void SetCBufferPS(ID3D11DeviceContext *deviceContext, XMFLOAT3 worldCamPos, XMFLOAT2 clusterMapSize);
			void SetDiffuseAndNormalTextureArrayViews(ID3D11DeviceContext *deviceContext, ID3D11ShaderResourceView* textureArrDiffuseMapsView, ID3D11ShaderResourceView* textureArrNormalMapsView);
			void SetLightmapClusterMapsTextureArrayView(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* lightMapTexArrayView, ID3D11ShaderResourceView* clusterMapTexArrayView);

			void Apply(ID3D11DeviceContext *deviceContext);

		private:
			DoorShader *mShader;
		};

	}
}