#pragma once

#include "Common.h"

namespace Library
{
	namespace BSPEngine {

		class Vertex;
		class DepthPassShader;


		class DepthPassMaterial
		{

		public:
			DepthPassMaterial();
			~DepthPassMaterial();

			void Initialize(ID3D11Device* device, HWND hwnd);
			void CreateVertexBuffer(ID3D11Device* device, const byte *pMem, size_t numBytes, ID3D11Buffer** vertexBuffer) const;
			void WriteVertex(byte *pMem, size_t offset, const Vertex &vertex);
			size_t VertexSize() const;
			size_t WeldBuffers(byte *pMem, size_t numVertices, ULONG *pIndices, size_t numIndices);

			void Update(ID3D11DeviceContext *deviceContext, XMMATRIX wvp);
			void Apply(ID3D11DeviceContext *deviceContext);

		private:
			DepthPassShader *mShader;
		};

	}
}